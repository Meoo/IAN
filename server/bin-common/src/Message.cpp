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
#include <cstdlib>


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


const void * Message::get_payload() const
{
  switch (buffer_type_)
  {
  case BufferType::Allocated:
    return (const void *)&data_->message.payload_ptr;

  case BufferType::Flatbuffer:
    return (const void *)data_fb_->data();

  default:
    return nullptr;
  }
}

size_t Message::get_payload_size() const
{
  switch (buffer_type_)
  {
  case BufferType::Allocated:
    return data_->payload_size;

  case BufferType::Flatbuffer:
    return data_fb_->size();

  default:
    return 0;
  }
}


Message Message::from_flatbuffer(Message::Type type, flatbuffers::FlatBufferBuilder & builder)
{
  Message ret(type);
  ret.buffer_type_ = BufferType::Flatbuffer;
  ret.data_fb_ = std::make_shared<flatbuffers::DetachedBuffer>(builder.Release());
  return ret;
}
