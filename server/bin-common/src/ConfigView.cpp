/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bincommon/Config.hpp>
#include <bincommon/ConfigView.hpp>


ConfigView::ConfigView(const std::string& group)
  : prefix_(group + ".")
{
}

bool ConfigView::get_bool(const std::string& key, bool default_val)
{
  return config::get_bool(prefix_ + key, default_val);
}

std::string ConfigView::get_string(const std::string& key, const std::string& default_val)
{
  return config::get_string(prefix_ + key, default_val);
}

int ConfigView::get_int(const std::string& key, int default_val)
{
  return config::get_int(prefix_ + key, default_val);
}

ConfigView ConfigView::sub_view(const std::string& group)
{
  return ConfigView(prefix_ + group);
}
