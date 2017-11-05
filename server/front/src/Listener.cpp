
#include "Listener.hpp"

#include "ClientConnection.hpp"

#include <common/EasyProfiler.hpp>

namespace ssl = boost::asio::ssl;
namespace ip = boost::asio::ip;


Listener::Listener(boost::asio::io_service & asio)
  : ssl_context_(ssl::context::sslv23)
  , acceptor_(asio)
  , socket_(asio)
{
  // Config
  ip::tcp::endpoint endpoint(ip::address::from_string("0.0.0.0"), 8080);

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

    ssl_context_.set_password_callback(std::bind([] { return "IANkey"; })); // Use bind to ignore args
    ssl_context_.use_certificate_chain_file("cert.pem");
    ssl_context_.use_private_key_file("cert.pem", boost::asio::ssl::context::pem);
    ssl_context_.use_tmp_dh_file("dh.pem");

    if (SSL_CTX_set_cipher_list(ssl_context_.native_handle(),
        "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS"
        ) != 1)
    {
      // TODO
    }
  }
  catch (std::exception&)
  {
    // TODO
  }

  // Init acceptor
  boost::system::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  if (ec)
  {
    // TODO
  }

  acceptor_.bind(endpoint, ec);
  if (ec)
  {
    // TODO
  }
}

void Listener::run()
{
  boost::system::error_code ec;

  acceptor_.listen(
    boost::asio::socket_base::max_connections, ec);
  if (ec)
  {
    // TODO
  }

  do_accept();
}

void Listener::do_accept()
{
  acceptor_.async_accept(
    socket_,
    std::bind(
      &Listener::on_accept,
      shared_from_this(),
      std::placeholders::_1));
}

void Listener::on_accept(boost::system::error_code ec)
{
  EASY_FUNCTION();

  if (ec)
  {
    // TODO
  }
  else
  {
    auto client = std::make_shared<ClientConnection>(std::move(socket_), ssl_context_);
    client->run();
  }

  do_accept();
}
