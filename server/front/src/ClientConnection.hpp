/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/Message.hpp>
#include <bin-common/MessageQueue.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/websocket.hpp>

#include <spdlog/spdlog.h>

#include <memory>


class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
 public:
  using SslContext = boost::asio::ssl::context;
  using TcpSocket  = boost::asio::ip::tcp::socket;
  using SslStream  = boost::asio::ssl::stream<TcpSocket>;
  using WsStream   = boost::beast::websocket::stream<SslStream>;


  ClientConnection(const std::shared_ptr<spdlog::logger> & logger, TcpSocket && socket,
                   SslContext & ssl_ctx);
  ~ClientConnection();

  ClientConnection(const ClientConnection &) = delete;
  ClientConnection & operator=(const ClientConnection &) = delete;


  void run();

  void send_message(const Message & message);


 private:
  std::shared_ptr<spdlog::logger> logger_;

  bool dropped_ = false;
  bool is_writing_ = false;

  WsStream stream_;
  TcpSocket & socket_;

  boost::asio::io_context::strand strand_;
  boost::asio::steady_timer timer_;

  // Outbound
  MessageQueue message_queue_;
  Message message_outbound_;

  // Inbound
  boost::beast::http::request<boost::beast::http::string_body> request_;
  boost::beast::flat_buffer read_buffer_;


  void abort();
  void shutdown();


  void do_write_message(Message && message);


  void set_timeout(const boost::asio::steady_timer::duration & delay);


  void on_timeout(boost::system::error_code ec);
  void on_shutdown(boost::system::error_code ec);

  void on_ssl_handshake(boost::system::error_code ec);
  void on_read_request(boost::system::error_code ec);
  void on_ws_handshake(boost::system::error_code ec);

  void on_read(boost::system::error_code ec, std::size_t readlen);
  void on_write_message(boost::system::error_code ec, std::size_t writelen);


  void handle_read_error(boost::system::error_code ec);
};
