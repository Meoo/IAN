/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ws/WsAcceptor.hpp>

#include <bin-common/AsioPool.hpp>
#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigValue.hpp>
#include <common/EasyProfiler.hpp>
#include <common/Platform.hpp>

#include <boost/asio/io_context.hpp>

#include <spdlog/spdlog.h>

#include <csignal>
#include <cstdlib>
#include <thread>


namespace signals
{
bool should_stop   = false;
bool should_reload = false;
AsioPool * asio_pool;

extern "C" void sig_stop(int)
{
  should_stop = true;
  asio_pool->stop();
}

extern "C" void sig_reload(int)
{
  should_reload = true;
  asio_pool->stop();
}
} // namespace signals


int main(int argc, char ** argv)
{
  EASY_MAIN_THREAD;
  plat::set_thread_name("Front main");

  // Init logger
  // spdlog::set_async_mode(512); ?
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
  AsioPool asio_pool(logger);
  boost::asio::io_context & asio =
      asio_pool.createAsio("front", ConfigIntValue("front.threads", 2));

  // Websocket
  {
    auto wslogger = spdlog::create("front.ws", logger->sinks().begin(), logger->sinks().end());
    auto acceptor = std::make_shared<WsAcceptor>(wslogger, asio);
    acceptor->run();
  }


  // Install signal handlers
  signals::asio_pool = &asio_pool;

  std::signal(SIGINT, signals::sig_stop);
  std::signal(SIGTERM, signals::sig_stop);
#ifdef SIGHUP
  std::signal(SIGHUP, signals::sig_reload);
#endif


  // Loop when reloading configuration
  for (;;)
  {
    asio_pool.run();

    if (signals::should_stop)
    {
      logger->info("Received stop signal");
      break;
    }

    if (signals::should_reload)
    {
      if (!config::load())
      {
        logger->critical("Failed to reload config file");
        return 1;
      }

      logger->info("Config reloaded");
      signals::should_reload = false;
      continue;
    }

    logger->error("Asio stopped without signal");
    return 1;
  }

  logger->info("Shutting down");
  return 0;
}
