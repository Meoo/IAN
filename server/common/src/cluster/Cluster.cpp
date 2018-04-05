/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <common/Cluster.hpp>

#include "ClusterAcceptor.hpp"
#include "ClusterInternal.hpp"
#include "ClusterWatcher.hpp"
#include <common/Ssl.hpp>


namespace
{
std::shared_ptr<spdlog::logger> cluster_logger;
ConfigGroup cluster_config("cluster"); // TODO default constructor, or just leave it

boost::asio::io_context * cluster_asio;

boost::asio::ssl::context cluster_ssl(boost::asio::ssl::context::sslv23);
} // namespace

//

namespace cluster
{

void init(const std::shared_ptr<spdlog::logger> & logger, AsioPool & pool,
          const ConfigGroup & config)
{
  ::cluster_logger = logger;
  ::cluster_config = config;
  ::cluster_asio   = &pool.create_asio("cluster", config.get_int("threads", 2));

  init_ssl_context(logger.get(), config.get_group("ssl"), ::cluster_ssl);

  for (auto & group : config.get_childs("listen"))
  {
    auto acceptor = std::make_shared<ClusterAcceptor>(logger, *::cluster_asio, group);
    acceptor->run();
  }

  for (auto & group : config.get_childs("connect"))
  {
    auto watcher = std::make_shared<ClusterWatcher>(
        logger, *::cluster_asio,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(group.get_string("ip")),
                                       group.get_int("port", 17001)),
        group.get_bool("safe_link", false));
    watcher->run();
  }
}


namespace internal
{

boost::asio::ssl::context & get_ssl() { return ::cluster_ssl; }

} // namespace internal

} // namespace cluster
