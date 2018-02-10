/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <client/Client.hpp>
#include <client/ClientConnection.hpp>

#include <bin-common/Message.hpp>
#include <bin-common/MessageQueue.hpp>
#include <common/Log.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/websocket.hpp>

#include <chrono>
#include <memory>


class WsConnection final : public ClientConnection
{
 public:
  using SslContext = boost::asio::ssl::context;
  using TcpSocket  = boost::asio::ip::tcp::socket;
  using SslStream  = boost::asio::ssl::stream<TcpSocket>;
  using WsStream   = boost::beast::websocket::stream<SslStream>;


  WsConnection(const std::shared_ptr<spdlog::logger> & logger, TcpSocket && socket,
               SslContext & ssl_ctx);
  ~WsConnection();


  void run();


 protected:
  void send_message(const Message & message) override;


 private:
  std::shared_ptr<spdlog::logger> logger_;

  bool dropped_       = false;
  bool shutting_down_ = false;
  bool is_writing_    = false;

  WsStream stream_;
  TcpSocket & socket_;

  boost::asio::io_context::strand strand_;
  boost::asio::steady_timer timer_;
  boost::asio::steady_timer state_timer_;

  // Outbound
  MessageQueue message_queue_;
  Message message_outbound_;

  // Inbound
  boost::beast::http::request<boost::beast::http::string_body> request_;
  boost::beast::flat_buffer read_buffer_;

  // Inbound rate limiter
  std::chrono::steady_clock::time_point rate_limit_start_;
  int rate_limit_messages_ = 0;
  size_t rate_limit_bytes_ = 0;


  void abort();
  void shutdown();


  void do_write_message(Message && message);


  void set_timeout(const boost::asio::steady_timer::duration & delay);
  void set_state_timeout(const boost::asio::steady_timer::duration & delay);
  void cancel_state_timeout();

  bool check_rate_limit(); // Return true on abort


  void on_timeout(boost::system::error_code ec);
  void on_shutdown(boost::system::error_code ec);

  void on_ssl_handshake(boost::system::error_code ec);
  void on_read_request(boost::system::error_code ec);
  void on_ws_handshake(boost::system::error_code ec);

  void on_control_frame(boost::beast::websocket::frame_type type, boost::string_view data);

  void on_read(boost::system::error_code ec, std::size_t readlen);
  void on_write_message(boost::system::error_code ec, std::size_t writelen);


  void handle_read_error(boost::system::error_code ec);
  void handle_write_error(boost::system::error_code ec);
};
