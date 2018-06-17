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

// Not an actual symbol
const char comment_mark = '#';

const char colon_mark         = ':';
const char namespace_sep_mark = '.';
const char curly_open_mark    = '{';
const char curly_close_mark   = '}';
const char bracket_open_mark  = '[';
const char bracket_close_mark = ']';

const char namespace_keyword[] = "NAMESPACE";
const char include_keyword[]   = "INCLUDE";
const char struct_keyword[]    = "STRUCT";
const char map_keyword[]       = "MAPPING";
const char enco_keyword[]      = "ENCODING";

const char * symbol_str(Symbol s)
{
  switch (s)
  {
#define SYMBOL(x)                                                                                  \
  case Symbol::x: return #x
    SYMBOL(identifier);
    SYMBOL(string);
    SYMBOL(integer);
    SYMBOL(number);
    SYMBOL(namespace_kw);
    SYMBOL(include_kw);
    SYMBOL(struct_kw);
    SYMBOL(map_kw);
    SYMBOL(enco_kw);
    SYMBOL(colon);
    SYMBOL(namespace_sep);
    SYMBOL(curly_open);
    SYMBOL(curly_close);
    SYMBOL(bracket_open);
    SYMBOL(bracket_close);
    SYMBOL(eof);
    SYMBOL(error);
#undef SYMBOL
  default: return "???";
  }
}

const AstIdentifier DEFAULT_ENCODING = "DEFAULT";

} // namespace


std::unique_ptr<AstRoot> Parser::parse(const std::string & filename, StreamReader & reader)
{
  Parser p(filename, reader);
  return p.parse_document();
}

//

void Parser::skip_whitespace()
{
  char c = peek_at(0);
  while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == comment_mark)
  {
    // Whitespace
    if (c == ' ' || c == '\t')
      ++pos_.column;

    // EOL
    if (c == '\n')
    {
      pos_.column = 0;
      ++pos_.line;
    }

    // Comment
    if (c == comment_mark)
    {
      // Skip all to EOL
      do
      {
        ++pos_.column;
        ++buffer_offset_;
        c = peek_at(0);
      } while (c != '\0' && c != '\n' && c != '\r');
      continue;
    }

    ++buffer_offset_;
    c = peek_at(0);
  }

  trim_opportunity();
}

