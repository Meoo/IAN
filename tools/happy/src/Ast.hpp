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


using AstIdentifier = std::string;

struct AstQualifiedIdentifier
{
  std::vector<AstIdentifier> space;
  AstIdentifier name;
};

using AstString  = std::string;
using AstInteger = int64_t;
using AstNumber  = double;

struct AstType
{
  AstQualifiedIdentifier identifier;

  bool is_array;
  AstInteger array_size;
};


enum class AstNodeType
{
  root,

  include,
  alias,
  data,
  data_field,
  map,
  enco,
};


class AstNode
{
 public:
  virtual ~AstNode() = default;
  AstNodeType type() const { return type_; }

 protected:
  AstNode(AstNodeType type) : type_(type) {}

 private:
  const AstNodeType type_;
};


class AstDataField : public AstNode
{
 public:
  AstDataField(AstIdentifier identifier, AstType type)
      : AstNode(AstNodeType::data_field), identifier(identifier), type(type)
  {
  }

  AstIdentifier identifier;
  AstType type;
};

class AstData : public AstNode
{
 public:
  AstData(AstIdentifier identifier) : AstNode(AstNodeType::data), identifier(identifier) {}

  AstIdentifier identifier;
  std::vector<std::unique_ptr<AstDataField>> fields;
};


class AstInclude : public AstNode
{
 public:
  AstInclude(AstString path) : AstNode(AstNodeType::include), path(path) {}

  AstString path;
};


class AstRoot : public AstNode
{
 public:
  AstRoot() : AstNode(AstNodeType::root) {}

  std::vector<std::unique_ptr<AstInclude>> includes;

  AstQualifiedIdentifier space;
  std::vector<std::unique_ptr<AstData>> data_decls;
};
