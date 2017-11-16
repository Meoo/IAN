
project(server-front)

hunter_add_package(Beast)
find_package(Beast CONFIG REQUIRED)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
  src/Main.cpp
  src/ClientListener.hpp
  src/ClientListener.cpp
  src/ClientConnection.hpp
  src/ClientConnection.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-bin-common
  Beast::Beast
  OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()
