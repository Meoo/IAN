/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClientListener.hpp"

#include <common/EasyProfiler.hpp>
#include <bincommon/Config.hpp>

#include <spdlog/spdlog.h>

#include <thread>


int main(int argc, char** argv)
{
  EASY_MAIN_THREAD;


  // Init logger
  //spdlog::set_async_mode(512); ?
  auto logger = spdlog::stdout_color_mt("front");

  // Init config
  if (!config::init())
  {
    logger->critical("Failed to read config file");
    return 1;
  }


  const size_t threads = config::getInt("front.threads", 2);

  logger->info("Front starting with {} thread(s)...", threads);


  boost::asio::io_service asio(threads);

  // Start listener
  {
    auto listener = std::make_shared<ClientListener>(asio);
    listener->run();
  }

  // Thread pool
  std::vector<std::thread> v;

  if (threads > 1)
  {
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
    {
      v.emplace_back(
        [&asio]
        {
          EASY_THREAD_SCOPE("ASIO Worker");
          asio.run();
        }
      );
    }
  }

  asio.run();

  logger->info("Front shutting down");
  spdlog::drop_all();

  return 0;
}
