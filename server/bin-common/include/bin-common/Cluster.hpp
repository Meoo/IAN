/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AsioPool.hpp"
#include "config/ConfigGroup.hpp"

#include <spdlog/spdlog.h>


namespace cluster
{

void init(const std::shared_ptr<spdlog::logger> & logger, AsioPool & pool,
          const ConfigGroup & config);
}
