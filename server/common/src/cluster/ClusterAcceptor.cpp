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

#include <common/Ssl.hpp>

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
  std::string listen_addr = config_.get_string("ip", ::default_listen_ip);
  int listen_port         = config_.get_int("port", ::default_listen_port);
  bool reuse_addr         = config_.get_bool("reuse_addr", false);
  safe_link_              = config_.get_bool("safe_link", false);

  ip::tcp::endpoint endpoint(ip::make_address(listen_addr), listen_port);

  boost::system::error_code ec;

  do
  {
    if (reuse_addr)
    {
      acceptor_.set_option(boost::asio::ip::tcp::socket::reuse_address(true), ec);
      if (ec)
      {
        IAN_ERROR(logger_, "Failed to set reuse address option on acceptor: {}", ec.message());
        break;
      }
    }

    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
      IAN_ERROR(logger_, "Failed to open acceptor: {}", ec.message());
      break;
    }

    acceptor_.bind(endpoint, ec);
    if (ec)
    {
      IAN_ERROR(logger_, "Acceptor failed to bind to {}:{} : {}", listen_addr, listen_port,
                ec.message());
      acceptor_.close(ec);
      break;
    }

    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if (ec)
    {
      IAN_ERROR(logger_, "Acceptor failed to listen for peers: {}", ec.message());
      acceptor_.close(ec);
      break;
    }

  } while (false);

  if (ec)
  {
    IAN_DEBUG(logger_, "Acceptor open will retry in 10s");

    // Delay before retry
    timer_.expires_from_now(std::chrono::seconds(10));
    timer_.async_wait(std::bind(&ClusterAcceptor::open, shared_from_this()));
  }
  else
  {
    // All green
    IAN_INFO(logger_, "Acceptor listening for peers: {}:{}", listen_addr, listen_port);
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
    IAN_ERROR(logger_, "Accept failed: {}", ec.message());
  }
  else
  {
    IAN_INFO(logger_, "Accepting peer {}:{}", socket_.remote_endpoint().address().to_string(),
             socket_.remote_endpoint().port());

    auto peer = std::make_shared<ClusterConnection>(logger_, std::move(socket_));
    peer->run(ClusterConnection::Server, safe_link_);
  }

  accept_next();
}
