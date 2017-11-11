
#include <string>


namespace config
{
  bool init(const std::string& file = std::string());

  bool getBool(const std::string& key, bool default);
  std::string getString(const std::string& key, const std::string& default = std::string());
  int getInt(const std::string& key, int default = 0);
}
