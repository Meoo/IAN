
project(server-front)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

set(SOURCE_WS
  src/ws/WsAcceptor.hpp
  src/ws/WsAcceptor.cpp
  src/ws/WsConnection.hpp
  src/ws/WsConnection.cpp)

source_group(ws FILES ${SOURCE_WS})

add_executable(${PROJECT_NAME}
  ${SOURCE_WS}
  src/Main.cpp
  src/ClientState.hpp
  src/ClientState.cpp
  src/FrontGlobals.hpp
  src/FrontGlobals.cpp
  src/Ssl.hpp
  src/Ssl.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-bin-common
  OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()
