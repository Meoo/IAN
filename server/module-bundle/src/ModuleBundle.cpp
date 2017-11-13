/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <modulebundle/ModuleBundle.hpp>

#include <modules/mod_headers.gen.h>

#include <memory>


// Better error messages in some cases
namespace modules {}


namespace
{

  class ModuleEntry
  {
  public:
    ModuleEntry(const char * name, std::function<IModule*()> factory)
      : name(name), factory(factory) {}

    const char * name;
    std::function<IModule*()> factory;
  };

  const ModuleEntry module_list[] =
  {
#   define IAN_MODULE_MACRO(mod) {#mod, []() { return static_cast<IModule*>(new modules::mod); }},
#   include <modules/mod_list.gen>
#   undef IAN_MODULE_MACRO
    { nullptr, []() { return nullptr; } }
  };

} // unnamed


namespace modbundle
{

  void load_modules(IModuleHost * host)
  {
  }

}
