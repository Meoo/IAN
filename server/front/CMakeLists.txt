
project(server-front)

set(SOURCE_CL
  src/client/Client.hpp
  src/client/Client.cpp
  src/client/ClientConnection.hpp)

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
  src/FrontGlobals.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common)

target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")
