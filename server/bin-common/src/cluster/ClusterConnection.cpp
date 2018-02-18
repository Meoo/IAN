/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusterConnection.hpp"
#include "ClusterInternal.hpp"

#include <common/EasyProfiler.hpp>

#include <proto-in/ClusterHandshake_generated.h>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/read.hpp>
#include <boost/beast/core/buffers_cat.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/endian/conversion.hpp>


namespace asio = boost::asio;


#define LOG_SOCKET_TUPLE remote_.address().to_string(), remote_.port()

namespace
{

const size_t read_chunk_size = 16 * 1024; // 16k

bool is_connection_reset_error(const boost::system::error_code & ec)
{
  if (ec.category() == asio::error::get_system_category() &&
      (ec == asio::error::connection_aborted || ec == asio::error::connection_reset))
  {
    return true;
  }

  if (ec.category() == asio::error::get_misc_category() && ec == asio::error::eof)
  {
    return true;
  }

  if (ec.category() == asio::ssl::error::get_stream_category() &&
      ec == asio::ssl::error::stream_truncated)
  {
    return true;
  }

  return false;
}

bool is_connection_canceled_error(const boost::system::error_code & ec)
{
  if (ec.category() == asio::error::get_ssl_category() &&
      ERR_GET_REASON(ec.value()) == SSL_R_PROTOCOL_IS_SHUTDOWN)
  {
    return true;
  }

  return false;
}

} // namespace


ClusterConnection::ClusterConnection(const std::shared_ptr<spdlog::logger> & logger,
                                     TcpSocket && socket)
    : logger_(logger), stream_(std::move(socket), cluster::internal::get_ssl()),
      socket_(stream_.next_layer()), strand_(socket_.get_io_context()),
      timer_(socket_.get_io_context())
{
}

void ClusterConnection::run(SslRole role, bool safe_link)
{
  // Should never happen, just check to be safe
  if (!socket_.is_open())
  {
    IAN_ERROR(logger_, "Trying to run ClusterConnection with closed socket");
    return;
  }

  safe_link_ = safe_link;

  // Cache remote endpoint
  boost::system::error_code ec;
  remote_ = socket_.remote_endpoint(ec);

  // Force peer verification
  stream_.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert, ec);
  if (ec)
    IAN_ERROR(logger_, "Failed to force peer verification for cluster connection: : {}:{} : {}",
              LOG_SOCKET_TUPLE, ec.message());

  // Always queue outgoing messages until handshake is performed
  is_writing_ = true;

  // SSL handshake
  stream_.async_handshake(
      role == Client ? asio::ssl::stream_base::client : asio::ssl::stream_base::server,
      asio::bind_executor(strand_, std::bind(&ClusterConnection::on_ssl_handshake,
                                             shared_from_this(), std::placeholders::_1)));
}

void ClusterConnection::send_message(const Message & message)
{
  if (message.is_empty())
  {
    IAN_ERROR(logger_, "send_message: message is empty");
    return;
  }

  // Execute in strand
  asio::post(strand_, [this, self{shared_from_this()}, message{message}]() mutable {
    if (dropped_ || shutting_down_)
      return;

    if (!is_writing_)
    {
      is_writing_ = true;
      do_write_message(std::move(message));
    }
    else
    {
      message_queue_.push(std::move(message));
    }
  });
}

