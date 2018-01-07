
project(server-front)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
  src/Main.cpp
  src/ClientAcceptor.hpp
  src/ClientAcceptor.cpp
  src/ClientConnection.hpp
  src/ClientConnection.cpp
  src/ClientState.hpp
  src/ClientState.cpp
  src/FrontGlobals.hpp
  src/FrontGlobals.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-bin-common
  OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()
