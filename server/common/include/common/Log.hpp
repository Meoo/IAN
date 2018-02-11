/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spdlog/spdlog.h>


#define IAN__LOG_WRAP(x) do{ x ;}while((void)0,0)

#ifdef NDEBUG
#define IAN_TRACE(logger, ...) IAN__LOG_WRAP()
#define IAN_DEBUG(logger, ...) IAN__LOG_WRAP()
#else
#define IAN_TRACE(logger, ...) IAN__LOG_WRAP((logger)->trace(__VA_ARGS__))
#define IAN_DEBUG(logger, ...) IAN__LOG_WRAP((logger)->debug(__VA_ARGS__))
#endif

#define IAN_INFO(logger, ...) IAN__LOG_WRAP((logger)->info(__VA_ARGS__))
#define IAN_WARN(logger, ...) IAN__LOG_WRAP((logger)->warn(__VA_ARGS__))
#define IAN_ERROR(logger, ...) IAN__LOG_WRAP((logger)->error(__VA_ARGS__))
#define IAN_CRITICAL(logger, ...) IAN__LOG_WRAP((logger)->critical(__VA_ARGS__))
