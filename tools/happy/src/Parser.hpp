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


struct DocumentPosition
{
  int line;
  int column;
};


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
  alias_kw,
  include_kw,
  data_kw,
  map_kw,
  enco_kw,
  for_kw,

  // Symbols
  colon,         // :
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
  Parser() = default;

  void process(StreamReader & reader, HappyRoot & root);


 private:
  StreamReader * reader_ = nullptr;
  DocumentPosition pos_{1, 0};

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

  HappyIdentifier parse_identifier();
  HappyString parse_string();
  HappyInteger parse_integer();
  HappyNumber parse_number();

  HappyType parse_type();

  DocumentPosition current_position() const { return pos_; }

  void expect(Symbol expected);
  [[noreturn]] void unexpected(Symbol expected = Symbol::invalid);
  [[noreturn]] void unexpected(const char * context);


  void parse_include(HappyContainer & node);

  void parse_data(HappyContainer & node);
  void parse_data_field(HappyData & node);

  void parse_document(HappyRoot & node);
};
// class Parser
