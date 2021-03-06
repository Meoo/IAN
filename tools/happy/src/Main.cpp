/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Ast.hpp"
#include "AstPrint.hpp"
#include "Parser.hpp"

#include <cstdio>
#include <cstring>
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
  std::unique_ptr<AstRoot> root;

  try
  {
    StdinReader reader;
    root = Parser::parse("<stdin>", reader);
  }
  catch (ParseException & e)
  {
    std::cerr << e.position().file << ":" << e.position().line << ":" << e.position().column << ": " << e.what()
              << std::endl;
    return 1;
  }
  catch (std::exception & e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << *root;
  return 0;
}
