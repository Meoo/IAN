/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClientListener.hpp"

#include <common/EasyProfiler.hpp>
#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigValue.hpp>

#include <spdlog/spdlog.h>

#include <csignal>
#include <cstdlib>
#include <thread>


namespace signals
{
  bool shouldStop = false;
  bool shouldReload = false;
  boost::asio::io_service* asio;

  extern "C" void sigStop(int)
  {
    shouldStop = true;
    asio->stop();
  }

  extern "C" void sigReload(int)
  {
    shouldReload = true;
    asio->stop();
  }
}


void runAsio(size_t threads, boost::asio::io_service& asio)
{
  // Thread pool
  std::vector<std::thread> pool;

  if (threads > 1)
  {
    pool.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
    {
      pool.emplace_back(
        [&asio]
        {
          EASY_THREAD_SCOPE("ASIO Worker");
          asio.run();
        }
      );
    }
  }

  asio.run();

  for (auto& thread : pool)
  {
    thread.join();
  }
}


int main(int argc, char** argv)
{
  EASY_MAIN_THREAD;


  // Init logger
  //spdlog::set_async_mode(512); ?
  auto logger = spdlog::stdout_color_mt("front");
  std::atexit(spdlog::drop_all);

  // Load config file
  if (!config::load())
  {
    logger->critical("Failed to read config file");
    spdlog::drop_all();
    return 1;
  }


  // ASIO & listener
  ConfigIntValue threads("front.threads", 2);
  boost::asio::io_service asio(threads);

  {
    auto listener = std::make_shared<ClientListener>(logger, asio);
    listener->run();
  }


  // Install signal handlers
  signals::asio = &asio;

  std::signal(SIGINT, signals::sigStop);
  std::signal(SIGTERM, signals::sigStop);
#ifdef SIGHUP
  std::signal(SIGHUP, signals::sigReload);
#endif


  // Loop when reloading configuration
  for(;;)
  {
    logger->info("Ready with {} thread(s)", threads);

    runAsio(threads, asio);

    if (signals::shouldStop)
    {
      logger->info("Received stop signal");
      break;
    }

    if (signals::shouldReload)
    {
      if (!config::load())
      {
        logger->critical("Failed to reload config file");
        return 1;
      }

      logger->info("Config reloaded");
      asio.reset();
      signals::shouldReload = false;
      continue;
    }

    logger->error("ASIO stopped without signal");
    return 1;
  }

  logger->info("Shutting down");
  return 0;
}
