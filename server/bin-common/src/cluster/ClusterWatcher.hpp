/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ClusterConnection.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <spdlog/spdlog.h>

#include <memory>


class ClusterWatcher : public std::enable_shared_from_this<ClusterWatcher>
{
 public:
  using TcpEndpoint = boost::asio::ip::tcp::endpoint;
  using TcpSocket   = boost::asio::ip::tcp::socket;


  ClusterWatcher(const std::shared_ptr<spdlog::logger> & logger, boost::asio::io_context & asio,
                 const TcpEndpoint & endpoint);

  ClusterWatcher(const ClusterWatcher &) = delete;
  ClusterWatcher & operator=(const ClusterWatcher &) = delete;


  void run();


 private:
  std::shared_ptr<spdlog::logger> logger_;

  boost::asio::steady_timer timer_;

  TcpEndpoint endpoint_;

  TcpSocket socket_;
  std::weak_ptr<ClusterConnection> connection_;


  void try_connect();
  void wait(const boost::asio::steady_timer::duration & delay);

  void on_connect(boost::system::error_code ec);
  void on_timer(boost::system::error_code ec);
};
