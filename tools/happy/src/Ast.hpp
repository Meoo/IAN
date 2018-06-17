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


struct DocumentPosition
{
  std::string file;
  int line;
  int column;
};


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
  struct_decl,
  struct_field,
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

public:
  DocumentPosition origin;

 private:
  const AstNodeType type_;
};


class AstStructField : public AstNode
{
 public:
  AstStructField(AstIdentifier identifier, AstType type)
      : AstNode(AstNodeType::struct_field), identifier(identifier), type(type)
  {
  }

  AstIdentifier identifier;
  AstType type;
};

class AstStruct : public AstNode
{
 public:
  AstStruct(AstIdentifier identifier) : AstNode(AstNodeType::struct_decl), identifier(identifier) {}

  AstIdentifier identifier;
  std::vector<std::unique_ptr<AstStructField>> fields;
};


class AstMapping : public AstNode
{
 public:
  AstMapping(AstIdentifier struct_id, AstQualifiedIdentifier map_category)
      : AstNode(AstNodeType::map), struct_id(struct_id), map_category(map_category)
  {
  }

  AstIdentifier struct_id;
  AstQualifiedIdentifier map_category;
};


class AstEncoding : public AstNode
{
 public:
  AstEncoding(AstIdentifier struct_id, AstIdentifier enco_id)
      : AstNode(AstNodeType::enco), struct_id(struct_id), enco_id(enco_id)
  {
  }

  AstIdentifier struct_id;
  AstIdentifier enco_id;
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
  std::vector<std::unique_ptr<AstStruct>> struct_decls;
  std::vector<std::unique_ptr<AstMapping>> map_decls;
  std::vector<std::unique_ptr<AstEncoding>> enco_decls;
};
