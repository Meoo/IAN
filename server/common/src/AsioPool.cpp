/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <common/AsioPool.hpp>

#include <common/EasyProfiler.hpp>
#include <common/Platform.hpp>

#include <thread>


struct AsioPool::AsioInstance
{
  AsioInstance(const std::string & name, const ConfigIntValue & threads)
      : name(name), asio(threads), work(asio), threads(threads)
  {
  }

  void thread_run()
  {
    std::string thread_name("Asio: " + name);
    EASY_THREAD_SCOPE(thread_name.c_str());
    plat::set_thread_name(thread_name.c_str());

    asio.run();
  }

  std::string name;
  AsioCtx asio;
  AsioCtx::work work;

  ConfigIntValue threads;
  std::vector<std::thread> workers;
};


AsioPool::AsioPool(const std::shared_ptr<spdlog::logger> & logger) : logger_(logger) {}

// Destructor can't be inlined because AsioInstance definition would not be available
AsioPool::~AsioPool() = default;

void AsioPool::run()
{
  // Start all threads
  for (auto & instance : asios_)
  {
    IAN_INFO(logger_, "Starting asio '{}' ({} threads)", instance->name, instance->threads);

    instance->asio.restart();
    while (instance->workers.size() < (std::size_t) instance->threads)
      instance->workers.emplace_back(std::bind(&AsioInstance::thread_run, instance.get()));
  }

  // Wait until all threads are finished (blocking)
  for (auto & instance : asios_)
  {
    for (auto & worker : instance->workers)
      worker.join();

    instance->workers.clear();
  }
}

void AsioPool::stop()
{
  IAN_INFO(logger_, "Asio threads will stop");

  for (auto & instance : asios_)
    instance->asio.stop();
}

AsioPool::AsioCtx & AsioPool::create_asio(const std::string & name, const ConfigIntValue & threads)
{
  asios_.emplace_back(std::make_unique<AsioInstance>(name, threads));
  return asios_.back()->asio;
}
