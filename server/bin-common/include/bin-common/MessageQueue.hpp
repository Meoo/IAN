/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/Message.hpp>

#include <list>


class MessageQueue
{
 public:
  MessageQueue() = default;

  MessageQueue(const MessageQueue &) = delete;
  MessageQueue & operator=(const MessageQueue &) = delete;


  void push(Message && message);
  bool try_pop(Message & message);

  size_t get_message_count() const { return queue_.size(); }
  size_t get_total_size() const { return total_size_; }


 private:
  std::list<Message> queue_;
  size_t total_size_ = 0;
};
