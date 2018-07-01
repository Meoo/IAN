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


class AstNode
{
public:
  virtual ~AstNode() = default;

protected:
  AstNode() = default;

public:
  DocumentPosition origin;
};


class AstStructField : public AstNode
{
 public:
  AstStructField(AstIdentifier identifier, AstType type)
      : identifier(identifier), type(type)
  {
  }

  AstIdentifier identifier;
  AstType type;
};

class AstStruct : public AstNode
{
 public:
  AstStruct(AstIdentifier identifier) : identifier(identifier) {}

  AstIdentifier identifier;
  std::vector<std::unique_ptr<AstStructField>> fields;
};


class AstMappingField : public AstNode
{
public:
  AstMappingField(AstIdentifier identifier, AstString mapping_decl)
    : identifier(identifier), mapping_decl(mapping_decl)
  {
  }

  AstIdentifier identifier;
  AstString mapping_decl;
};

class AstMapping : public AstNode
{
 public:
  AstMapping(AstIdentifier struct_id, AstQualifiedIdentifier map_category)
      : struct_id(struct_id), map_category(map_category)
  {
  }

  AstIdentifier struct_id;
  AstQualifiedIdentifier map_category;
  std::vector<std::unique_ptr<AstMappingField>> fields;
};


class AstEncodingNode : public AstNode
{
};

class AstEncodingField : public AstEncodingNode
{
public:
  AstEncodingField(AstIdentifier identifier, AstIdentifier encoding)
    : identifier(identifier), encoding(encoding)
  {
  }

  AstIdentifier identifier;
  AstIdentifier encoding;
};

class AstEncodingDeltaBlock : public AstEncodingNode
{
public:
  std::vector<std::unique_ptr<AstEncodingNode>> fields;
};

class AstEncoding : public AstNode
{
 public:
  AstEncoding(AstIdentifier struct_id, AstIdentifier enco_id)
      : struct_id(struct_id), enco_id(enco_id)
  {
  }

  AstIdentifier struct_id;
  AstIdentifier enco_id;
  std::vector<std::unique_ptr<AstEncodingNode>> fields;
};


class AstInclude : public AstNode
{
 public:
  AstInclude(AstString path) : path(path) {}

  AstString path;
};


class AstRoot : public AstNode
{
 public:
  AstRoot() {}

  std::vector<std::unique_ptr<AstInclude>> includes;

  AstQualifiedIdentifier space;
  std::vector<std::unique_ptr<AstStruct>> struct_decls;
  std::vector<std::unique_ptr<AstMapping>> map_decls;
  std::vector<std::unique_ptr<AstEncoding>> enco_decls;
};
