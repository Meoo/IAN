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
  std::string identifier;
};

using HappyString = std::string;
using HappyInteger = int64_t;
using HappyNumber = double;


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
  HappyData() : HappyContainer(HappyNodeType::data) {}
};


class HappyComment : public HappyNode
{
 public:
  HappyComment(std::string content) : HappyNode(HappyNodeType::comment), content_(content) {}

  const std::string & content() const { return content_; }

 private:
  const std::string content_;
};
