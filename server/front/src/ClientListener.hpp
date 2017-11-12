
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#include <spdlog/spdlog.h>

#include <memory>


class ClientListener : public std::enable_shared_from_this<ClientListener>
{
public:
  using SslContext = boost::asio::ssl::context;
  using TcpAcceptor = boost::asio::ip::tcp::acceptor;
  using TcpSocket = boost::asio::ip::tcp::socket;


  explicit ClientListener(boost::asio::io_service & asio);

  ClientListener(const ClientListener&) = delete;
  ClientListener& operator=(const ClientListener&) = delete;


  void run();


private:
  std::shared_ptr<spdlog::logger> logger_;

  SslContext ssl_context_;
  TcpAcceptor acceptor_;
  TcpSocket socket_;


  void do_accept();
  void on_accept(boost::system::error_code ec);

};