/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Dispatcher.hpp>

#include <memory>


class ClientState : public std::enable_shared_from_this<ClientState>
{
 public:
 private:
  int state_timeout_ = 0;
  Dispatcher<256, bool, const void *, size_t> message_dispatcher_;
};
