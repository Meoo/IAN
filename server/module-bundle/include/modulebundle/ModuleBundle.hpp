/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <modulesdk/IModule.hpp>
#include <modulesdk/IModuleHost.hpp>


#ifdef IAN_BUNDLE_SHARED

// https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
# ifdef IAN_BUNDLE_BUILD
#   ifdef __GNUC__
#     define IAN_BUNDLE_API __attribute__ ((dllexport))
#   else
#     define IAN_BUNDLE_API __declspec(dllexport)
#   endif
# else
#   ifdef __GNUC__
#     define IAN_BUNDLE_API __attribute__ ((dllimport))
#   else
#     define IAN_BUNDLE_API __declspec(dllimport)
#   endif
# endif
#else
# if __GNUC__ >= 4
#   define IAN_BUNDLE_API __attribute__ ((visibility ("default")))
# else
#   define IAN_BUNDLE_API
# endif
#endif

#else

#define IAN_BUNDLE_API

#endif


namespace modbundle
{
  IAN_BUNDLE_API void load_modules(IModuleHost * host);
}
