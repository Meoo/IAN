
#include "ClientConnection.hpp"

#include <common/EasyProfiler.hpp>

#include <beast/http/read.hpp>


ClientConnection::ClientConnection(TcpSocket && socket, SslContext & ssl_ctx)
  : socket_(std::move(socket))
  , stream_(socket_, ssl_ctx)
  , strand_(socket_.get_io_service())
  , timer_(socket_.get_io_service())
{
}

void ClientConnection::run()
{
  // 5s timeout for connection setup
  set_timeout(std::chrono::seconds(5));

  // SSL handshake
  stream_.next_layer().async_handshake(
    boost::asio::ssl::stream_base::server,
    strand_.wrap(
      std::bind(
        &ClientConnection::on_ssl_handshake,
        shared_from_this(),
        std::placeholders::_1)));
}

void ClientConnection::set_timeout(const boost::asio::steady_timer::duration & delay)
{
  boost::system::error_code ec;
  timer_.cancel(ec); // Errors ignored

  timer_.expires_from_now(delay);
  timer_.async_wait(
    strand_.wrap(
      std::bind(
        &ClientConnection::on_timer,
        shared_from_this(),
        std::placeholders::_1)));
}

void ClientConnection::on_timer(boost::system::error_code ec)
{
  if (ec == boost::asio::error::operation_aborted)
    return;

  // Abort stream at socket level
  // Reuse ec, but ignore return
  socket_.shutdown(socket_.shutdown_both, ec);
  socket_.close(ec);
}

void ClientConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (ec)
  {
    // TODO
    return;
  }

  // Read HTTP header
  beast::http::async_read(stream_.next_layer(), read_buffer_, request_,
    strand_.wrap(
      std::bind(
        &ClientConnection::on_read_request,
        shared_from_this(),
        std::placeholders::_1)));
}

void ClientConnection::on_read_request(boost::system::error_code ec)
{
  if (ec)
  {
    // TODO
    return;
  }

  if (beast::websocket::is_upgrade(request_))
  {
    // Accept the websocket handshake
    stream_.async_accept(
      request_,
      strand_.wrap(
        std::bind(
          &ClientConnection::on_ws_handshake,
          shared_from_this(),
          std::placeholders::_1)));
  }
  else
  {
    // TODO http
    beast::http::response<beast::http::string_body> response;
    response.result(beast::http::status::not_implemented);
    response.body = "Not implemented";
    beast::http::write(stream_.next_layer(), response, ec);
    stream_.next_layer().shutdown(ec);
  }
}

void ClientConnection::on_ws_handshake(boost::system::error_code ec)
{
  if (ec)
  {
    // TODO
    return;
  }

  stream_.binary(true);
  //stream_.auto_fragment(true);

  // Refresh timeout
  set_timeout(std::chrono::seconds(30));

}
