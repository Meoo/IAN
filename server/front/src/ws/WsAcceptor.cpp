/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WsAcceptor.hpp"

#include "WsConnection.hpp"

#include <FrontGlobals.hpp>

#include <bin-common/config/Config.hpp>
#include <bin-common/Ssl.hpp>
#include <common/EasyProfiler.hpp>

namespace ssl = boost::asio::ssl;
namespace ip  = boost::asio::ip;


namespace
{

const char default_listen_ip[] = "0.0.0.0";
const int default_listen_port  = 7011;

} // namespace


WsAcceptor::WsAcceptor(const std::shared_ptr<spdlog::logger> & logger,
                       boost::asio::io_context & asio)
    : logger_(logger), ssl_context_(ssl::context::sslv23), acceptor_(asio), socket_(asio),
      timer_(asio)
{
}

void WsAcceptor::run()
{
  init_ssl_context(logger_.get(), ConfigGroup("front.ssl"), ssl_context_);

  open();
}

void WsAcceptor::open()
{
  // Config
  std::string listenAddr = config::get_string("front.ws.listen_ip", ::default_listen_ip);
  int listenPort         = config::get_int("front.ws.listen_port", ::default_listen_port);
  bool reuseAddr         = config::get_bool("front.ws.listen_reuse_addr", false);

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

    logger_->info("Acceptor bound to {}:{}", listenAddr, listenPort);

    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if (ec)
    {
      logger_->error("Acceptor failed to listen for clients: {}", ec.message());
      break;
    }

  } while (false);

  if (ec)
  {
    SPDLOG_DEBUG("Acceptor open will retry in 10s");

    // Delay before retry
    timer_.expires_from_now(std::chrono::seconds(10));
    timer_.async_wait(std::bind(&WsAcceptor::open, shared_from_this()));
  }
  else
  {
    // All green
    logger_->info("Acceptor listening for clients");
    accept_next();
  }
}

void WsAcceptor::accept_next()
{
  if (front::active_connection_count >= front::connection_limit_hard)
  {
    logger_->warn("Hard limit reached, closing acceptor for 5s");

    acceptor_.close();

    // Delay before reopen
    timer_.expires_from_now(std::chrono::seconds(5));
    timer_.async_wait(std::bind(&WsAcceptor::on_timer_reopen, shared_from_this()));
  }
  else
  {
    acceptor_.async_accept(
        socket_, std::bind(&WsAcceptor::on_accept, shared_from_this(), std::placeholders::_1));
  }
}

void WsAcceptor::on_accept(boost::system::error_code ec)
{
  EASY_FUNCTION();

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

    auto client = std::make_shared<WsConnection>(logger_, std::move(socket_), ssl_context_);
    client->run();
  }

  if (front::active_connection_count > front::connection_limit_hard / 2)
  {
    // Delay before next accept (20ms = 50 accept/s)
    timer_.expires_from_now(std::chrono::milliseconds(20));
    timer_.async_wait(std::bind(&WsAcceptor::accept_next, shared_from_this()));
  }
  else
  {
    accept_next();
  }
}

void WsAcceptor::on_timer_accept() { accept_next(); }

void WsAcceptor::on_timer_reopen()
{
  if (front::active_connection_count >= front::connection_limit_hard)
  {
    SPDLOG_DEBUG("Not yet under hard limit, will retry in 5s");

    // Delay again
    timer_.expires_from_now(std::chrono::seconds(5));
    timer_.async_wait(std::bind(&WsAcceptor::on_timer_reopen, shared_from_this()));
  }
  else
  {
    // Reopen
    open();
  }
}
