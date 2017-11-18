
project(server-bin-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

add_library(${PROJECT_NAME} STATIC
  include/bincommon/Config.hpp
  include/bincommon/ConfigView.hpp
  include/bincommon/Ipc.hpp
  src/Config.cpp
  src/ConfigView.cpp
  src/Ipc.cpp
  src/ipc/InternalConnection.cpp
  src/ipc/InternalConnection.hpp
  src/ipc/InternalListener.cpp
  src/ipc/InternalListener.hpp
  src/ipc/InternalWatcher.cpp
  src/ipc/InternalWatcher.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  Boost::system)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")
