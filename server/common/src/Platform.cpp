/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <common/Platform.hpp>

#ifdef WIN32
#  include <windows.h>
#elif defined __linux__
#  include <sys/prctl.h>
#else
#  include <pthread.h>
#endif

namespace plat
{

// set_thread_name
#ifdef WIN32
// https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#  pragma pack(push, 8)
struct THREADNAME_INFO
{
  DWORD dwType;     // Must be 0x1000.
  LPCSTR szName;    // Pointer to name (in user addr space).
  DWORD dwThreadID; // Thread ID (-1=caller thread).
  DWORD dwFlags;    // Reserved for future use, must be zero.
};
#  pragma pack(pop)
void set_thread_name(const char * thread_name)
{
  THREADNAME_INFO info;
  info.dwType     = 0x1000;
  info.szName     = thread_name;
  info.dwThreadID = GetCurrentThreadId();
  info.dwFlags    = 0;
#  pragma warning(push)
#  pragma warning(disable : 6320 6322)
  __try
  {
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
  }
#  pragma warning(pop)
}
#elif defined __linux__
void set_thread_name(const char * thread_name) { prctl(PR_SET_NAME, thread_name); }
#else
void set_thread_name(const char * thread_name) { pthread_setname_np(thread_name); }
#endif

} // namespace plat
