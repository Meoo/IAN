/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bin-common/Message.hpp>

#include <memory>


class Client;

class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
 public:
  ClientConnection() = default;
  virtual ~ClientConnection() = default;

  ClientConnection(const ClientConnection &) = delete;
  ClientConnection & operator=(const ClientConnection &) = delete;


 protected:
  // An outbound message should be sent to the peer (implemented by connection)
  virtual void send_message(const Message & message) = 0;

  // An inbound message was received from the peer, forward it to the Client instance
  void process_message(Message && message) {} // TODO


 private:
  std::shared_ptr<Client> client_;
};
