
project(server-bin-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

set(SOURCE_CONFIG
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

source_group(config FILES ${SOURCE_CONFIG})

add_library(${PROJECT_NAME} STATIC
  ${SOURCE_CONFIG}
  include/bin-common/Dispatcher.hpp
  include/bin-common/Message.hpp
  include/bin-common/MessageQueue.hpp
  src/Message.cpp
  src/MessageQueue.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  server-internal-protocol
  Boost::system)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")
