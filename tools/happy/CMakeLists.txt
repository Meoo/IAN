
project(tools-happy)

add_executable(${PROJECT_NAME}
  src/Ast.hpp
  src/Main.cpp
  src/Parser.cpp
  src/Parser.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "happy")
