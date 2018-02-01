/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/config/Config.hpp>
#include <bin-common/config/ConfigValue.hpp>


// ConfigBoolValue

ConfigBoolValue::ConfigBoolValue(const std::string & key, bool default_val)
    : path_(key), default_val_(default_val), value_(config::get_bool(path_, default_val_))
{
}

ConfigBoolValue::ConfigBoolValue(const ConfigBoolValue & other)
    : path_(other.path_), default_val_(other.default_val_), value_(other.value_)
{
}

void ConfigBoolValue::on_update() { value_ = config::get_bool(path_, default_val_); }


// ConfigStringValue

ConfigStringValue::ConfigStringValue(const std::string & key,
                                     const std::string & default_val /*= std::string()*/)
    : path_(key), default_val_(default_val), value_(config::get_string(path_, default_val_))
{
}

ConfigStringValue::ConfigStringValue(const ConfigStringValue & other)
    : path_(other.path_), default_val_(other.default_val_), value_(other.value_)
{
}

void ConfigStringValue::on_update() { value_ = config::get_string(path_, default_val_); }


// ConfigIntValue

ConfigIntValue::ConfigIntValue(const std::string & key, int default_val /*= 0*/)
    : path_(key), default_val_(default_val), value_(config::get_int(path_, default_val_))
{
}

ConfigIntValue::ConfigIntValue(const ConfigIntValue & other)
    : path_(other.path_), default_val_(other.default_val_), value_(other.value_)
{
}

void ConfigIntValue::on_update() { value_ = config::get_int(path_, default_val_); }
