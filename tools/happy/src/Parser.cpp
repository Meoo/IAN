/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Parser.hpp"


namespace
{

const char comment_mark     = '#';
const char colon_mark       = ':';
const char curly_open_mark  = '{';
const char curly_close_mark = '}';

const char alias_keyword[]   = "ALIAS";
const char include_keyword[] = "INCLUDE";
const char data_keyword[]    = "DATA";
const char map_keyword[]     = "MAP";
const char enco_keyword[]    = "ENCO";
const char for_keyword[]     = "FOR";

} // namespace


void Parser::process(StreamReader & reader, HappyRoot & root)
{
  reader_ = &reader;
  try
  {
    parse_document(root);
  }
  catch (...)
  {
    reader_ = nullptr;
    throw;
  }
  reader_ = nullptr;
}

//

void Parser::skip_whitespace()
{
  char c = peek_at(0);
  while (c == ' ' || c == '\t' || c == '\n' || c == '\r')
  {
    if (c == ' ' || c == '\t')
      ++pos_.column;

    if (c == '\n')
    {
      pos_.column = 0;
      ++pos_.line;
    }

    ++buffer_offset_;
    c = peek_at(0);
  }

  trim_opportunity();
}

void Parser::advance(std::size_t offset)
{
  buffer_offset_ += offset;
  trim_opportunity();
}

char Parser::peek_at(std::size_t offset)
{
  while (buffer_offset_ + offset >= buffer_.size())
  {
    // Read data from stream
    std::size_t initial_size = buffer_.size();
    buffer_.resize(buffer_offset_ + offset + 0x0800);

    std::size_t readlen = reader_->read(&buffer_[initial_size], buffer_.size() - initial_size);
    buffer_.resize(initial_size + readlen);

    if (readlen == 0)
      return '\0';
  }

  return buffer_[buffer_offset_ + offset];
}

void Parser::trim_opportunity()
{
  if (buffer_offset_ >= 0x4000)
  {
    tmp_buffer_.clear();
    tmp_buffer_.reserve(buffer_.size() - buffer_offset_);
    tmp_buffer_.insert(tmp_buffer_.begin(), buffer_.cbegin() + buffer_offset_, buffer_.cend());
    buffer_.swap(tmp_buffer_);
    buffer_offset_ = 0;
  }
}

//

Symbol Parser::peek_symbol()
{
  if (next_symbol_ != Symbol::invalid)
    return next_symbol_;

  skip_whitespace();

  char next_char = peek_at(0);

  if (next_char == '\0')
    return (next_symbol_ = Symbol::eof);

  if (next_char >= '0' && next_char <= '9')
  {
    // integer or number
    for (std::size_t off = 1;; ++off)
    {
      bool number = false;
      next_char   = peek_at(off);
      if ((next_char >= '0' && next_char <= '9') || next_char == '.')
      {
        if (next_char == '.')
          return (next_symbol_ = Symbol::number);
      }
      else
        break;
    }
    return (next_symbol_ = Symbol::integer);
  }

  // Signs
#define SIGN(x)                                                                                    \
  if (next_char == x##_mark)                                                                       \
    return (next_symbol_ = Symbol::x);
  SIGN(comment);
  SIGN(colon);
  SIGN(curly_open);
  SIGN(curly_close);
#undef SIGN

  if (next_char == '\'' || next_char == '"')
    return (next_symbol_ = Symbol::string);

  if ((next_char >= 'a' && next_char <= 'z') || (next_char >= 'A' && next_char <= 'Z'))
  {
    // Keyword or identifier
    std::string str;
    str += next_char;

    for (std::size_t off = 1;; ++off)
    {
      next_char = peek_at(off);
      if ((next_char >= 'a' && next_char <= 'z') || (next_char >= 'A' && next_char <= 'Z') ||
          (next_char >= '0' && next_char <= '9') || next_char == '_')
        str += next_char;
      else
        break;
    }

    // Keywords
#define KEYWORD(x)                                                                                 \
  if (str == x##_keyword)                                                                          \
  return (next_symbol_ = Symbol::x##_kw)
    KEYWORD(alias);
    KEYWORD(include);
    KEYWORD(data);
    KEYWORD(map);
    KEYWORD(enco);
    KEYWORD(for);
#undef KEYWORD

    return (next_symbol_ = Symbol::identifier);
  }

  return (next_symbol_ = Symbol::error);
}

void Parser::parse_symbol(Symbol symbol)
{
  if (peek_symbol() != symbol)
    throw ParseException(current_position());

  switch (symbol)
  {
    // Signs
#define SIGN(x)                                                                                    \
  case Symbol::x: ++buffer_offset_; break;
    SIGN(comment);
    SIGN(colon);
    SIGN(curly_open);
    SIGN(curly_close);
#undef SIGN

    // Keywords
#define KEYWORD(x)                                                                                 \
  case Symbol::x##_kw: buffer_offset_ += sizeof(x##_keyword) - 1; break
    KEYWORD(alias);
    KEYWORD(include);
    KEYWORD(data);
    KEYWORD(map);
    KEYWORD(enco);
    KEYWORD(for);
#undef KEYWORD

  default: throw std::logic_error("Invalid parse_symbol usage");
  }

  trim_opportunity();
}

//

HappyIdentifier Parser::parse_identifier()
{
  if (peek_symbol() != Symbol::identifier)
    unexpected();

  // TODO identifier
  return {};
}

HappyString Parser::parse_string()
{
  if (peek_symbol() != Symbol::string)
    unexpected();

  // TODO string
  return {};
}

HappyInteger Parser::parse_integer()
{
  if (peek_symbol() != Symbol::integer)
    unexpected();

  // TODO integer
  return 0;
}

HappyNumber Parser::parse_number()
{
  if (peek_symbol() == Symbol::integer)
    return parse_integer();

  if (peek_symbol() != Symbol::number)
    unexpected();

  // TODO number
  return 0.0;
}

//

void Parser::unexpected() { throw ParseException(current_position()); }

//

void Parser::parse_comment(HappyContainer & node)
{
  parse_symbol(Symbol::comment);
  // std::string content = get_all();
  node.emplace<HappyComment>("comment content");
}
// parse_comment()

//

void Parser::parse_include(HappyContainer & node) {}

//

void Parser::parse_data(HappyContainer & node)
{
  parse_symbol(Symbol::data_kw);

  auto & data = *node.emplace<HappyData>();
  // TODO data_id
  HappyIdentifier data_id = parse_identifier();

  parse_symbol(Symbol::curly_open);

  Symbol symbol = peek_symbol();
  while (symbol != Symbol::curly_close)
  {
    switch (symbol)
    {
    case Symbol::curly_close: break; // Loop will break

    case Symbol::comment: parse_comment(data); break;
    case Symbol::identifier: parse_data_field(data); break;
    default: unexpected();
    }

    symbol = peek_symbol();
  }
}
// parse_data()

void Parser::parse_data_field(HappyData & node) {}

//

void Parser::parse_document(HappyRoot & node)
{
  Symbol symbol;

  for (;;)
  {
    symbol = peek_symbol();
    switch (symbol)
    {
    case Symbol::comment: parse_comment(node); break;
    case Symbol::include_kw: parse_include(node); break;
    case Symbol::data_kw: parse_data(node); break;
    case Symbol::eof: return;
    default: unexpected();
    }
  }
}
// parse_document()
