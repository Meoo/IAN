/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <common/Message.hpp>


namespace
{

class MessageImplFlatbuffer final : public internal::MessageImpl
{
 public:
  MessageImplFlatbuffer(flatbuffers::DetachedBuffer && buffer) : buffer_(std::move(buffer)) {}

  const void * data() const override { return buffer_.data(); }
  size_t size() const override { return buffer_.size(); }

 private:
  flatbuffers::DetachedBuffer buffer_;
};

class MessageImplVector final : public internal::MessageImpl
{
 public:
  MessageImplVector(std::vector<std::uint8_t> && buffer) : buffer_(std::move(buffer)) {}

  const void * data() const override { return buffer_.data(); }
  size_t size() const override { return buffer_.size(); }

 private:
  std::vector<std::uint8_t> buffer_;
};

} // namespace


Message Message::from_flatbuffer(flatbuffers::FlatBufferBuilder & builder)
{
  return Message(std::make_shared<::MessageImplFlatbuffer>(builder.Release()));
}

Message Message::from_vector(std::vector<std::uint8_t> && data)
{
  return Message(std::make_shared<::MessageImplVector>(std::move(data)));
}
