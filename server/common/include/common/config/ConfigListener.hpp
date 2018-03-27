/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


namespace config
{
namespace impl
{
void invoke_config_listeners();
} // namespace impl
} // namespace config


class ConfigListener
{
 public:
  ConfigListener();
  virtual ~ConfigListener();

  ConfigListener(const ConfigListener &) = delete;
  ConfigListener & operator=(const ConfigListener &) = delete;


 protected:
  friend void config::impl::invoke_config_listeners();

  // Override this function
  // A mutex is held while this function is active, do not perform any
  // action that could create or destroy a ConfigListener (including this)
  // or a deadlock will happen.
  virtual void on_update() = 0;
};
