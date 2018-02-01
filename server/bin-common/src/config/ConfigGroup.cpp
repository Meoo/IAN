/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigGroup.hpp>


ConfigGroup::ConfigGroup(const std::string & path) : path_(path) {}

ConfigBoolValue ConfigGroup::get_bool(const std::string & path, bool default_val) const
{
  return ConfigBoolValue(path_ + "." + path, default_val);
}

bool ConfigGroup::get_bool_value(const std::string & path, bool default_val) const
{
  return config::get_bool(path_ + "." + path, default_val);
}

ConfigStringValue ConfigGroup::get_string(const std::string & path,
                                          const std::string & default_val) const
{
  return ConfigStringValue(path_ + "." + path, default_val);
}

std::string ConfigGroup::get_string_value(const std::string & path,
                                          const std::string & default_val) const
{
  return config::get_string(path_ + "." + path, default_val);
}

ConfigIntValue ConfigGroup::get_int(const std::string & path, int default_val) const
{
  return ConfigIntValue(path_ + "." + path, default_val);
}

int ConfigGroup::get_int_value(const std::string & path, int default_val) const
{
  return config::get_int(path_ + "." + path, default_val);
}

ConfigGroup ConfigGroup::get_group(const std::string & path) const
{
  return ConfigGroup(path_ + "." + path);
}

std::vector<ConfigGroup> ConfigGroup::get_childs() const { return config::get_childs(path_); }

std::vector<ConfigGroup> ConfigGroup::get_childs(const std::string & path) const
{
  return config::get_childs(path_ + "." + path);
}
