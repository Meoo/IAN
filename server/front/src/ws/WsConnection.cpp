/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WsConnection.hpp"

#include <FrontGlobals.hpp>

#include <common/EasyProfiler.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <spdlog/fmt/ostr.h> // To display string_view


namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ws   = boost::beast::websocket;


#define LOG_SOCKET_TUPLE remote_.address().to_string(), remote_.port()
#define SHARED_FROM_THIS std::static_pointer_cast<WsConnection>(shared_from_this())

namespace
{

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


WsConnection::WsConnection(const std::shared_ptr<spdlog::logger> & logger, TcpSocket && socket,
                           SslContext & ssl_ctx)
    : logger_(logger), stream_(std::move(socket), ssl_ctx),
      socket_(stream_.next_layer().next_layer()), strand_(socket_.get_io_context()),
      timer_(socket_.get_io_context()), state_timer_(socket_.get_io_context()),
      rate_limit_start_(std::chrono::steady_clock::now())
{
  ++front::active_connection_count;
}

WsConnection::~WsConnection() { --front::active_connection_count; }

void WsConnection::run()
{
  // Should never happen, just check to be safe
  if (!socket_.is_open())
  {
    IAN_ERROR(logger_, "Trying to run ClusterConnection with closed socket");
    return;
  }

  // Cache remote endpoint
  boost::system::error_code ec;
  remote_ = socket_.remote_endpoint(ec);

  // Timeout for connection setup
  set_timeout(std::chrono::seconds(front::ws_setup_timeout));

  // SSL handshake
  stream_.next_layer().async_handshake(
      asio::ssl::stream_base::server,
      asio::bind_executor(strand_, std::bind(&WsConnection::on_ssl_handshake, SHARED_FROM_THIS,
                                             std::placeholders::_1)));
}

void WsConnection::send_message(const Message & message)
{
  if (message.is_empty())
  {
    IAN_ERROR(logger_, "send_message: message is empty");
    return;
  }

  // Execute in strand
  asio::post(strand_, [this, self{SHARED_FROM_THIS}, message{message}]() mutable {
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

void WsConnection::abort()
{
  if (dropped_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  state_timer_.cancel(ec);

  message_queue_.clear();

  IAN_INFO(logger_, "Client disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  socket_.close(ec);
  dropped_ = true;
}

void WsConnection::shutdown()
{
  if (dropped_ || shutting_down_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  state_timer_.cancel(ec);

  message_queue_.clear();

  if (!socket_.is_open())
    return;

  // Shutdown stream at SSL level
  stream_.next_layer().async_shutdown(asio::bind_executor(
      strand_, std::bind(&WsConnection::on_shutdown, SHARED_FROM_THIS, std::placeholders::_1)));
  shutting_down_ = true;
}

void WsConnection::do_write_message(Message && message)
{
  message_outbound_ = std::move(message);

  stream_.async_write(
      asio::buffer(message_outbound_.data(), message_outbound_.size()),
      asio::bind_executor(strand_, std::bind(&WsConnection::on_write_message, SHARED_FROM_THIS,
                                             std::placeholders::_1, std::placeholders::_2)));
}

void WsConnection::set_timeout(const asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  timer_.cancel(ec); // Errors ignored

  timer_.expires_from_now(delay);
  timer_.async_wait(asio::bind_executor(
      strand_, std::bind(&WsConnection::on_timeout, SHARED_FROM_THIS, std::placeholders::_1)));
}

void WsConnection::set_state_timeout(const asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  state_timer_.cancel(ec); // Errors ignored

  state_timer_.expires_from_now(delay);
  state_timer_.async_wait(asio::bind_executor(
      strand_, std::bind(&WsConnection::on_timeout, SHARED_FROM_THIS, std::placeholders::_1)));
}

void WsConnection::cancel_state_timeout()
{
  boost::system::error_code ec;
  state_timer_.cancel(ec); // Errors ignored
}

bool WsConnection::check_rate_limit()
{
  if (rate_limit_messages_ > front::rate_limit_messages ||
      rate_limit_bytes_ > front::rate_limit_bytes)
  {
    auto now = std::chrono::steady_clock::now();

    if (now - rate_limit_start_ > std::chrono::seconds(1))
    {
      IAN_TRACE(logger_, "Reset rate limiter counters: {}:{}", LOG_SOCKET_TUPLE);

      rate_limit_start_    = now;
      rate_limit_messages_ = 0;
      rate_limit_bytes_    = 0;
    }
    else
    {
      IAN_WARN(logger_, "Rate limiter kill: {}:{}", LOG_SOCKET_TUPLE);
      this->abort();
      return true;
    }
  }

  return false;
}

void WsConnection::on_timeout(boost::system::error_code ec)
{
  if (dropped_ || ec == boost::asio::error::operation_aborted)
    return;

  IAN_INFO(logger_, "Client timed out: {}:{}", LOG_SOCKET_TUPLE);
  this->abort();
}

void WsConnection::on_shutdown(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    if (!is_connection_reset_error(ec) && !is_connection_canceled_error(ec))
      IAN_WARN(logger_, "Shutdown error for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
               ec.value());
  }

  IAN_INFO(logger_, "Client disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  socket_.close(ec);
  dropped_ = true;
}

void WsConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (ec)
  {
    IAN_WARN(logger_, "SSL handshake failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             ec.message(), ec.value());
    this->abort();
    return;
  }

  IAN_TRACE(logger_, "SSL handshake complete for client: {}:{}", LOG_SOCKET_TUPLE);

  // Read HTTP header
  http::async_read(
      stream_.next_layer(), read_buffer_, request_,
      asio::bind_executor(strand_, std::bind(&WsConnection::on_read_request, SHARED_FROM_THIS,
                                             std::placeholders::_1)));
}

void WsConnection::on_read_request(boost::system::error_code ec)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  if (read_buffer_.size() > 0)
  {
    IAN_WARN(logger_, "Data left in buffer after request read (size {}) for client: {}:{}",
             read_buffer_.size(), LOG_SOCKET_TUPLE);
    read_buffer_ = boost::beast::flat_buffer();
  }

  if (front::active_connection_count > front::connection_limit_soft)
  {
    IAN_DEBUG(logger_, "Client denied (Overloaded): {}:{}", LOG_SOCKET_TUPLE);

    // TODO http
    http::response<http::string_body> response;
    response.result(http::status::service_unavailable);
    response.body() = "Overloaded";
    http::write(stream_.next_layer(), response, ec);
    shutdown();
  }
  else if (ws::is_upgrade(request_))
  {
    IAN_DEBUG(logger_, "Upgrading to websocket for client: {}:{}", LOG_SOCKET_TUPLE);

    // Accept the websocket handshake
    stream_.async_accept(
        request_, asio::bind_executor(strand_, std::bind(&WsConnection::on_ws_handshake,
                                                         SHARED_FROM_THIS, std::placeholders::_1)));
  }
  else
  {
    IAN_INFO(logger_, "Processing http request for client: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             request_.method_string(), request_.target());

    // TODO http
    http::response<http::string_body> response;
    response.result(http::status::not_implemented);
    response.body() = "Not implemented";
    http::write(stream_.next_layer(), response, ec);
    shutdown();
  }
}

void WsConnection::on_ws_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  // Clear request data (not used anymore)
  request_ = decltype(request_)();

  if (ec)
  {
    IAN_WARN(logger_, "Websocket handshake failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             ec.message(), ec.value());
    shutdown();
    return;
  }

  IAN_TRACE(logger_, "Websocket handshake complete for client: {}:{}", LOG_SOCKET_TUPLE);

  stream_.binary(true);
  stream_.auto_fragment(front::ws_message_auto_fragment);
  stream_.read_message_max(front::message_max_size);

  // Set control handler
  // No need to get a shared_ptr instance (callback is called within the strand)
  // We need to put it in a variable before calling control_callback (must be a reference)
  // FIXME Disabled: fixed in https://github.com/boostorg/beast/pull/957 (not released in boost)
  /*auto control_cb = std::bind(&WsConnection::on_control_frame, this, std::placeholders::_1,
                              std::placeholders::_2);
  stream_.control_callback(control_cb);*/

  // Refresh timeout
  set_timeout(std::chrono::seconds(front::ws_timeout + 5));

  // Start reading packets
  stream_.async_read(
      read_buffer_,
      asio::bind_executor(strand_, std::bind(&WsConnection::on_read, SHARED_FROM_THIS,
                                             std::placeholders::_1, std::placeholders::_2)));
}

void WsConnection::on_control_frame(ws::frame_type type, boost::string_view data)
{
  // Rate limit
  rate_limit_messages_++;
  rate_limit_bytes_ += data.length();

  if (check_rate_limit())
    return;

  // Reset timeout if required
  if (timer_.expires_from_now() <= std::chrono::seconds(front::ws_timeout))
    set_timeout(std::chrono::seconds(front::ws_timeout + 5));

  switch (type)
  {
  case ws::frame_type::ping: IAN_TRACE(logger_, "Ping received: {}:{}", LOG_SOCKET_TUPLE); break;
  case ws::frame_type::pong: IAN_TRACE(logger_, "Pong received: {}:{}", LOG_SOCKET_TUPLE); break;
  case ws::frame_type::close:
    IAN_TRACE(logger_, "Close received: code {} reason '{}' {}:{}", stream_.reason().code,
              stream_.reason().reason.c_str(), LOG_SOCKET_TUPLE);
    shutdown();
    break;
  }
}

void WsConnection::on_read(boost::system::error_code ec, std::size_t readlen)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  IAN_TRACE(logger_, "Message received from client: {}:{} : len {}", LOG_SOCKET_TUPLE, readlen);

  // Rate limit
  rate_limit_messages_++;
  rate_limit_bytes_ += readlen;

  if (check_rate_limit())
    return;

  if (!stream_.got_binary())
  {
    IAN_ERROR(logger_, "Only binary data is supported: {}:{}", LOG_SOCKET_TUPLE);
    shutdown();
    return;
  }

  // Get data
  std::vector<std::uint8_t> buf(readlen);
  read_buffer_.consume(
      asio::buffer_copy(asio::buffer(buf.data(), buf.size()), read_buffer_.data()));

  // TODO Check readlen == read_buffer.size() ? Guaranteed?

  // Start reading next
  stream_.async_read(
      read_buffer_,
      asio::bind_executor(strand_, std::bind(&WsConnection::on_read, SHARED_FROM_THIS,
                                             std::placeholders::_1, std::placeholders::_2)));

  // Reset timeout if required
  if (timer_.expires_from_now() <= std::chrono::seconds(front::ws_timeout))
    set_timeout(std::chrono::seconds(front::ws_timeout + 5));

  process_message(Message::from_vector(std::move(buf)));
}


void WsConnection::on_write_message(boost::system::error_code ec, std::size_t writelen)
{
  // Clear data that was begin sent
  message_outbound_ = Message();

  if (ec)
  {
    handle_write_error(ec);
    return;
  }

  IAN_TRACE(logger_, "Message sent to client: {}:{} : len {}", LOG_SOCKET_TUPLE, writelen);

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

void WsConnection::handle_read_error(boost::system::error_code ec)
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

  IAN_WARN(logger_, "Read failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
           ec.value());

  shutdown();
}

void WsConnection::handle_write_error(boost::system::error_code ec)
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

  IAN_WARN(logger_, "Write failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
           ec.value());

  shutdown();
}
