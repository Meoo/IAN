
project(server-node)

add_executable(${PROJECT_NAME}
  server_node.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  protocols-internal)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "node")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()
