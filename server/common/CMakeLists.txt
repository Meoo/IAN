
project(server-common)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC
  include/common/EasyProfiler.hpp
  include/common/Log.hpp
  include/common/Platform.hpp
  src/Platform.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  spdlog::spdlog)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "iancommon")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PUBLIC WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()

if(IAN_ENABLE_EASY_PROFILER)
  find_package(easy_profiler REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC easy_profiler)
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_ENABLE_PROFILING)
endif()
