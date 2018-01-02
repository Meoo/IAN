/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>


class Message
{
 public:
  // Construct a null message
  Message() = default;

  Message(const Message &) = default;
  Message(Message &&)      = default;
  Message & operator=(const Message &) = default;


  // If is_null returns true, calling a get_ function is undefined behavior
  bool is_null() const { return !data_; }

  // Message, including header
  const void * get_message() const;
  size_t get_message_size() const;

  // Actual message payload
  void * get_payload();
  const void * get_payload() const;
  size_t get_payload_size() const;


  // Static methods to construct messages
  static Message create_empty(size_t payload_size);


 private:
  struct MessageData;

  std::shared_ptr<MessageData> data_;
};
