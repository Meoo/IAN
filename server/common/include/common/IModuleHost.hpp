/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Log.hpp>


class IModuleHost
{
 public:
  IModuleHost()          = default;
  virtual ~IModuleHost() = default;

  IModuleHost(const IModuleHost &) = delete;
  IModuleHost & operator=(const IModuleHost &) = delete;


  virtual std::shared_ptr<spdlog::logger> createLogger(const std::string & module_name);
};
