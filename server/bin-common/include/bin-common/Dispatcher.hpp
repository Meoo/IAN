/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>
#include <functional>


template<size_t Count, typename Return, typename... Args >
class Dispatcher
{
 public:
  using Handler = std::function<Return(const Args &...)>;


  void set_default_handler(Handler && handler) { default_handler_ = std::move(handler); }

  void set_handler(size_t code, Handler && handler) { handlers_[code] = std::move(handler); }


  Return dispatch(size_t code, const Args &... args)
  {
    auto & handler = handlers_[code];

    if (handler)
      return handler(args...);

    return post_default(args...);
  }

  Return dispatch_default(const Args &... args) { return default_handler_(args...); }


 private:
  Handler default_handler_;
  std::array<Handler, Count> handlers_;
};
