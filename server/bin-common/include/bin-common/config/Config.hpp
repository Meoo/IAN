/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>


namespace config
{

// Not thread safe, call only for initialization
bool load(const std::string & file = std::string());

bool get_bool(const std::string & key, bool default_val);
std::string get_string(const std::string & key, const std::string & default_val = std::string());
int get_int(const std::string & key, int default_val = 0);

} // namespace config
