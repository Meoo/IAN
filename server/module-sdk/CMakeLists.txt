
project(server-module-sdk)

add_library(${PROJECT_NAME} STATIC
  common.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianmodulesdk")
