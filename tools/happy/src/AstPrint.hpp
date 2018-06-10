/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Ast.hpp"


template<typename S>
S & operator <<(S & s, const AstQualifiedIdentifier & id)
{
  for (const auto & space : id.space)
    s << space << '.';
  s << id.name;
  return s;
}

template<typename S>
S & operator <<(S & s, const AstType & type)
{
  s << type.identifier;
  if (type.is_array)
    s << '[' << type.array_size << ']';
  return s;
}


template<typename S>
S & operator <<(S & s, const AstRoot & root)
{
  for (const auto & node : root.includes)
    s << *node << '\n';

  if (!root.space.name.empty())
    s << "NAMESPACE " << root.space << '\n';
  for (const auto & node : root.struct_decls)
    s << *node << '\n';
  return s;
}

template<typename S>
S & operator <<(S & s, const AstInclude & inc)
{
  s << "INCLUDE '" << inc.path << "'";
  return s;
}

template<typename S>
S & operator <<(S & s, const AstStruct & data)
{
  s << "DATA " << data.identifier << "\n{\n";
  for (const auto & node : data.fields)
    s << ' ' << *node << '\n';
  return s << '}';
}

template<typename S>
S & operator <<(S & s, const AstStructField & field)
{
  s << field.identifier << ": " << field.type;
  return s;
}

/*
// Dispatcher
template<typename S>
S & operator <<(S & s, const AstNode & node)
{
  switch(node.type())
  {
    case AstNodeType::data: return s << static_cast<const AstStruct&>(node);
    case AstNodeType::data_field: return s << static_cast<const AstStructField&>(node);
    case AstNodeType::include: return s << static_cast<const AstInclude&>(node);
    default: break;
  }
  return s << "???";
}
*/
