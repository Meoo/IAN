/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>


struct HappyIdentifier
{
  std::string name;
};

using HappyString = std::string;
using HappyInteger = int64_t;
using HappyNumber = double;

struct HappyType
{
  HappyIdentifier identifier;

  bool is_array;
  HappyInteger array_size;
};


enum class HappyNodeType
{
  root,

  include,
  alias,
  data,
  data_field,
  map,
  enco,

  comment
};


class HappyNode
{
 public:
  virtual ~HappyNode() = default;
  HappyNodeType type() const { return type_; }

 protected:
  HappyNode(HappyNodeType type) : type_(type) {}

 private:
  const HappyNodeType type_;
};


class HappyContainer : public HappyNode
{
 protected:
  HappyContainer(HappyNodeType type) : HappyNode(type) {}

 public:
  template<typename T, typename... Args>
  T * emplace(Args &&... args)
  {
    auto sptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto ptr  = sptr.get();
    childs_.emplace_back(std::move(sptr));
    return ptr;
  }

 private:
  std::vector<std::unique_ptr<HappyNode>> childs_;
};

class HappyRoot : public HappyContainer
{
 public:
  HappyRoot() : HappyContainer(HappyNodeType::root) {}
};

class HappyData : public HappyContainer
{
 public:
  HappyData(HappyIdentifier identifier) : HappyContainer(HappyNodeType::data), identifier(identifier) {}

  HappyIdentifier identifier;
};


class HappyComment : public HappyNode
{
 public:
  HappyComment(std::string content) : HappyNode(HappyNodeType::comment), content(content) {}

  std::string content;
};

class HappyDataField : public HappyNode
{
 public:
  HappyDataField(HappyIdentifier identifier, HappyType type) : HappyNode(HappyNodeType::data_field), identifier(identifier), type(type) {}

  HappyIdentifier identifier;
  HappyType type;
};
