
#include <common/Config.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>


namespace
{
  boost::property_tree::ptree config_tree;
}


namespace config
{
  bool init(const std::string& file)
  {
    try
    {
      boost::property_tree::read_info(
        file.empty() ? "config.info" : file, ::config_tree);
    }
    catch (...)
    {
      return false;
    }
    return true;
  }

  bool getBool(const std::string& key, bool default_val)
  {
    return ::config_tree.get(key, default_val);
  }

  std::string getString(const std::string& key, const std::string& default_val)
  {
    return ::config_tree.get(key, default_val);
  }

  int getInt(const std::string& key, int default_val)
  {
    return ::config_tree.get(key, default_val);
  }
}
