/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClientConnection.hpp"

#include <common/EasyProfiler.hpp>

#include <beast/http/read.hpp>
#include <beast/websocket/ssl.hpp>


#define LOG_SOCKET_TUPLE                                                                           \
  socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port()


ClientConnection::ClientConnection(const std::shared_ptr<spdlog::logger> & logger,
                                   TcpSocket && socket, SslContext & ssl_ctx)
    : logger_(logger), socket_(std::move(socket)), stream_(socket_, ssl_ctx),
      strand_(socket_.get_io_service()), timer_(socket_.get_io_service())
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
  set_timeout(std::chrono::seconds(3));

  // SSL handshake
  stream_.next_layer().async_handshake(
      boost::asio::ssl::stream_base::server,
      strand_.wrap(std::bind(&ClientConnection::on_ssl_handshake, shared_from_this(),
                             std::placeholders::_1)));
}

void ClientConnection::abort()
{
  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);

  if (dropped_)
    return;

  logger_->info("Client disconnected: {}:{}", LOG_SOCKET_TUPLE);

  // Abort stream at socket level
  socket_.shutdown(socket_.shutdown_both, ec);
  dropped_ = true;
}

void ClientConnection::shutdown()
{
  boost::system::error_code ec;

  // Ignore errors
  timer_.cancel(ec);

  if (!socket_.is_open())
    return;

  stream_.next_layer().async_shutdown(strand_.wrap(
      std::bind(&ClientConnection::on_shutdown, shared_from_this(), std::placeholders::_1)));
}

void ClientConnection::set_timeout(const boost::asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  timer_.cancel(ec); // Errors ignored

  timer_.expires_from_now(delay);
  timer_.async_wait(strand_.wrap(
      std::bind(&ClientConnection::on_timeout, shared_from_this(), std::placeholders::_1)));
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
    if (ec != boost::asio::error::connection_aborted && ec != boost::asio::error::connection_reset)
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
  beast::http::async_read(stream_.next_layer(), read_buffer_, request_,
                          strand_.wrap(std::bind(&ClientConnection::on_read_request,
                                                 shared_from_this(), std::placeholders::_1)));
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
    read_buffer_ = beast::flat_buffer();
  }

  if (beast::websocket::is_upgrade(request_))
  {
    logger_->trace("Upgrading to websocket for client: {}:{}", LOG_SOCKET_TUPLE);

    // Accept the websocket handshake
    stream_.async_accept(request_,
                         strand_.wrap(std::bind(&ClientConnection::on_ws_handshake,
                                                shared_from_this(), std::placeholders::_1)));
  }
  else
  {
    logger_->trace("Processing http request for client: {}:{}", LOG_SOCKET_TUPLE);

    // TODO http
    beast::http::response<beast::http::string_body> response;
    response.result(beast::http::status::not_implemented);
    response.body = "Not implemented";
    beast::http::write(stream_.next_layer(), response, ec);
    shutdown();
  }
}

void ClientConnection::on_ws_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    logger_->warn("Websocket handshake failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE,
                  ec.message(), ec.value());
    shutdown();
    return;
  }

  logger_->trace("Websocket handshake complete for client: {}:{}", LOG_SOCKET_TUPLE);

  stream_.binary(true);
  // stream_.auto_fragment(true);
  stream_.read_message_max(0x4000);

  // Refresh timeout
  set_timeout(std::chrono::seconds(30));

  // Start reading packets
  stream_.async_read(read_buffer_,
                     strand_.wrap(std::bind(&ClientConnection::on_read, shared_from_this(),
                                            std::placeholders::_1)));
}

void ClientConnection::on_read(boost::system::error_code ec)
{
  if (ec)
  {
    handle_read_error(ec);
    return;
  }

  // Get data
  beast::flat_buffer read_buffer;
  std::swap(read_buffer_, read_buffer);

  // Start reading next
  stream_.async_read(read_buffer_,
                     strand_.wrap(std::bind(&ClientConnection::on_read, shared_from_this(),
                                            std::placeholders::_1)));

  // Reset timeout if required
  if (timer_.expires_from_now() <= std::chrono::seconds(15))
    set_timeout(std::chrono::seconds(30));

  // TODO Process data
}

void ClientConnection::on_write(boost::system::error_code ec)
{
  if (ec)
  {
    // TODO
    return;
  }

  // TODO
}

void ClientConnection::handle_read_error(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec == boost::asio::error::connection_aborted || ec == boost::asio::error::connection_reset)
  {
    abort();
    return;
  }

  logger_->warn("Read failed for client: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
                ec.value());

  shutdown();
}