void Parser::advance(std::size_t offset)
{
  for (std::size_t i = 0; i < offset; ++i)
  {
    char c = peek_at(i);

    if (c == '\n')
    {
      pos_.column = 0;
      ++pos_.line;
    }
    else
      ++pos_.column;
  }

  buffer_offset_ += offset;
  next_symbol_ = Symbol::invalid;
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

  if ((next_char >= '0' && next_char <= '9') || next_char == '-' || next_char == '+')
  {
    // integer or number
    for (std::size_t off = 1;; ++off)
    {
      next_char = peek_at(off);
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
  SIGN(colon);
  SIGN(namespace_sep);
  SIGN(curly_open);
  SIGN(curly_close);
  SIGN(bracket_open);
  SIGN(bracket_close);
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
    KEYWORD(namespace);
    KEYWORD(include);
    KEYWORD(struct);
    KEYWORD(map);
    KEYWORD(enco);
#undef KEYWORD

    return (next_symbol_ = Symbol::identifier);
  }

  return (next_symbol_ = Symbol::error);
}

void Parser::parse_symbol(Symbol symbol)
{
  expect(symbol);

  switch (symbol)
  {
    // Signs
#define SIGN(x)                                                                                    \
  case Symbol::x: advance(1); break;
    SIGN(colon);
    SIGN(namespace_sep);
    SIGN(curly_open);
    SIGN(curly_close);
    SIGN(bracket_open);
    SIGN(bracket_close);
#undef SIGN

    // Keywords
#define KEYWORD(x)                                                                                 \
  case Symbol::x##_kw: advance(sizeof(x##_keyword) - 1); break
    KEYWORD(namespace);
    KEYWORD(include);
    KEYWORD(struct);
    KEYWORD(map);
    KEYWORD(enco);
#undef KEYWORD

  default: throw std::logic_error("Invalid parse_symbol usage");
  }

  trim_opportunity();
}

//

AstIdentifier Parser::parse_identifier()
{
  expect(Symbol::identifier);

  AstIdentifier ret;

  std::size_t len;
  for (len = 0;; ++len)
  {
    char next_char = peek_at(len);
    if ((next_char >= 'a' && next_char <= 'z') || (next_char >= 'A' && next_char <= 'Z') ||
        (next_char >= '0' && next_char <= '9') || next_char == '_')
      ret += next_char;
    else
      break;
  }

  advance(len);
  return ret;
}

AstQualifiedIdentifier Parser::parse_qualified_identifier()
{
  AstQualifiedIdentifier qual;
  AstIdentifier tmp = parse_identifier();

  while (peek_symbol() == Symbol::namespace_sep)
  {
    parse_symbol(Symbol::namespace_sep);
    qual.space.emplace_back(tmp);
    tmp = parse_identifier();
  }

  qual.name = tmp;
  return qual;
}

AstString Parser::parse_string()
{
  expect(Symbol::string);

  AstString ret;

  int len    = 1;
  char delim = peek_at(0);

  for (;; ++len)
  {
    char next_char = peek_at(len);

    if (next_char == delim)
    {
      ++len;
      break;
    }

    if (next_char == '\\')
    {
      ++len;
      next_char = peek_at(len);
      switch (next_char)
      {
      case 'n': ret += '\n'; break;
      case 't': ret += '\t'; break;
      case '0': ret += '\0'; break;
      default: ret += next_char;
      }
    }
    else
      ret += next_char;
  }

  advance(len);
  return ret;
}

AstInteger Parser::parse_integer()
{
  expect(Symbol::integer);

  AstInteger ret = 0;

  int len       = 0;
  bool negative = false;

  char next_char = peek_at(0);
  if (next_char == '-')
  {
    negative = true;
    ++len;
  }
  else if (next_char == '+')
  {
    ++len;
  }

  for (;; ++len)
  {
    next_char = peek_at(len);

    if (!(next_char >= '0' && next_char <= '9'))
      break;

    ret = ret * 10 + (next_char - '0');
  }

  advance(len);
  return negative ? -ret : ret;
}

AstNumber Parser::parse_number()
{
  if (peek_symbol() == Symbol::integer)
    return (AstNumber)parse_integer();

  expect(Symbol::number);

  AstNumber ret = 0;

  int len       = 0;
  bool negative = false;

  char next_char = peek_at(0);
  if (next_char == '-')
  {
    negative = true;
    ++len;
  }
  else if (next_char == '+')
  {
    ++len;
  }

  for (;; ++len)
  {
    next_char = peek_at(len);

    if (!(next_char >= '0' && next_char <= '9'))
      break;

    ret = ret * 10 + (next_char - '0');
  }

  // Skip dot
  ++len;
  AstNumber fract = 0.1;

  for (;; ++len)
  {
    next_char = peek_at(len);

    if (!(next_char >= '0' && next_char <= '9'))
      break;

    if (next_char != '0')
      ret = ret + (next_char - '0') * fract;
    fract *= 0.1;
  }

  advance(len);
  return negative ? -ret : ret;
}

AstType Parser::parse_type()
{
  AstType ret;
  ret.identifier = parse_qualified_identifier();

  ret.is_array = peek_symbol() == Symbol::bracket_open;
  if (ret.is_array)
  {
    parse_symbol(Symbol::bracket_open);
    ret.array_size = parse_integer();
    parse_symbol(Symbol::bracket_close);
  }

  return ret;
}

//

void Parser::expect(Symbol expected)
{
  if (peek_symbol() != expected)
    unexpected(expected);
}

void Parser::unexpected(Symbol expected /*= Symbol::invalid*/)
{
  char message[1024]{0};
  if (expected != Symbol::invalid)
    std::snprintf(message, sizeof(message), "Unexpected symbol %s, expected %s",
                  ::symbol_str(peek_symbol()), ::symbol_str(expected));
  else
    std::snprintf(message, sizeof(message), "Unexpected symbol %s", ::symbol_str(peek_symbol()));

  throw ParseException(current_position(), message);
}

void Parser::unexpected(const char * context)
{
  char message[1024]{0};
  std::snprintf(message, sizeof(message), "Unexpected symbol %s in %s", ::symbol_str(peek_symbol()),
                context);

  throw ParseException(current_position(), message);
}

//

std::unique_ptr<AstInclude> Parser::parse_include()
{
  DocumentPosition node_pos = current_position();

  parse_symbol(Symbol::include_kw);
  auto node = std::make_unique<AstInclude>(parse_string());
  node->origin = node_pos;
  return node;
}

//

std::unique_ptr<AstStruct> Parser::parse_struct()
{
  DocumentPosition node_pos = current_position();

  parse_symbol(Symbol::struct_kw);
  auto node = std::make_unique<AstStruct>(parse_identifier());
  node->origin = node_pos;

  parse_symbol(Symbol::curly_open);

  Symbol symbol = peek_symbol();
  while (symbol != Symbol::curly_close)
  {
    switch (symbol)
    {
    case Symbol::identifier: node->fields.emplace_back(parse_struct_field()); break;
    default: unexpected("data");
    }

    symbol = peek_symbol();
  }

  parse_symbol(Symbol::curly_close);
  return node;
}
// parse_struct()

std::unique_ptr<AstStructField> Parser::parse_struct_field()
{
  DocumentPosition node_pos = current_position();

  AstIdentifier field_id = parse_identifier();
  parse_symbol(Symbol::colon);
  AstType field_type = parse_type();
  auto node = std::make_unique<AstStructField>(field_id, field_type);
  node->origin = node_pos;
  return node;
}

//

std::unique_ptr<AstMapping> Parser::parse_mapping()
{
  DocumentPosition node_pos = current_position();

  parse_symbol(Symbol::map_kw);
  AstIdentifier struct_id = parse_identifier();
  parse_symbol(Symbol::colon);
  AstQualifiedIdentifier map_category = parse_qualified_identifier();

  auto node = std::make_unique<AstMapping>(struct_id, map_category);
  node->origin = node_pos;

  parse_symbol(Symbol::curly_open);

  Symbol symbol = peek_symbol();
  while (symbol != Symbol::curly_close)
  {
    switch (symbol)
    {
    default: unexpected("mapping");
    }

    symbol = peek_symbol();
  }

  return node;
}

//

std::unique_ptr<AstEncoding> Parser::parse_encoding()
{
  DocumentPosition node_pos = current_position();

  parse_symbol(Symbol::enco_kw);

  AstIdentifier struct_id = parse_identifier();
  AstIdentifier enco_id;
  if (peek_symbol() == Symbol::colon)
  {
    parse_symbol(Symbol::colon);
    enco_id = parse_identifier();
  }
  else
    enco_id = DEFAULT_ENCODING;

  auto node = std::make_unique<AstEncoding>(struct_id, enco_id);
  node->origin = node_pos;

  parse_symbol(Symbol::curly_open);

  Symbol symbol = peek_symbol();
  while (symbol != Symbol::curly_close)
  {
    switch (symbol)
    {
    default: unexpected("encoding");
    }

    symbol = peek_symbol();
  }

  return node;
}

//

std::unique_ptr<AstRoot> Parser::parse_document()
{
  DocumentPosition node_pos = current_position();

  auto node = std::make_unique<AstRoot>();
  node->origin = node_pos;

  while (peek_symbol() == Symbol::include_kw)
    node->includes.emplace_back(parse_include());

  if (peek_symbol() == Symbol::namespace_kw)
  {
    parse_symbol(Symbol::namespace_kw);
    node->space = parse_qualified_identifier();
  }

  Symbol symbol;
  for (;;)
  {
    symbol = peek_symbol();
    switch (symbol)
    {

    case Symbol::struct_kw: node->struct_decls.emplace_back(parse_struct()); break;
    case Symbol::map_kw: node->map_decls.emplace_back(parse_mapping()); break;
    case Symbol::enco_kw: node->enco_decls.emplace_back(parse_encoding()); break;
    case Symbol::eof: return node;
    default: unexpected("document");
    }
  }
}
// parse_document()