void ClusterConnection::abort()
{
  if (dropped_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  message_queue_.clear();

  IAN_INFO(logger_, "Peer disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  socket_.close(ec);
  dropped_ = true;
}

void ClusterConnection::shutdown()
{
  if (dropped_ || shutting_down_)
    return;

  if (downgraded_)
  {
    this->abort();
    return;
  }

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  message_queue_.clear();

  if (!socket_.is_open())
    return;

  // Shutdown stream at SSL level
  stream_.async_shutdown(
      asio::bind_executor(strand_, std::bind(&ClusterConnection::on_shutdown, shared_from_this(),
                                             std::placeholders::_1)));
  shutting_down_ = true;
}

void ClusterConnection::read_next()
{
  if (safe_link_)
  {
    // SSL
    stream_.async_read_some(
        read_buffer_.prepare(read_chunk_size),
        asio::bind_executor(strand_, std::bind(&ClusterConnection::on_read, shared_from_this(),
                                               std::placeholders::_1, std::placeholders::_2)));
  }
  else
  {
    // Clear
    socket_.async_read_some(
        read_buffer_.prepare(read_chunk_size),
        asio::bind_executor(strand_, std::bind(&ClusterConnection::on_read, shared_from_this(),
                                               std::placeholders::_1, std::placeholders::_2)));
  }
}

void ClusterConnection::do_write_message(Message && message)
{
  message_outbound_   = std::move(message);
  out_message_len_le_ = boost::endian::native_to_little(
      (uint32_t)(message_outbound_.get_payload_size()));

  // Send size + type + payload
  auto data = boost::beast::buffers_cat(
      asio::buffer(&out_message_len_le_, sizeof(out_message_len_le_)),
      asio::buffer(message_outbound_.get_payload(), message_outbound_.get_payload_size()));

  if (safe_link_)
  {
    // SSL
    asio::async_write(
        stream_, data,
        asio::bind_executor(strand_,
                            std::bind(&ClusterConnection::on_write_message, shared_from_this(),
                                      std::placeholders::_1, std::placeholders::_2)));
  }
  else
  {
    // Clear
    asio::async_write(
        socket_, data,
        asio::bind_executor(strand_,
                            std::bind(&ClusterConnection::on_write_message, shared_from_this(),
                                      std::placeholders::_1, std::placeholders::_2)));
  }
}

void ClusterConnection::on_shutdown(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    if (!is_connection_reset_error(ec) && !is_connection_canceled_error(ec))
      IAN_WARN(logger_, "Shutdown error for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
               ec.value());
  }

  IAN_INFO(logger_, "Peer disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  socket_.close(ec);
  dropped_ = true;
}

void ClusterConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (ec)
  {
    IAN_WARN(logger_, "SSL handshake failed for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             ec.message(), ec.value());
    this->abort();
    return;
  }

  IAN_TRACE(logger_, "SSL handshake complete for peer: {}:{}", LOG_SOCKET_TUPLE);

  // Write IAN handshake
  flatbuffers::FlatBufferBuilder builder;
  auto offset = proto::CreateClusterHandshake(builder, 1, 2, safe_link_);
  proto::FinishClusterHandshakeBuffer(builder, offset);

  message_outbound_ = Message::from_flatbuffer(builder);
  out_message_len_le_ =
      boost::endian::native_to_little((uint32_t)message_outbound_.get_payload_size());

  auto data = boost::beast::buffers_cat(
      asio::buffer(&out_message_len_le_, sizeof(out_message_len_le_)),
      asio::buffer(message_outbound_.get_payload(), message_outbound_.get_payload_size()));

  asio::async_write(
      stream_, data,
      asio::bind_executor(strand_,
                          std::bind(&ClusterConnection::on_ian_handshake_sent, shared_from_this(),
                                    std::placeholders::_1, std::placeholders::_2)));
}

void ClusterConnection::on_ian_handshake_sent(boost::system::error_code ec, std::size_t writelen)
{
  // Clear data that was begin sent
  message_outbound_ = Message();

  if (ec)
  {
    handle_write_error(ec);
    return;
  }

  IAN_TRACE(logger_, "Handshake written (len: {}) for peer: {}:{}", writelen, LOG_SOCKET_TUPLE);

  // Read IAN handshake
  stream_.async_read_some(
      read_buffer_.prepare(read_chunk_size),
      asio::bind_executor(strand_,
                          std::bind(&ClusterConnection::on_ian_handshake, shared_from_this(),
                                    std::placeholders::_1, std::placeholders::_2)));
}

void ClusterConnection::on_ian_handshake(boost::system::error_code ec, std::size_t readlen)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  read_buffer_.commit(readlen);

  if (in_message_len_ == 0)
  {
    // Read handshake length
    if (read_buffer_.size() < sizeof(in_message_len_))
    {
      // If first packet contains less than 4 bytes something is really wrong
      IAN_ERROR(logger_, "Not enough data to read handshake length : {}:{} : {}", LOG_SOCKET_TUPLE,
                read_buffer_.size());
      shutdown();
      return;
    }

    // Extract message length (little endian)
    read_buffer_.consume(asio::buffer_copy(
        asio::buffer((void *)&in_message_len_, sizeof(in_message_len_)), read_buffer_.data()));
    boost::endian::little_to_native_inplace(in_message_len_);

    if (in_message_len_ == 0) // TODO Max message length
    {
      IAN_ERROR(logger_, "Invalid incoming message length : {}:{} : {}", LOG_SOCKET_TUPLE,
                in_message_len_);
      shutdown();
      return;
    }
  }

  if (read_buffer_.size() < in_message_len_)
  {
    IAN_TRACE(logger_, "Not enough data to parse handshake : {}:{} : {} < {}", LOG_SOCKET_TUPLE,
              read_buffer_.size(), in_message_len_);

    // Not enough data, read again
    stream_.async_read_some(
        read_buffer_.prepare(read_chunk_size),
        asio::bind_executor(strand_,
                            std::bind(&ClusterConnection::on_ian_handshake, shared_from_this(),
                                      std::placeholders::_1, std::placeholders::_2)));
    return;
  }

  if (read_buffer_.size() > in_message_len_)
  {
    IAN_WARN(logger_, "Received more data than expected for handshake : {}:{} : {} > {}",
             LOG_SOCKET_TUPLE, read_buffer_.size(), in_message_len_);
    shutdown();
    return;
  }

  // Flatten
  std::vector<uint8_t> buf(in_message_len_);
  read_buffer_.consume(
      asio::buffer_copy(asio::buffer(buf.data(), buf.size()), read_buffer_.data()));

  // Verify handshake
  {
    flatbuffers::Verifier verifier((const uint8_t *)buf.data(), buf.size());
    if (buf.size() < 8 || !proto::ClusterHandshakeBufferHasIdentifier(buf.data()) ||
        !proto::VerifyClusterHandshakeBuffer(verifier))
    {
      IAN_WARN(logger_, "Cluster handshake verification failed : {}:{}", LOG_SOCKET_TUPLE);
      shutdown();
      return;
    }
  }

  const proto::ClusterHandshake * handshake = proto::GetClusterHandshake(buf.data());

  auto major = handshake->version_major();
  auto minor = handshake->version_minor();

  IAN_DEBUG(logger_, "Proto version for peer : {}:{} : {}.{}", LOG_SOCKET_TUPLE, major, minor);

  safe_link_ = safe_link_ && handshake->safe_link();

  // Handshake done!
  IAN_INFO(logger_, "Peer ready: {}:{}", LOG_SOCKET_TUPLE);

  // Reset for next message
  in_message_len_ = 0;

  // Start reading
  read_next();

  // Start writing messages if any are queued
  // is_writing_ = true at this point, set in run()
  Message message;
  if (message_queue_.try_pop(message))
  {
    do_write_message(std::move(message));
  }
  else
  {
    is_writing_ = false;
  }
}

void ClusterConnection::on_read(boost::system::error_code ec, std::size_t readlen)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  read_buffer_.commit(readlen);

  // Data read loop (one read might contain multiple messages)
  for (;;)
  {
    // Read size
    if (in_message_len_ == 0)
    {
      // Not enough data
      if (read_buffer_.size() < sizeof(in_message_len_))
        break;

      // Extract message length (little endian)
      read_buffer_.consume(asio::buffer_copy(
          asio::buffer((void *)&in_message_len_, sizeof(in_message_len_)), read_buffer_.data()));
      boost::endian::little_to_native_inplace(in_message_len_);

      if (in_message_len_ == 0) // TODO Max message length
      {
        IAN_ERROR(logger_, "Invalid incoming message length : {}:{} : {}", LOG_SOCKET_TUPLE,
                  in_message_len_);
        shutdown();
        return;
      }
    }

    if (read_buffer_.size() < in_message_len_)
      break;

    // Enough data to read message

    // Flatten data
    std::vector<uint8_t> buf(in_message_len_);
    read_buffer_.consume(
        asio::buffer_copy(asio::buffer(buf.data(), buf.size()), read_buffer_.data()));

    IAN_TRACE(logger_, "Message received from peer : {}:{} : type {} len {}", LOG_SOCKET_TUPLE,
              type, buf.size());
    // TODO

    // Reset for next message
    in_message_len_ = 0;
  }

  read_next();
}

void ClusterConnection::on_write_message(boost::system::error_code ec, std::size_t writelen)
{
  // Clear data that was begin sent
  message_outbound_ = Message();

  if (ec)
  {
    handle_write_error(ec);
    return;
  }

  IAN_TRACE(logger_, "Message written (len: {}) for peer: {}:{}", writelen, LOG_SOCKET_TUPLE);

  // Write next message
  Message message;
  if (message_queue_.try_pop(message))
  {
    do_write_message(std::move(message));
  }
  else
  {
    is_writing_ = false;
  }
}

void ClusterConnection::handle_read_error(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (is_connection_reset_error(ec))
  {
    this->abort();
    return;
  }

  if (is_connection_canceled_error(ec))
    return;

  IAN_WARN(logger_, "Read failed for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
           ec.value());

  shutdown();
}

void ClusterConnection::handle_write_error(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (is_connection_reset_error(ec))
  {
    this->abort();
    return;
  }

  if (is_connection_canceled_error(ec))
    return;

  IAN_WARN(logger_, "Write failed for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
           ec.value());

  shutdown();
}
