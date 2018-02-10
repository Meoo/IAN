/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusterConnection.hpp"
#include "ClusterInternal.hpp"

#include <common/EasyProfiler.hpp>

#include <boost/asio/bind_executor.hpp>


namespace asio = boost::asio;


#define LOG_SOCKET_TUPLE                                                                           \
  socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port()


ClusterConnection::ClusterConnection(const std::shared_ptr<spdlog::logger> & logger,
                                     TcpSocket && socket)
    : logger_(logger), stream_(std::move(socket), cluster::internal::get_ssl()),
      socket_(stream_.next_layer()), strand_(socket_.get_io_context()),
      timer_(socket_.get_io_context())
{
}

ClusterConnection::~ClusterConnection()
{
  // Close must be called last or using LOG_SOCKET_TUPLE will throw
  boost::system::error_code ec;
  socket_.close(ec);
}

void ClusterConnection::run(SslRole role)
{
  // Force peer verification
  boost::system::error_code ec;
  stream_.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert, ec);
  if (ec)
    IAN_ERROR(logger_, "Failed to force peer verification for cluster connection: : {}:{} : {}",
              LOG_SOCKET_TUPLE, ec.message());

  // SSL handshake
  stream_.async_handshake(
      role == Client ? asio::ssl::stream_base::client : asio::ssl::stream_base::server,
      asio::bind_executor(strand_, std::bind(&ClusterConnection::on_ssl_handshake,
                                             shared_from_this(), std::placeholders::_1)));
}

void ClusterConnection::on_ssl_handshake(boost::system::error_code ec)
{
  if (dropped_)
    return;

  if (ec)
  {
    IAN_WARN(logger_, "SSL handshake failed for peer: {}:{} : {} {}", LOG_SOCKET_TUPLE,
             ec.message(), ec.value());
    // TODO this->abort();
    return;
  }

  IAN_TRACE(logger_, "SSL handshake complete for peer: {}:{}", LOG_SOCKET_TUPLE);

  // TODO
}
