/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigGroup.hpp>


ConfigGroup::ConfigGroup(const std::string & key) : key_(key) {}

ConfigBoolValue ConfigGroup::get_bool(const std::string & key, bool default_val) const
{
  return ConfigBoolValue(key_ + "." + key, default_val);
}

bool ConfigGroup::get_bool_value(const std::string & key, bool default_val) const
{
  return config::get_bool(key_ + "." + key, default_val);
}

ConfigStringValue ConfigGroup::get_string(const std::string & key,
                                          const std::string & default_val) const
{
  return ConfigStringValue(key_ + "." + key, default_val);
}

std::string ConfigGroup::get_string_value(const std::string & key,
                                          const std::string & default_val) const
{
  return config::get_string(key_ + "." + key, default_val);
}

ConfigIntValue ConfigGroup::get_int(const std::string & key, int default_val) const
{
  return ConfigIntValue(key_ + "." + key, default_val);
}

int ConfigGroup::get_int_value(const std::string & key, int default_val) const
{
  return config::get_int(key_ + "." + key, default_val);
}

ConfigGroup ConfigGroup::get_subgroup(const std::string & group) const
{
  return ConfigGroup(key_ + "." + group);
}
