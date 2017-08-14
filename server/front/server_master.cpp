
#include <boost/asio.hpp>

#include <websocketpp/config/debug_asio.hpp>
#include <websocketpp/server.hpp>

#include <set>

namespace asio = boost::asio;

using websocketpp::connection_hdl;

using server = websocketpp::server<websocketpp::config::debug_asio_tls>;
using context_ptr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

// See https://wiki.mozilla.org/Security/Server_Side_TLS for more details about
// the TLS modes. The code below demonstrates how to implement both the modern
enum tls_mode {
  MOZILLA_INTERMEDIATE = 1,
  MOZILLA_MODERN = 2
};

class broadcast_server {
public:
  broadcast_server() {
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;

    m_server.init_asio();

    m_server.set_validate_handler(bind(&broadcast_server::on_validate, this, _1));
    m_server.set_open_handler(bind(&broadcast_server::on_open, this, _1));
    m_server.set_close_handler(bind(&broadcast_server::on_close, this, _1));
    m_server.set_message_handler(bind(&broadcast_server::on_message, this, _1, _2));
    m_server.set_tls_init_handler(bind(&broadcast_server::on_tls_init, this, MOZILLA_MODERN, _1));
  }

  bool on_validate(connection_hdl hdl) {
    auto con = m_server.get_con_from_hdl(hdl);
    const auto & subprotos = con->get_requested_subprotocols();

    if (subprotos.size() != 2)
      return false;

    if (subprotos.at(0) != "ian")
      return false;

    // TODO magic

    return true;
  }

  void on_open(connection_hdl hdl) {
    m_connections.insert(hdl);
  }

  void on_close(connection_hdl hdl) {
    m_connections.erase(hdl);
  }

  void on_message(connection_hdl hdl, server::message_ptr msg) {
    for (auto it : m_connections) {
      m_server.send(it, msg);
    }
  }

  context_ptr on_tls_init(tls_mode mode, connection_hdl hdl) {
    using websocketpp::lib::bind;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    std::cout << "using TLS mode: " << (mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate") << std::endl;
    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
      if (mode == MOZILLA_MODERN) {
        // Modern disables TLSv1
        ctx->set_options(asio::ssl::context::default_workarounds |
          asio::ssl::context::no_sslv2 |
          asio::ssl::context::no_sslv3 |
          asio::ssl::context::no_tlsv1 |
          asio::ssl::context::no_tlsv1_1 |
          asio::ssl::context::single_dh_use);
      }
      else {
        ctx->set_options(asio::ssl::context::default_workarounds |
          asio::ssl::context::no_sslv2 |
          asio::ssl::context::no_sslv3 |
          asio::ssl::context::single_dh_use);
      }
      ctx->set_password_callback(bind([]() { return "IANkey"; })); // Use bind to ignore args
      ctx->use_certificate_chain_file("cert.pem");
      ctx->use_private_key_file("cert.pem", asio::ssl::context::pem);

      // Example method of generating this file:
      // `openssl dhparam -out dh.pem 2048`
      // Mozilla Intermediate suggests 1024 as the minimum size to use
      // Mozilla Modern suggests 2048 as the minimum size to use.
      ctx->use_tmp_dh_file("dh.pem");

      const char * ciphers;

      if (mode == MOZILLA_MODERN) {
        ciphers = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256";
      }
      else {
        ciphers = "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS";
      }

      if (SSL_CTX_set_cipher_list(ctx->native_handle(), ciphers) != 1) {
        std::cout << "Error setting cipher list" << std::endl;
      }
    }
    catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << std::endl;
    }
    return ctx;
  }

  void run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
  }
private:
  typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

  server m_server;
  con_list m_connections;
};

int main() {
  broadcast_server server;
  server.run(7011);
}
