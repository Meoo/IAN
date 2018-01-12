/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WsAcceptor.hpp"

#include "WsConnection.hpp"

#include <Ssl.hpp>

#include <bin-common/config/Config.hpp>
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
    : logger_(logger), ssl_context_(ssl::context::sslv23), acceptor_(asio), socket_(asio)
{
}

void WsAcceptor::run()
{
  init_ssl_context(ssl_context_);

  // Config
  std::string listenAddr = config::get_string("front.ws.listen_ip", ::default_listen_ip);
  int listenPort         = config::get_int("front.ws.listen_port", ::default_listen_port);

  ip::tcp::endpoint endpoint(ip::make_address(listenAddr), listenPort);

  boost::system::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  if (ec)
  {
    // TODO
    logger_->error("Failed to open acceptor: {}", ec.message());
  }

  acceptor_.bind(endpoint, ec);
  if (ec)
  {
    // TODO
    logger_->error("Acceptor failed to bind to {}:{} : {}", listenAddr, listenPort, ec.message());
  }

  logger_->info("Acceptor bound to {}:{}", listenAddr, listenPort);

  acceptor_.listen(boost::asio::socket_base::max_connections, ec);
  if (ec)
  {
    // TODO
    logger_->error("Acceptor failed to listen for clients: {}", ec.message());
  }

  logger_->info("Acceptor listening for clients");

  do_accept();
}

void WsAcceptor::do_accept()
{
  acceptor_.async_accept(
      socket_, std::bind(&WsAcceptor::on_accept, shared_from_this(), std::placeholders::_1));
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

  do_accept();
}
