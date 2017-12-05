/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigGroup.hpp>


ConfigGroup::ConfigGroup(const std::string& group)
  : prefix_(group + ".")
{
}

bool ConfigGroup::get_bool(const std::string& key, bool default_val)
{
  return config::get_bool(prefix_ + key, default_val);
}

std::string ConfigGroup::get_string(const std::string& key, const std::string& default_val)
{
  return config::get_string(prefix_ + key, default_val);
}

int ConfigGroup::get_int(const std::string& key, int default_val)
{
  return config::get_int(prefix_ + key, default_val);
}

ConfigGroup ConfigGroup::get_subgroup(const std::string& group)
{
  return ConfigGroup(prefix_ + group);
}
