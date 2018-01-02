/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/Message.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>


#define PAYLOAD_OFFSET offsetof(Message::MessageData, message.payload_ptr)


struct Message::MessageData
{
  size_t payload_size;

  union
  {
    char message_ptr;
    struct
    {
      int message_type;
      char payload_ptr;
    } message;
  };
};


const void * Message::get_message() const { return (const void *)&data_->message_ptr; }

size_t Message::get_message_size() const { return data_->payload_size + PAYLOAD_OFFSET; }

void * Message::get_payload() { return (void *)&data_->message.payload_ptr; }

const void * Message::get_payload() const { return (const void *)&data_->message.payload_ptr; }

size_t Message::get_payload_size() const { return data_->payload_size; }


Message Message::create_empty(size_t payload_size)
{
  const size_t total_size = payload_size + PAYLOAD_OFFSET;
  Message ret;
  ret.data_ = std::shared_ptr<MessageData>((MessageData *)std::malloc(total_size), std::free);
  std::memset(ret.data_.get(), 0, total_size);
  ret.data_->payload_size = payload_size;
  return ret;
}
