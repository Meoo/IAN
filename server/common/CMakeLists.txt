
project(server-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

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
  include/common/Dispatcher.hpp
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
  Boost::system
  OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()
