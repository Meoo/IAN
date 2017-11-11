
cmake_minimum_required(VERSION 3.7)


include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.19.148.tar.gz"
  SHA1 "63631441d472a0f8d80f8efefa02b544193aafea"
  LOCAL
)


project(IAN)

set(IAN_VERSION 1.0.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 17)

# Target Windows 7
set(IAN_WINDOWS_VERSION 0x0601)


function(ian_version_required VER)
  if(${IAN_VERSION} VERSION_LESS ${VER})
    message(FATAL_ERROR "IAN version ${VER} required (current version is ${IAN_VERSION})")
  endif()
endfunction()


# Options
set(IAN_ENABLE_EASY_PROFILER OFF CACHE BOOL "Enable easy_profiler profiling")
set(IAN_EASY_PROFILER_INSTALL_PATH "" CACHE PATH "easy_profiler install path")

if(IAN_ENABLE_EASY_PROFILER AND IAN_EASY_PROFILER_INSTALL_PATH)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${IAN_EASY_PROFILER_INSTALL_PATH}/lib/cmake/easy_profiler)
endif()



add_subdirectory(client)
add_subdirectory(server)

add_subdirectory(protocols)

add_subdirectory(modules)


ian_client_finalize()
ian_server_finalize()
