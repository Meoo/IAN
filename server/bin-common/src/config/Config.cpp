/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>

#include <bin-common/config/ConfigGroup.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ConfigListeners.hpp"


namespace
{
inline boost::property_tree::ptree & get_config_tree()
{
  static boost::property_tree::ptree config_tree;
  return config_tree;
}
} // namespace


namespace config
{

bool load(const std::string & file)
{
  try
  {
    boost::property_tree::read_info(file.empty() ? "config.info" : file, ::get_config_tree());

    impl::invoke_config_listeners();
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool get_bool(const std::string & path, bool default_val)
{
  return ::get_config_tree().get(path, default_val);
}

std::string get_string(const std::string & path, const std::string & default_val)
{
  return ::get_config_tree().get(path, default_val);
}

int get_int(const std::string & path, int default_val)
{
  return ::get_config_tree().get(path, default_val);
}

std::vector<ConfigGroup> get_childs(const std::string & path)
{
  std::vector<ConfigGroup> ret;
  boost::property_tree::ptree def;

  const auto & tree = ::get_config_tree().get_child(path, def);

  for (const auto & it : tree)
    ret.emplace_back(path + "." + it.first);

  return ret;
}

} // namespace config
