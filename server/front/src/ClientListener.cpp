/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClientListener.hpp"

#include "ClientConnection.hpp"

#include <common/EasyProfiler.hpp>
#include <bincommon/Config.hpp>

namespace ssl = boost::asio::ssl;
namespace ip = boost::asio::ip;


namespace
{
  const char default_listen_ip[] = "0.0.0.0";
  const int default_listen_port = 7011;
  const char default_cert[] = "cert.pem";
  // Mozilla modern (as of 12/11/2017) https://wiki.mozilla.org/Security/Server_Side_TLS
  const char default_cipher_list[] = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256";
}


ClientListener::ClientListener(const std::shared_ptr<spdlog::logger> & logger, boost::asio::io_service & asio)
  : logger_(logger)
  , ssl_context_(ssl::context::sslv23)
  , acceptor_(asio)
  , socket_(asio)
{
  // Config
  std::string listenAddr = config::get_string("front.listen_ip", ::default_listen_ip);
  int listenPort = config::get_int("front.listen_port", ::default_listen_port);

  std::string certChain = config::get_string("front.certificate_chain", ::default_cert);
  std::string privKey = config::get_string("front.private_key", ::default_cert);
  std::string dh = config::get_string("front.dh");
  std::string password = config::get_string("front.private_key_password");
  std::string cipherList = config::get_string("front.cipher_list", ::default_cipher_list);

  ip::tcp::endpoint endpoint(ip::address::from_string(listenAddr), listenPort);

  // SSL setup
  try
  {
    ssl_context_.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::no_sslv3 |
      boost::asio::ssl::context::no_tlsv1 |
      boost::asio::ssl::context::no_tlsv1_1 |
      boost::asio::ssl::context::single_dh_use);

    ssl_context_.set_password_callback(std::bind([password] { return password; })); // Use bind to ignore args
    ssl_context_.use_certificate_chain_file(certChain);
    ssl_context_.use_private_key_file(privKey, boost::asio::ssl::context::pem);
    if (!dh.empty())
      ssl_context_.use_tmp_dh_file(dh);

    if (SSL_CTX_set_dh_auto(ssl_context_.native_handle(), 1) != 1)
    {
      logger_->warn("Failed to initialize SSL dh auto");
    }

    if (SSL_CTX_set_ecdh_auto(ssl_context_.native_handle(), 1) != 1)
    {
      logger_->warn("Failed to initialize SSL ecdh auto");
    }

    if (SSL_CTX_set_cipher_list(ssl_context_.native_handle(), cipherList.c_str()) != 1)
    {
      logger_->warn("Failed to initialize SSL cipher list");
    }
  }
  catch (std::exception& ex)
  {
    // TODO
    logger_->error("Failed to initialize SSL context: {}", ex.what());
  }

  // Init acceptor
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
}

void ClientListener::run()
{
  boost::system::error_code ec;

  acceptor_.listen(
    boost::asio::socket_base::max_connections, ec);
  if (ec)
  {
    // TODO
    logger_->error("Acceptor failed to listen for clients: {}", ec.message());
  }

  logger_->info("Acceptor listening for clients");

  do_accept();
}

void ClientListener::do_accept()
{
  acceptor_.async_accept(
    socket_,
    std::bind(
      &ClientListener::on_accept,
      shared_from_this(),
      std::placeholders::_1));
}

void ClientListener::on_accept(boost::system::error_code ec)
{
  EASY_FUNCTION();

  if (ec)
  {
    // TODO
    logger_->error("Accept failed: {}", ec.message());
  }
  else
  {
    logger_->info("Accepting client {}:{}",
      socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());

    // Disable Nagle
    boost::asio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay);

    auto client = std::make_shared<ClientConnection>(logger_, std::move(socket_), ssl_context_);
    client->run();
  }

  do_accept();
}
