/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <common/Config.hpp>
#include <common/ConfigView.hpp>


ConfigView::ConfigView(const std::string& group)
  : prefix_(group + ".")
{
}

bool ConfigView::getBool(const std::string& key, bool default_val)
{
  return config::getBool(prefix_ + key, default_val);
}

std::string ConfigView::getString(const std::string& key, const std::string& default_val)
{
  return config::getString(prefix_ + key, default_val);
}

int ConfigView::getInt(const std::string& key, int default_val)
{
  return config::getInt(prefix_ + key, default_val);
}

ConfigView ConfigView::subView(const std::string& group)
{
  return ConfigView(prefix_ + group);
}
