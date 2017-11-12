
project(server-common)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

add_library(${PROJECT_NAME} STATIC
  include/common/Config.hpp
  include/common/ConfigView.hpp
  include/common/EasyProfiler.hpp
  src/Config.cpp
  src/ConfigView.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  spdlog::spdlog
  Boost::system)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "iancommon")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()

if(IAN_ENABLE_EASY_PROFILER)
  find_package(easy_profiler REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC easy_profiler)
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_ENABLE_PROFILING)
endif()
