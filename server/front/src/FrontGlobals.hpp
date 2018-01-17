/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/config/ConfigValue.hpp>

#include <atomic>


namespace front
{

// Config

// Global
extern ConfigIntValue connection_limit_soft;
extern ConfigIntValue connection_limit_hard;

extern ConfigIntValue message_max_size;
extern ConfigIntValue rate_limit_messages;
extern ConfigIntValue rate_limit_bytes;

// Websockets
extern ConfigBoolValue ws_message_auto_fragment;
extern ConfigIntValue ws_setup_timeout;
extern ConfigIntValue ws_timeout;


// Variables

extern std::atomic<std::size_t> active_connection_count;

} // namespace front
