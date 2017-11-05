
project(server-module-sdk)

add_library(${PROJECT_NAME} STATIC
  include/modulesdk/EasyProfiler.hpp
  include/modulesdk/IModule.hpp
  include/modulesdk/IModuleHost.hpp
  common.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianmodulesdk")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()

if(IAN_ENABLE_EASY_PROFILER)
  find_package(easy_profiler REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC easy_profiler)
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_ENABLE_PROFILING)
endif()
