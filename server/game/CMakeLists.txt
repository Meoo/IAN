
project(server-game)

add_executable(${PROJECT_NAME}
  server_game.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "game")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")
