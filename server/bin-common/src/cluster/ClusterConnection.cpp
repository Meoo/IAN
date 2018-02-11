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
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/endian/conversion.hpp>


namespace asio = boost::asio;


#define LOG_SOCKET_TUPLE                                                                           \
  socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port()

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

ClusterConnection::~ClusterConnection()
{
  // Close must be called last or using LOG_SOCKET_TUPLE will throw
  boost::system::error_code ec;
  socket_.close(ec);
}

void ClusterConnection::run(SslRole role, bool safe_link)
{
  safe_link_ = safe_link;

  // Force peer verification
  boost::system::error_code ec;
  stream_.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert, ec);
  if (ec)
    IAN_ERROR(logger_, "Failed to force peer verification for cluster connection: : {}:{} : {}",
              LOG_SOCKET_TUPLE, ec.message());

  // SSL handshake
  stream_.async_handshake(
      role == Client ? asio::ssl::stream_base::client : asio::ssl::stream_base::server,
      asio::bind_executor(strand_, std::bind(&ClusterConnection::on_ssl_handshake,
                                             shared_from_this(), std::placeholders::_1)));
}

void ClusterConnection::abort()
{
  if (dropped_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  // TODO state_timer_.cancel(ec);

  // TODO message_queue_.clear();

  IAN_INFO(logger_, "Peer disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  dropped_ = true;
}

void ClusterConnection::shutdown()
{
  if (dropped_ || shutting_down_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  // TODO state_timer_.cancel(ec);

  // TODO message_queue_.clear();

  if (!socket_.is_open())
    return;

  // Shutdown stream at SSL level
  stream_.async_shutdown(
      asio::bind_executor(strand_, std::bind(&ClusterConnection::on_shutdown, shared_from_this(),
                                             std::placeholders::_1)));
  shutting_down_ = true;
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
  dropped_ = true;
}

void ClusterConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    IAN_WARN(logger_, "SSL handshake failed for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             ec.message(), ec.value());
    this->abort();
    return;
  }

  IAN_TRACE(logger_, "SSL handshake complete for peer: {}:{}", LOG_SOCKET_TUPLE);

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

  if (message_len_ == -1U)
  {
    // Read handshake length
    if (read_buffer_.size() < sizeof(message_len_))
    {
      // If first packet contains less than 4 bytes something is really wrong
      IAN_ERROR(logger_, "Not enough data to read handshake length : {}:{} : {}", LOG_SOCKET_TUPLE,
                read_buffer_.size());
      shutdown();
      return;
    }

    // Extract message length (little endian)
    read_buffer_.consume(asio::buffer_copy(
        asio::buffer((void *)&message_len_, sizeof(message_len_)), read_buffer_.data()));
    boost::endian::little_to_native_inplace(message_len_);
  }

  if (read_buffer_.size() < message_len_)
  {
    IAN_DEBUG(logger_, "Not enough data to parse handshake : {}:{} : {} < {}", LOG_SOCKET_TUPLE,
              read_buffer_.size(), message_len_);
    // Not enough data (somehow), read again
    stream_.async_read_some(
        read_buffer_.prepare(read_chunk_size),
        asio::bind_executor(strand_,
                            std::bind(&ClusterConnection::on_ian_handshake, shared_from_this(),
                                      std::placeholders::_1, std::placeholders::_2)));
    return;
  }

  // Flatten
  boost::beast::flat_buffer buf;
  buf.commit(asio::buffer_copy(buf.prepare(read_buffer_.size()), read_buffer_.data()));

  // Verify handshake
  {
    flatbuffers::Verifier verifier((const uint8_t *)buf.data().data(), buf.size());
    if (!proto::VerifyClusterHandshakeBuffer(verifier))
    {
      IAN_ERROR(logger_, "Cluster handshake verification failed : {}:{}", LOG_SOCKET_TUPLE);
      shutdown();
      return;
    }
  }

  const proto::ClusterHandshake * handshake = proto::GetClusterHandshake(buf.data().data());

  auto major = handshake->version_major();
  auto minor = handshake->version_minor();

  IAN_DEBUG(logger_, "Proto version for peer : {}:{} : {}.{}", LOG_SOCKET_TUPLE, major, minor);

  safe_link_ = safe_link_ && handshake->safe_link();

  if (safe_link_)
  {
    // TODO Downgrade to clear messages
  }
  else
  {
    // TODO Start reading messages over SSL
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
