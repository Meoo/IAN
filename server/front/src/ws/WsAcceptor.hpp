/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/config/ConfigGroup.hpp>
#include <common/Log.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>

#include <memory>


class WsAcceptor : public std::enable_shared_from_this<WsAcceptor>
{
 public:
  using SslContext  = boost::asio::ssl::context;
  using TcpAcceptor = boost::asio::ip::tcp::acceptor;
  using TcpSocket   = boost::asio::ip::tcp::socket;


  WsAcceptor(const std::shared_ptr<spdlog::logger> & logger, boost::asio::io_context & asio,
             const ConfigGroup & config);

  WsAcceptor(const WsAcceptor &) = delete;
  WsAcceptor & operator=(const WsAcceptor &) = delete;


  void run();


 private:
  std::shared_ptr<spdlog::logger> logger_;
  ConfigGroup config_;

  SslContext ssl_context_;
  TcpAcceptor acceptor_;
  TcpSocket socket_;

  boost::asio::steady_timer timer_;


  void open();

  void accept_next();
  void on_accept(boost::system::error_code ec);

  void on_timer_accept();
  void on_timer_reopen();
};
