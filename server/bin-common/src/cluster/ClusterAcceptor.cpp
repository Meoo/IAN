/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusterAcceptor.hpp"

#include "ClusterConnection.hpp"
#include "ClusterInternal.hpp"

#include <bin-common/Ssl.hpp>

namespace ssl = boost::asio::ssl;
namespace ip  = boost::asio::ip;


namespace
{

const char default_listen_ip[] = "127.0.0.1";
const int default_listen_port  = 17001;

} // namespace


ClusterAcceptor::ClusterAcceptor(const std::shared_ptr<spdlog::logger> & logger,
                                 boost::asio::io_context & asio, const ConfigGroup & config)
    : logger_(logger), config_(config), acceptor_(asio), socket_(asio), timer_(asio)
{
}

void ClusterAcceptor::run() { open(); }

void ClusterAcceptor::open()
{
  // Config
  std::string listenAddr = config_.get_string("ip", ::default_listen_ip);
  int listenPort         = config_.get_int("port", ::default_listen_port);
  bool reuseAddr         = config_.get_bool("reuse_addr", false);

  ip::tcp::endpoint endpoint(ip::make_address(listenAddr), listenPort);

  boost::system::error_code ec;

  do
  {
    if (reuseAddr)
    {
      acceptor_.set_option(boost::asio::ip::tcp::socket::reuse_address(true), ec);
      if (ec)
      {
        logger_->error("Failed to set reuse address option on acceptor: {}", ec.message());
        break;
      }
    }

    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
      logger_->error("Failed to open acceptor: {}", ec.message());
      break;
    }

    acceptor_.bind(endpoint, ec);
    if (ec)
    {
      logger_->error("Acceptor failed to bind to {}:{} : {}", listenAddr, listenPort, ec.message());
      break;
    }

    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if (ec)
    {
      logger_->error("Acceptor failed to listen for peers: {}", ec.message());
      break;
    }

  } while (false);

  if (ec)
  {
    SPDLOG_DEBUG("Acceptor open will retry in 10s");

    // Delay before retry
    timer_.expires_from_now(std::chrono::seconds(10));
    timer_.async_wait(std::bind(&ClusterAcceptor::open, shared_from_this()));
  }
  else
  {
    // All green
    logger_->info("Acceptor listening for peers: {}:{}", listenAddr, listenPort);
    accept_next();
  }
}

void ClusterAcceptor::accept_next()
{
  acceptor_.async_accept(
      socket_, std::bind(&ClusterAcceptor::on_accept, shared_from_this(), std::placeholders::_1));
}

void ClusterAcceptor::on_accept(boost::system::error_code ec)
{
  if (ec)
  {
    // TODO
    logger_->error("Accept failed: {}", ec.message());
  }
  else
  {
    logger_->info("Accepting client {}:{}", socket_.remote_endpoint().address().to_string(),
                  socket_.remote_endpoint().port());

    // Disable Nagle
    boost::asio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay);

    auto client = std::make_shared<ClusterConnection>(logger_, std::move(socket_));
    client->run(ClusterConnection::Server);
  }

  accept_next();
}
