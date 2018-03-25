/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// Generated file
#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <type_traits>

namespace @RPC_NAMESPACE@
{

namespace impl
{
// http://en.cppreference.com/w/cpp/types/disjunction
// Remove when C++17
template<class...>
struct Disjunction : std::false_type {};
template<class B1>
struct Disjunction<B1> : B1 {};
template<class B1, class... Bn>
struct Disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, Disjunction<Bn...>> {};

template<class If, class... Interfaces>
using ContainsIf = impl::Disjunction<std::is_same<If, Interfaces>...>;
}

// Return callback

template<class Base, class R, typename T>
class RpcReturn
{
 public:
  explicit inline RpcReturn(Base * ptr) : ptr_(ptr) {}
  inline RpcReturn(RpcReturn && other) : ptr_(other.ptr_), rpc_id_(other.rpc_id_)
  {
    other.ptr_ = nullptr;
  }
  RpcReturn(const RpcReturn &) = delete;
  RpcReturn & operator=(const RpcReturn &) = delete;
  RpcReturn & operator=(RpcReturn && other)
  {
    ptr_ = other.ptr_;
    rpc_id_ = other.rpc_id_;
    other.ptr_ = nullptr;
    return *this;
  }

  inline ~RpcReturn()
  {
    if (ptr_)
      error(0, "RpcReturn object dropped");
  }

  inline void operator()(flatbuffers::FlatBufferBuilder & builder, flatbuffers::Offset<T> data)
  {
    if (!ptr_)
      return;
    auto rpc_rep = CreateRpcReply(builder, rpc_id_, RpcReplyUTraits<T>::enum_value, data.Union());
    static_cast<R *>(ptr_)->emit_rpc_reply(builder, rpc_rep);
    ptr_ = nullptr;
  }

  inline void error(std::uint32_t error_code, const char * error_str)
  {
    if (!ptr_)
      return;
    flatbuffers::FlatBufferBuilder builder;
    auto err     = CreateRpcErrorDirect(builder, error_code, error_str);
    auto rpc_rep = CreateRpcReply(builder, rpc_id_, RpcReplyU::RpcReplyU_RpcError, err.Union());
    static_cast<R *>(ptr_)->emit_rpc_reply(builder, rpc_rep);
    ptr_ = nullptr;
  }

 private:
  Base * ptr_;
  uint32_t rpc_id_ = 0;
};

// Result invoker

class RpcResultInvoker
{
 public:
  RpcResultInvoker() : expected_reply_(RpcReplyU::RpcReplyU_NONE) {}
  RpcResultInvoker(RpcResultInvoker && other)
      : expected_reply_(other.expected_reply_), callback_(other.callback_)
  {
    other.expected_reply_ = RpcReplyU::RpcReplyU_NONE;
  }
  RpcResultInvoker(const RpcResultInvoker &) = delete;
  RpcResultInvoker & operator=(const RpcResultInvoker &) = delete;
  RpcResultInvoker & operator=(RpcResultInvoker && other)
  {
    expected_reply_ = other.expected_reply_;
    callback_ = other.callback_;
    other.expected_reply_ = RpcReplyU::RpcReplyU_NONE;
    return *this;
  }

  template<typename F>
  RpcResultInvoker(RpcReplyU expected_reply, F && callback)
      : expected_reply_(expected_reply), callback_(std::forward<F>(callback))
  {
  }

  void invoke(const RpcReply & reply)
  {
    if (reply.data_type() == RpcReplyU::RpcReplyU_RpcError)
    {
      callback_(nullptr, reply.data_as_RpcError());
      return;
    }

    if (reply.data_type() != expected_reply_)
    {
      // Should never happen
      flatbuffers::FlatBufferBuilder builder;
      auto rep = CreateRpcErrorDirect(builder, 0, "Received invalid result");
      auto err = flatbuffers::GetRoot<RpcError>(builder.GetBufferPointer());
      callback_(nullptr, err);
      return;
    }

    callback_(reply.data(), nullptr);
  }

 private:
  RpcReplyU expected_reply_;
  std::function<void(const void *, const RpcError *)> callback_;
};

// { Interfaces

@RPC_HPP_INTERFACES@// } Interfaces

// Receiver

template<class Base, template<class, bool> class... Interfaces>
class RpcReceiver : public Interfaces<Base, true>...
{
protected:
  inline bool dispatch_rpc_request(const RpcRequest & request)
  {
    switch (request.data_type())
    {
@RPC_HPP_REQUEST_SWITCH@
    default: return false;
    }
  }
private:
@RPC_HPP_REQUEST_DISPATCHERS@
};

// Sender

template<class Base, template<class, bool> class... Interfaces>
class RpcSender : public Interfaces<Base, false>...
{
protected:
  std::uint32_t register_rpc_return_callback(RpcResultInvoker callback) final
  {
    std::unique_lock<std::mutex> lock(mutex_);
    std::uint32_t id = next_id_++;
    result_callbacks_.emplace(std::make_pair(id, std::move(callback)));
    return id;
  }
  inline bool dispatch_rpc_reply(const RpcReply & reply)
  {
    RpcResultInvoker cb;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      auto it = result_callbacks_.find(reply.id());
      if (it == result_callbacks_.end())
        return false;
      cb = std::move(it->second);
      result_callbacks_.erase(it);
    }
    cb.invoke(reply);
  }
private:
  std::mutex mutex_;
  std::uint32_t next_id_ = 1;
  std::map<std::uint32_t, RpcResultInvoker> result_callbacks_;
};

} // namespace @RPC_NAMESPACE@
