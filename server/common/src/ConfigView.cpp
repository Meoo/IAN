
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
