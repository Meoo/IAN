
cmake_minimum_required(VERSION 3.7)


include(cmake/HunterGate.cmake)
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.19.224.tar.gz"
  SHA1 "ac20d7c592855e38bdc901b1d0e43e7cd01f5470"
  LOCAL
)


project(IAN)

set(IAN_VERSION 1.0.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 14)

# Default install directory is build/install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()


# Target Windows 7
set(IAN_WINDOWS_VERSION 0x0601)


function(ian_version_required VER)
  if(${IAN_VERSION} VERSION_LESS ${VER})
    message(FATAL_ERROR "IAN version ${VER} required (current version is ${IAN_VERSION})")
  endif()
endfunction()


# Options
set(IAN_BUILD_CLIENT ON CACHE BOOL "Build IAN client")
set(IAN_BUILD_SERVER ON CACHE BOOL "Build IAN server")

set(IAN_FLATBUFFERS_USE_TYPESCRIPT ON CACHE BOOL "Build TypeScript instead of JavaScript for flatbuffers")

set(IAN_ENABLE_EASY_PROFILER OFF CACHE BOOL "Enable easy_profiler profiling")
set(IAN_EASY_PROFILER_INSTALL_PATH "" CACHE PATH "easy_profiler install path")

if(IAN_ENABLE_EASY_PROFILER AND IAN_EASY_PROFILER_INSTALL_PATH)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${IAN_EASY_PROFILER_INSTALL_PATH}/lib/cmake/easy_profiler)
endif()

set(IAN_IPO OFF CACHE BOOL "Use interprocedural optimization")

if(IAN_IPO)
  if(${CMAKE_VERSION} VERSION_LESS 3.9)
    message(WARNING "IPO is not supported: CMake version < 3.9")
  else()
    cmake_policy(SET CMP0069 NEW)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUP OUTPUT IPO_ERR)
    if(IPO_SUP)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
      message(WARNING "IPO is not supported: ${IPO_ERR}")
    endif()
  endif()
endif()


# Warnings
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()


add_subdirectory(client)
add_subdirectory(server)

add_subdirectory(modules)


ian_client_finalize()
