/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Ast.hpp"

#include <cstring>
#include <exception>
#include <string>


class ParseException final : public std::exception
{
 public:
  ParseException(DocumentPosition position, const char * message) : position_(position)
  {
    std::strncpy(what_, message, sizeof(what_) - 1);
  }

  const char * what() const noexcept final { return what_; }
  DocumentPosition position() const noexcept { return position_; }

 private:
  char what_[1024]{0};
  DocumentPosition position_;
};


enum class Symbol
{
  identifier,
  string,
  integer,
  number,

  // Keywords
  namespace_kw,
  include_kw,
  struct_kw,
  mapping_kw,
  encoding_kw,
  delta_kw,

  // Symbols
  colon,         // :
  namespace_sep, // .
  curly_open,    // {
  curly_close,   // }
  bracket_open,  // [
  bracket_close, // ]

  // Special
  eof,
  error,
  invalid
};


class StreamReader
{
 public:
  virtual ~StreamReader() = default;

  // length argument is max read length
  // Output is actual read length
  // Return 0 to signal end of stream
  virtual std::size_t read(char * output_ptr, std::size_t length) = 0;
};


class Parser
{
 public:
  static std::unique_ptr<AstRoot> parse(const std::string & filename, StreamReader & reader);


 private:
  Parser(std::string filename, StreamReader & reader) : reader_(&reader), pos_{filename, 1, 0} {}

  StreamReader * reader_ = nullptr;
  DocumentPosition pos_;

  std::vector<char> buffer_;
  std::vector<char> tmp_buffer_;
  std::size_t buffer_offset_ = 0;

  Symbol next_symbol_ = Symbol::invalid;

  //

  void skip_whitespace(); // also skip comments
  void advance(std::size_t offset);
  char peek_at(std::size_t offset); // Return '\0' when eof is reached
  void trim_opportunity();

  Symbol peek_symbol();
  void parse_symbol(Symbol symbol);

  AstIdentifier parse_identifier();
  AstQualifiedIdentifier parse_qualified_identifier();
  AstString parse_string();
  AstInteger parse_integer();
  AstNumber parse_number();

  AstType parse_type();

  DocumentPosition current_position() const { return pos_; }

  void expect(Symbol expected);
  [[noreturn]] void unexpected(Symbol expected = Symbol::invalid);
  [[noreturn]] void unexpected(const char * context);


  std::unique_ptr<AstInclude> parse_include();

  std::unique_ptr<AstStruct> parse_struct();
  std::unique_ptr<AstStructField> parse_struct_field();

  std::unique_ptr<AstMapping> parse_mapping();
  std::unique_ptr<AstMappingField> parse_mapping_field();

  std::unique_ptr<AstEncoding> parse_encoding();
  std::unique_ptr<AstEncodingNode> parse_encoding_node();
  std::unique_ptr<AstEncodingField> parse_encoding_field();
  std::unique_ptr<AstEncodingDeltaBlock> parse_encoding_delta_block();

  std::unique_ptr<AstRoot> parse_document();
};
// class Parser
