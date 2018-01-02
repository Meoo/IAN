/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/MessageQueue.hpp>


void MessageQueue::push(Message && message)
{
  queue_.emplace_back(std::move(message));
  total_size_ += queue_.back().get_message_size();
}

bool MessageQueue::try_pop(Message & message)
{
  if (queue_.empty())
    return false;

  message = std::move(queue_.back());
  queue_.pop_back();
  total_size_ -= message.get_message_size();
  return true;
}
