
project(server-bin-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

add_library(${PROJECT_NAME} STATIC
  include/bincommon/Config.hpp
  include/bincommon/ConfigView.hpp
  src/Config.cpp
  src/ConfigView.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  Boost::system)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")
