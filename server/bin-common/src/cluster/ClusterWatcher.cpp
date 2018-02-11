/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusterWatcher.hpp"

#include <bin-common/Ssl.hpp>

namespace ssl = boost::asio::ssl;

#define LOG_SOCKET_TUPLE endpoint_.address().to_string(), endpoint_.port()


ClusterWatcher::ClusterWatcher(const std::shared_ptr<spdlog::logger> & logger,
                               boost::asio::io_context & asio, const TcpEndpoint & endpoint,
                               bool safe_link)
    : logger_(logger), safe_link_(safe_link), endpoint_(endpoint), socket_(asio), timer_(asio)
{
}

void ClusterWatcher::run() { try_connect(); }

void ClusterWatcher::try_connect()
{
  socket_.async_connect(
      endpoint_, std::bind(&ClusterWatcher::on_connect, shared_from_this(), std::placeholders::_1));
}

void ClusterWatcher::wait(const boost::asio::steady_timer::duration & delay)
{
  timer_.expires_from_now(delay);
  timer_.async_wait(
      std::bind(&ClusterWatcher::on_timer, shared_from_this(), std::placeholders::_1));
}

void ClusterWatcher::on_connect(boost::system::error_code ec)
{
  if (ec)
  {
    if (ec == boost::asio::error::timed_out)
      IAN_WARN(logger_, "Timed out: {}:{}", LOG_SOCKET_TUPLE); // TODO Debug ?

    else
      IAN_WARN(logger_, "Connect failed: {}:{} : {} {}", LOG_SOCKET_TUPLE, ec.message(),
               ec.value());
  }
  else
  {
    auto client = std::make_shared<ClusterConnection>(logger_, std::move(socket_));
    client->run(ClusterConnection::Client, safe_link_);
    connection_ = client;
  }

  wait(std::chrono::seconds(30));
}

void ClusterWatcher::on_timer(boost::system::error_code ec)
{
  if (ec == boost::asio::error::operation_aborted)
    return; // TODO Should never happen

  if (connection_.expired())
    try_connect();

  else
    wait(std::chrono::seconds(30));
}
