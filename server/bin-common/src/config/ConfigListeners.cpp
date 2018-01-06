/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ConfigListeners.hpp"


namespace config
{
namespace impl
{

std::mutex & get_mutex()
{
  static std::mutex mutex;
  return mutex;
}
std::set<ConfigListener *> & get_listeners()
{
  static std::set<ConfigListener *> listeners;
  return listeners;
}

void invoke_config_listeners()
{
  std::unique_lock<std::mutex> guard(get_mutex());

  for (auto listener : get_listeners())
    listener->on_update();
}

} // namespace impl
} // namespace config
