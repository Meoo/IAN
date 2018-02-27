/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "FrontGlobals.hpp"


namespace front
{

// Config

// Global
ConfigIntValue connection_limit_soft("front.connection_limit_soft", 1000);
ConfigIntValue connection_limit_hard("front.connection_limit_hard", 1050);

ConfigIntValue message_max_size("front.message_max_size", 0x4000);
ConfigIntValue rate_limit_messages("front.rate_limit_messages", 100);
ConfigIntValue rate_limit_bytes("front.rate_limit_bytes", 0x8000);

// Websockets
ConfigBoolValue ws_message_auto_fragment("front.ws.message_auto_fragment", true);
ConfigIntValue ws_setup_timeout("front.ws.socket_setup_timeout", 3);
ConfigIntValue ws_timeout("front.ws.socket_timeout", 30);


// Variables

std::atomic<std::size_t> active_connection_count;

} // namespace front
