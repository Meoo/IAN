
#include <beast/websocket.hpp>
#include <beast/http/string_body.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <spdlog/spdlog.h>

#include <memory>


class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
public:
  using SslContext = boost::asio::ssl::context;
  using TcpSocket = boost::asio::ip::tcp::socket;
  using SslStream = boost::asio::ssl::stream<TcpSocket&>;
  using WsStream = beast::websocket::stream<SslStream>;


  ClientConnection(TcpSocket && socket, SslContext & ssl_ctx);

  ClientConnection(const ClientConnection&) = delete;
  ClientConnection& operator=(const ClientConnection&) = delete;


  void run();


private:
  std::shared_ptr<spdlog::logger> logger_;

  TcpSocket socket_;
  WsStream stream_;

  boost::asio::io_service::strand strand_;
  boost::asio::steady_timer timer_;

  beast::http::request<beast::http::string_body> request_;
  beast::multi_buffer read_buffer_;
  beast::multi_buffer write_buffer_;


  void set_timeout(const boost::asio::steady_timer::duration & delay);


  void on_timer(boost::system::error_code ec);

  void on_ssl_handshake(boost::system::error_code ec);
  void on_read_request(boost::system::error_code ec);
  void on_ws_handshake(boost::system::error_code ec);

  void on_read(boost::system::error_code ec);
  void on_write(boost::system::error_code ec);

};