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
S & operator <<(S & s, const HappyIdentifier & id)
{
  s << id.name;
  return s;
}

template<typename S>
S & operator <<(S & s, const HappyType & type)
{
  s << type.identifier;
  if (type.is_array)
    s << '[' << type.array_size << ']';
  return s;
}


template<typename S>
S & operator <<(S & s, const HappyRoot & root)
{
  for (const auto & node : root.childs)
    s << *node << '\n';
  return s;
}

template<typename S>
S & operator <<(S & s, const HappyInclude & inc)
{
  s << "INCLUDE '" << inc.path << "'";
  return s;
}

template<typename S>
S & operator <<(S & s, const HappyData & data)
{
  s << "DATA " << data.identifier << "\n{\n";
  for (const auto & node : data.childs)
    s << ' ' << *node << '\n';
  return s << '}';
}

template<typename S>
S & operator <<(S & s, const HappyDataField & field)
{
  s << field.identifier << ": " << field.type;
  return s;
}


// Dispatcher
template<typename S>
S & operator <<(S & s, const HappyNode & node)
{
  switch(node.type())
  {
    case HappyNodeType::data: return s << static_cast<const HappyData&>(node);
    case HappyNodeType::data_field: return s << static_cast<const HappyDataField&>(node);
    case HappyNodeType::include: return s << static_cast<const HappyInclude&>(node);
    default: break;
  }
  return s << "???";
}
