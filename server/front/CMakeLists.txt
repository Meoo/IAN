
project(server-front)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

set(SOURCE_CL
  src/client/Client.hpp
  src/client/Client.cpp
  src/client/ClientConnection.hpp
  src/client/ClientState.hpp
  src/client/ClientState.cpp)

source_group(client FILES ${SOURCE_CL})

set(SOURCE_WS
  src/ws/WsAcceptor.hpp
  src/ws/WsAcceptor.cpp
  src/ws/WsConnection.hpp
  src/ws/WsConnection.cpp)

source_group(ws FILES ${SOURCE_WS})

add_executable(${PROJECT_NAME}
  ${SOURCE_CL}
  ${SOURCE_WS}
  src/Main.cpp
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
