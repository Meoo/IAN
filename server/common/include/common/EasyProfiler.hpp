/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IAN_EASY_PROFILER_HEADER
#define IAN_EASY_PROFILER_HEADER

#ifdef IAN_ENABLE_PROFILING
#  include <easy/profiler.h>
#else

#  define EASY_BLOCK(...)
#  define EASY_NONSCOPED_BLOCK(...)
#  define EASY_FUNCTION(...)
#  define EASY_END_BLOCK
#  define EASY_PROFILER_ENABLE
#  define EASY_PROFILER_DISABLE
#  define EASY_EVENT(...)
#  define EASY_THREAD(...)
#  define EASY_THREAD_SCOPE(...)
#  define EASY_MAIN_THREAD
#  define EASY_SET_EVENT_TRACING_ENABLED(isEnabled)
#  define EASY_SET_LOW_PRIORITY_EVENT_TRACING(isLowPriority)

namespace profiler
{
inline void startListen(int = 0) {} // NOLINT
inline void stopListen() {}         // NOLINT
} // namespace profiler

#endif

#endif
