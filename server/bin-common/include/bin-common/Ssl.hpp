/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/config/ConfigGroup.hpp>
#include <common/Log.hpp>

#include <boost/asio/ssl/context.hpp>


void init_ssl_context(spdlog::logger * logger, const ConfigGroup & config,
                      boost::asio::ssl::context & ssl_context);
