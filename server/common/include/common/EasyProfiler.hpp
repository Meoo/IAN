
#ifndef IAN_EASY_PROFILER_HEADER
#define IAN_EASY_PROFILER_HEADER

#ifdef IAN_ENABLE_PROFILING
#  include <easy/profiler.h>
#else

# define EASY_BLOCK(...)
# define EASY_NONSCOPED_BLOCK(...)
# define EASY_FUNCTION(...)
# define EASY_END_BLOCK
# define EASY_PROFILER_ENABLE
# define EASY_PROFILER_DISABLE
# define EASY_EVENT(...)
# define EASY_THREAD(...)
# define EASY_THREAD_SCOPE(...)
# define EASY_MAIN_THREAD
# define EASY_SET_EVENT_TRACING_ENABLED(isEnabled)
# define EASY_SET_LOW_PRIORITY_EVENT_TRACING(isLowPriority)

namespace profiler {
  inline void startListen(int = 0) {}
  inline void stopListen() {}
}

#endif

#endif
