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

ListenersData & get_listeners_data()
{
  static ListenersData data;
  return data;
}

void invoke_config_listeners()
{
  auto & data = get_listeners_data();

  std::unique_lock<std::mutex> guard(data.mutex);

  for (auto listener : data.listeners)
    listener->on_update();
}

} // namespace impl
} // namespace config
