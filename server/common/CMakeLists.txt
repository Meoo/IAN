
project(server-common)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC
  common.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  spdlog::spdlog)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "iancommon")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()
