/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClientConnection.hpp"

#include <common/EasyProfiler.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "FrontGlobals.hpp"


namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ws   = boost::beast::websocket;


#define LOG_SOCKET_TUPLE                                                                           \
  socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port()


ClientConnection::ClientConnection(const std::shared_ptr<spdlog::logger> & logger,
                                   TcpSocket && socket, SslContext & ssl_ctx)
    : logger_(logger), stream_(std::move(socket), ssl_ctx),
      socket_(stream_.next_layer().next_layer()), strand_(socket_.get_io_context()),
      timer_(socket_.get_io_context()), state_timer_(socket_.get_io_context())
{
}

ClientConnection::~ClientConnection()
{
  // Close must be called last or using LOG_SOCKET_TUPLE will throw
  boost::system::error_code ec;
  socket_.close(ec);
}

void ClientConnection::run()
{
  // 3s timeout for connection setup
  set_timeout(std::chrono::seconds(front::ws_setup_timeout));

  // SSL handshake
  stream_.next_layer().async_handshake(
      asio::ssl::stream_base::server,
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_ssl_handshake,
                                             shared_from_this(), std::placeholders::_1)));
}

void ClientConnection::send_message(const Message & message)
{
  if (message.is_null())
  {
    logger_->error("send_message: message is null");
    return;
  }

  // Execute in strand
  asio::dispatch(strand_, [this, self{shared_from_this()}, message{message}]() mutable {
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

void ClientConnection::abort()
{
  if (dropped_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  state_timer_.cancel(ec);

  message_queue_.clear();

  logger_->info("Client disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  dropped_ = true;
}

void ClientConnection::shutdown()
{
  if (dropped_)
    return;

  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);
  state_timer_.cancel(ec);

  message_queue_.clear();

  if (!socket_.is_open())
    return;

  // Shutdown stream at SSL level
  stream_.next_layer().async_shutdown(
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_shutdown, shared_from_this(),
                                             std::placeholders::_1)));
}

void ClientConnection::do_write_message(Message && message)
{
  message_outbound_ = std::move(message);

  stream_.async_write(
      asio::buffer(message_outbound_.get_message(), message_outbound_.get_message_size()),
      asio::bind_executor(strand_,
                          std::bind(&ClientConnection::on_write_message, shared_from_this(),
                                    std::placeholders::_1, std::placeholders::_2)));
}

void ClientConnection::set_timeout(const asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  timer_.cancel(ec); // Errors ignored

  timer_.expires_from_now(delay);
  timer_.async_wait(
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_timeout, shared_from_this(),
                                             std::placeholders::_1)));
}

void ClientConnection::set_state_timeout(const asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  state_timer_.cancel(ec); // Errors ignored

  state_timer_.expires_from_now(delay);
  state_timer_.async_wait(
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_timeout, shared_from_this(),
                                             std::placeholders::_1)));
}

void ClientConnection::cancel_state_timeout()
{
  boost::system::error_code ec;
  state_timer_.cancel(ec); // Errors ignored
}

void ClientConnection::on_timeout(boost::system::error_code ec)
{
  if (dropped_ || ec == boost::asio::error::operation_aborted)
    return;

  logger_->info("Client timed out: {}:{}", LOG_SOCKET_TUPLE);
  abort();
}

void ClientConnection::on_shutdown(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    if (ec != asio::error::connection_aborted && ec != asio::error::connection_reset)
      logger_->warn("Shutdown error for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
                    ec.value());
  }

  logger_->info("Client disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
}

void ClientConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    logger_->warn("SSL handshake failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
                  ec.value());
    abort();
    return;
  }

  logger_->trace("SSL handshake complete for client: {}:{}", LOG_SOCKET_TUPLE);

  // Read HTTP header
  http::async_read(
      stream_.next_layer(), read_buffer_, request_,
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_read_request, shared_from_this(),
                                             std::placeholders::_1)));
}

void ClientConnection::on_read_request(boost::system::error_code ec)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  if (read_buffer_.size() > 0)
  {
    logger_->warn("Data left in buffer after request read (size {}) for client: {}:{}",
                  read_buffer_.size(), LOG_SOCKET_TUPLE);
    read_buffer_ = boost::beast::flat_buffer();
  }

  if (ws::is_upgrade(request_))
  {
    logger_->trace("Upgrading to websocket for client: {}:{}", LOG_SOCKET_TUPLE);

    // Accept the websocket handshake
    stream_.async_accept(
        request_,
        asio::bind_executor(strand_, std::bind(&ClientConnection::on_ws_handshake,
                                               shared_from_this(), std::placeholders::_1)));
  }
  else
  {
    logger_->trace("Processing http request for client: {}:{}", LOG_SOCKET_TUPLE);

    // TODO http
    http::response<http::string_body> response;
    response.result(http::status::not_implemented);
    response.body() = "Not implemented";
    http::write(stream_.next_layer(), response, ec);
    shutdown();
  }
}

void ClientConnection::on_ws_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  // Clear request data (not used anymore)
  request_ = decltype(request_)();

  if (ec)
  {
    logger_->warn("Websocket handshake failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE,
                  ec.message(), ec.value());
    shutdown();
    return;
  }

  logger_->trace("Websocket handshake complete for client: {}:{}", LOG_SOCKET_TUPLE);

  stream_.binary(true);
  stream_.auto_fragment(front::ws_message_auto_fragment);
  stream_.read_message_max(front::message_max_size);

  // Refresh timeout
  set_timeout(std::chrono::seconds(front::ws_timeout + 5));

  // Start reading packets
  stream_.async_read(
      read_buffer_,
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_read, shared_from_this(),
                                             std::placeholders::_1, std::placeholders::_2)));
}

void ClientConnection::on_read(boost::system::error_code ec, std::size_t readlen)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  logger_->trace("Message read (len: {}) for client: {}:{}", readlen, LOG_SOCKET_TUPLE);

  // Get data
  boost::beast::flat_buffer read_buffer;
  std::swap(read_buffer_, read_buffer);

  // Start reading next
  stream_.async_read(
      read_buffer_,
      asio::bind_executor(strand_, std::bind(&ClientConnection::on_read, shared_from_this(),
                                             std::placeholders::_1, std::placeholders::_2)));

  // Reset timeout if required
  if (timer_.expires_from_now() <= std::chrono::seconds(front::ws_timeout))
    set_timeout(std::chrono::seconds(front::ws_timeout + 5));

  // TODO Process data
  // if(stream_.got_binary())
}


void ClientConnection::on_write_message(boost::system::error_code ec, std::size_t writelen)
{
  // Clear data that was begin sent
  message_outbound_ = Message();

  if (ec)
  {
    // TODO
    return;
  }

  logger_->trace("Message written (len: {}) for client: {}:{}", writelen, LOG_SOCKET_TUPLE);

  if (dropped_)
    return;

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

void ClientConnection::handle_read_error(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec == asio::error::connection_aborted || ec == asio::error::connection_reset)
  {
    abort();
    return;
  }

  logger_->warn("Read failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
                ec.value());

  shutdown();
}
