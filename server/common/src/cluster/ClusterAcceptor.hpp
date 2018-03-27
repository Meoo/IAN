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
#include <boost/asio/steady_timer.hpp>

#include <memory>


class ClusterAcceptor : public std::enable_shared_from_this<ClusterAcceptor>
{
 public:
  using TcpAcceptor = boost::asio::ip::tcp::acceptor;
  using TcpSocket   = boost::asio::ip::tcp::socket;


  ClusterAcceptor(const std::shared_ptr<spdlog::logger> & logger, boost::asio::io_context & asio,
                  const ConfigGroup & config);

  ClusterAcceptor(const ClusterAcceptor &) = delete;
  ClusterAcceptor & operator=(const ClusterAcceptor &) = delete;


  void run();


 private:
  std::shared_ptr<spdlog::logger> logger_;
  ConfigGroup config_;

  bool safe_link_;

  TcpAcceptor acceptor_;
  TcpSocket socket_;

  boost::asio::steady_timer timer_;


  void open();

  void accept_next();
  void on_accept(boost::system::error_code ec);
};
