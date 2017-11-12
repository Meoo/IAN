
#include "ClientListener.hpp"

#include "ClientConnection.hpp"

#include <common/Config.hpp>
#include <common/EasyProfiler.hpp>

namespace ssl = boost::asio::ssl;
namespace ip = boost::asio::ip;


ClientListener::ClientListener(boost::asio::io_service & asio)
  : ssl_context_(ssl::context::sslv23)
  , acceptor_(asio)
  , socket_(asio)
{
  logger_ = spdlog::get("front");

  // Config
  std::string listenAddr = config::getString("front.listen_ip", "0.0.0.0");
  int listenPort = config::getInt("front.listen_port", 7011);

  std::string certChain = config::getString("front.certificate_chain", "cert.pem");
  std::string privKey = config::getString("front.private_key", "cert.pem");
  std::string dh = config::getString("front.dh", "dh.pem");
  std::string password = config::getString("front.private_key_password");

  ip::tcp::endpoint endpoint(ip::address::from_string(listenAddr), listenPort);

  // SSL setup
  try
  {
    ssl_context_.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::no_tlsv1 |
      boost::asio::ssl::context::no_tlsv1_1 |
      boost::asio::ssl::context::no_sslv3 |
      boost::asio::ssl::context::single_dh_use);

    ssl_context_.set_password_callback(std::bind([password] { return password; })); // Use bind to ignore args
    ssl_context_.use_certificate_chain_file(certChain);
    ssl_context_.use_private_key_file(privKey, boost::asio::ssl::context::pem);
    ssl_context_.use_tmp_dh_file(dh);

    if (SSL_CTX_set_cipher_list(ssl_context_.native_handle(),
        "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS"
        ) != 1)
    {
      // TODO
      logger_->error("Failed to initialize SSL cipher list");
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

    auto client = std::make_shared<ClientConnection>(std::move(socket_), ssl_context_);
    client->run();
  }

  do_accept();
}
