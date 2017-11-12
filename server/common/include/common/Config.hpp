
#include <string>


namespace config
{
  // Call only once in main
  bool init(const std::string& file = std::string());

  bool getBool(const std::string& key, bool default_val);
  std::string getString(const std::string& key, const std::string& default_val = std::string());
  int getInt(const std::string& key, int default_val = 0);
}
