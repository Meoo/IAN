/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <flatbuffers/flatbuffers.h>

#include <memory>


class Message
{
 public:
  enum class BufferType
  {
    Empty,
    Allocated,
    Flatbuffer,
  };

  // Construct an empty message
  Message() = default;

  Message(const Message &) = default;
  Message(Message &&)      = default;
  Message & operator=(const Message &) = default;


  bool is_empty() const { return buffer_type_ == BufferType::Empty; }

  // Message payload
  const void * get_payload() const;
  size_t get_payload_size() const;


  // Static methods to construct messages
  static Message from_flatbuffer(flatbuffers::FlatBufferBuilder & builder);


 private:
  struct MessageData;

  BufferType buffer_type_ = BufferType::Empty;
  std::shared_ptr<MessageData> data_;
  std::shared_ptr<flatbuffers::DetachedBuffer> data_fb_;
};
