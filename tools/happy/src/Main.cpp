/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Ast.hpp"
#include "Parser.hpp"

#include <cstdio>
#include <iostream>


class StdinReader : public StreamReader
{
 public:
   std::size_t read(char * output_ptr, std::size_t length) override
  {
    length = std::fread(output_ptr, 1, length, stdin);

    if (std::feof(stdin))
      return length;

    if (std::ferror(stdin))
      throw std::runtime_error(std::strerror(errno));

    return length;
  }
};


int main()
{
  HappyRoot root;

  try
  {
    Parser parser;
    StdinReader reader;
    parser.process(reader, root);
  }
  catch (std::exception & e)
  {
    std::cerr << "Error : " << e.what() << std::endl;
    return 1;
  }

  std::cout << "OK" << std::endl;
  return 0;
}
