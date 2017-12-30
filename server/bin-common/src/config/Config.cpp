/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ConfigListeners.hpp"


namespace
{
boost::property_tree::ptree config_tree;
}


namespace config
{

bool load(const std::string & file)
{
  try
  {
    boost::property_tree::read_info(file.empty() ? "config.info" : file, ::config_tree);

    impl::invoke_config_listeners();
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool get_bool(const std::string & key, bool default_val)
{
  return ::config_tree.get(key, default_val);
}

std::string get_string(const std::string & key, const std::string & default_val)
{
  return ::config_tree.get(key, default_val);
}

int get_int(const std::string & key, int default_val)
{
  return ::config_tree.get(key, default_val);
}

} // namespace config
