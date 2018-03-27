/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/config/ConfigValue.hpp>
#include <common/Log.hpp>

#include <boost/asio/io_context.hpp>

#include <memory>
#include <string>
#include <vector>


class AsioPool
{
 public:
  using AsioCtx = boost::asio::io_context;


  explicit AsioPool(const std::shared_ptr<spdlog::logger> & logger);
  ~AsioPool();

  AsioPool(const AsioPool &) = delete;
  AsioPool & operator=(const AsioPool &) = delete;


  // Start processing (blocking)
  void run();

  // Stop processing for all asio threads
  void stop();


  AsioCtx & create_asio(const std::string & name, const ConfigIntValue & threads);


 private:
  struct AsioInstance;

  std::shared_ptr<spdlog::logger> logger_;

  std::vector<std::unique_ptr<AsioInstance>> asios_;
};
