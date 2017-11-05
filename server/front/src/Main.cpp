
#include "Listener.hpp"

#include <common/EasyProfiler.hpp>

#include <thread>

int main(int argc, char** argv)
{
  EASY_MAIN_THREAD;

  const size_t threads = 2;

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

  return 0;
}
