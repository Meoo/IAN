/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#include <spdlog/spdlog.h>

#include <memory>


class ClientListener : public std::enable_shared_from_this<ClientListener>
{
 public:
  using SslContext  = boost::asio::ssl::context;
  using TcpAcceptor = boost::asio::ip::tcp::acceptor;
  using TcpSocket   = boost::asio::ip::tcp::socket;


  ClientListener(const std::shared_ptr<spdlog::logger> & logger, boost::asio::io_context & asio);

  ClientListener(const ClientListener &) = delete;
  ClientListener & operator=(const ClientListener &) = delete;


  void run();


 private:
  std::shared_ptr<spdlog::logger> logger_;

  SslContext ssl_context_;
  TcpAcceptor acceptor_;
  TcpSocket socket_;


  void do_accept();
  void on_accept(boost::system::error_code ec);
};
