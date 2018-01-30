/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusterInternal.hpp"
#include <bin-common/Cluster.hpp>

#include <bin-common/Ssl.hpp>


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
  ::cluster_asio   = &pool.createAsio("cluster", config.get_int("threads", 2));

  init_ssl_context(logger.get(), config.get_group("ssl"), cluster_ssl);
}


namespace internal
{

boost::asio::ssl::context & get_ssl() { return ::cluster_ssl; }

} // namespace internal

} // namespace cluster
