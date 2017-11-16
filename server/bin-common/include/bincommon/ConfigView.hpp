/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
