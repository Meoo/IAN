
project(server-front)

hunter_add_package(Beast)
find_package(Beast CONFIG REQUIRED)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
  src/Main.cpp
  src/Listener.hpp
  src/Listener.cpp
  src/ClientConnection.hpp
  src/ClientConnection.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  protocols-external
  Beast::Beast
  OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()
