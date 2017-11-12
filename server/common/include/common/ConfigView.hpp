
#include <string>


class ConfigView
{
public:
  ConfigView(const std::string& group);


  bool getBool(const std::string& key, bool default_val);
  std::string getString(const std::string& key, const std::string& default_val = std::string());
  int getInt(const std::string& key, int default_val = 0);

  ConfigView subView(const std::string& group);


private:
  std::string prefix_;

};
