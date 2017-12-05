
project(server-bin-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

add_library(${PROJECT_NAME} STATIC
  include/bin-common/config/Config.hpp
  include/bin-common/config/ConfigGroup.hpp
  include/bin-common/config/ConfigListener.hpp
  include/bin-common/config/ConfigValue.hpp
  src/config/Config.cpp
  src/config/ConfigGroup.cpp
  src/config/ConfigListener.cpp
  src/config/ConfigListeners.cpp
  src/config/ConfigListeners.hpp
  src/config/ConfigValue.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  server-internal-protocol
  Boost::system)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")
