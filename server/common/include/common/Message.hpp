/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vector>


namespace internal
{

class MessageImpl
{
 public:
  MessageImpl()          = default;
  virtual ~MessageImpl() = default;

  MessageImpl(const MessageImpl &) = default;
  MessageImpl & operator=(const MessageImpl &) = default;

  virtual const void * data() const = 0;
  virtual size_t size() const       = 0;
};

} // namespace internal


class Message
{
 public:
  // Construct an empty message
  Message() = default;

  Message(const Message &) = default;
  Message(Message &&)      = default;
  Message & operator=(const Message &) = default;
  Message & operator=(Message &&) = default;


  bool is_empty() const { return !impl_; }

  // Message payload
  const void * data() const { return impl_->data(); }
  size_t size() const { return impl_->size(); }


  // Static methods to construct messages
  static Message from_vector(std::vector<std::uint8_t> && data);


  // Internal constructor
  explicit Message(std::shared_ptr<internal::MessageImpl> && impl) : impl_(std::move(impl)) {}

 private:
  std::shared_ptr<internal::MessageImpl> impl_;
};
