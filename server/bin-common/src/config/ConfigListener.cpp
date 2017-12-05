/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/ConfigListener.hpp>

#include "ConfigListeners.hpp"


ConfigListener::ConfigListener()
{
  std::unique_lock<std::mutex> guard(config::impl::mutex);
  config::impl::listeners.insert(this);
}

ConfigListener::~ConfigListener()
{
  std::unique_lock<std::mutex> guard(config::impl::mutex);
  config::impl::listeners.erase(this);
}
