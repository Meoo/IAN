
project(server-common)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

hunter_add_package(spdlog)
find_package(spdlog REQUIRED)

hunter_add_package(Boost COMPONENTS system date_time)
find_package(Boost REQUIRED COMPONENTS system date_time)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# TODO: Workaround for vcpkg
if(VCPKG_TOOLCHAIN)
  find_package(fmt REQUIRED)
endif()

set(SOURCE_CONFIG
  include/common/config/Config.hpp
  include/common/config/ConfigGroup.hpp
  include/common/config/ConfigListener.hpp
  include/common/config/ConfigValue.hpp
  src/config/Config.cpp
  src/config/ConfigGroup.cpp
  src/config/ConfigListener.cpp
  src/config/ConfigListeners.cpp
  src/config/ConfigListeners.hpp
  src/config/ConfigValue.cpp)

source_group(config FILES ${SOURCE_CONFIG})

set(SOURCE_CLUSTER
  include/common/Cluster.hpp
  src/cluster/Cluster.cpp
  src/cluster/ClusterAcceptor.cpp
  src/cluster/ClusterAcceptor.hpp
  src/cluster/ClusterConnection.cpp
  src/cluster/ClusterConnection.hpp
  src/cluster/ClusterInternal.hpp
  src/cluster/ClusterWatcher.cpp
  src/cluster/ClusterWatcher.hpp)

source_group(cluster FILES ${SOURCE_CLUSTER})

add_library(${PROJECT_NAME} STATIC
  ${SOURCE_CONFIG}
  ${SOURCE_CLUSTER}
  include/common/AsioPool.hpp
  include/common/EasyProfiler.hpp
  include/common/Log.hpp
  include/common/Message.hpp
  include/common/MessageQueue.hpp
  include/common/Platform.hpp
  include/common/Ssl.hpp
  src/AsioPool.cpp
  src/Message.cpp
  src/MessageQueue.cpp
  src/Platform.cpp
  src/Ssl.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-cluster-protocol
  Threads::Threads
  spdlog::spdlog
  Boost::boost Boost::system Boost::date_time
  OpenSSL::SSL OpenSSL::Crypto)

# TODO: Workaround for vcpkg
if(VCPKG_TOOLCHAIN)
  target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "iancommon")

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PUBLIC WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()

if(IAN_ENABLE_EASY_PROFILER)
  find_package(easy_profiler REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC easy_profiler)
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_ENABLE_PROFILING)
endif()
