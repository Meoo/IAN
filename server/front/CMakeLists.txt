
project(server-front)

hunter_add_package(websocketpp)
find_package(websocketpp CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  server_master.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  protocols-external
  websocketpp::websocketpp)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "front")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})

  # OpenSSL has no debug symbols
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
endif()
