/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Log.hpp>

#include <bin-common/Message.hpp>
#include <bin-common/MessageQueue.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/multi_buffer.hpp>

#include <memory>


class ClusterConnection : public std::enable_shared_from_this<ClusterConnection>
{
 public:
  enum SslRole
  {
    Client,
    Server
  };

  using SslContext = boost::asio::ssl::context;
  using TcpSocket  = boost::asio::ip::tcp::socket;
  using SslStream  = boost::asio::ssl::stream<TcpSocket>;


  ClusterConnection(const std::shared_ptr<spdlog::logger> & logger, TcpSocket && socket);
  ~ClusterConnection();


  void run(SslRole role, bool safe_link);


 protected:
  void send_message(const Message & message);


 private:
  std::shared_ptr<spdlog::logger> logger_;

  bool dropped_       = false;
  bool shutting_down_ = false;
  bool is_writing_    = false;
  bool safe_link_;
  bool downgraded_ = false;

  SslStream stream_;
  TcpSocket & socket_;

  boost::asio::io_context::strand strand_;
  boost::asio::steady_timer timer_;

  // Outbound
  MessageQueue message_queue_;
  Message message_outbound_;
  uint32_t out_message_len_le_; // Little-endian

  // Inbound
  uint32_t in_message_len_ = 0;
  boost::beast::multi_buffer read_buffer_;


  void abort();
  void shutdown();

  void read_next();


  void do_write_message(Message && message);


  void on_shutdown(boost::system::error_code ec);

  void on_ssl_handshake(boost::system::error_code ec);
  void on_ian_handshake_sent(boost::system::error_code ec, std::size_t writelen);
  void on_ian_handshake(boost::system::error_code ec, std::size_t readlen);

  void on_read(boost::system::error_code ec, std::size_t readlen);
  void on_write_message(boost::system::error_code ec, std::size_t writelen);


  void handle_read_error(boost::system::error_code ec);
  void handle_write_error(boost::system::error_code ec);
};
