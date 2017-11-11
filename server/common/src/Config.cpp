
#include <common/Config.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>


namespace
{
  namespace pt = boost::property_tree;

  pt::ptree config_tree;

  // Use const accessor for get functions
  inline const pt::ptree& cfg() { return config_tree; }
}


namespace config
{
  bool init(const std::string& file)
  {
    namespace pt = boost::property_tree;

    try
    {
      pt::read_info(file.empty() ? "config.info" : file, config_tree);
    }
    catch (...)
    {
      return false;
    }
    return true;
  }

  bool getBool(const std::string& key, bool default)
  {
    auto it = cfg().find(key);

    if (it == cfg().not_found())
      return default;

    return it->second.get_value<bool>();
  }

  std::string getString(const std::string& key, const std::string& default)
  {
    auto it = cfg().find(key);

    if (it == cfg().not_found())
      return default;

    return it->second.get_value<std::string>();
  }

  int getInt(const std::string& key, int default)
  {
    auto it = cfg().find(key);

    if (it == cfg().not_found())
      return default;

    return it->second.get_value<int>();
  }
}
