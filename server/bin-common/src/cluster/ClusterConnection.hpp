/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include <spdlog/spdlog.h>

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


  void run(SslRole role);


 private:
  std::shared_ptr<spdlog::logger> logger_;

  bool dropped_       = false;
  bool shutting_down_ = false;
  bool is_writing_    = false;

  SslStream stream_;
  TcpSocket & socket_;

  boost::asio::io_context::strand strand_;
  boost::asio::steady_timer timer_;


  void on_ssl_handshake(boost::system::error_code ec);
};
