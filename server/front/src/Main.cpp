
#include "Listener.hpp"

#include <common/Config.hpp>
#include <common/EasyProfiler.hpp>

#include <spdlog/spdlog.h>

#include <thread>


int main(int argc, char** argv)
{
  EASY_MAIN_THREAD;


  // Init logger
  //spdlog::set_async_mode(128); ?
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
    auto listener = std::make_shared<Listener>(asio);
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

  spdlog::drop_all();

  return 0;
}
