
project(server-bin-common)

hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

set(SOURCE_CONFIG
  include/bin-common/config/Config.hpp
  include/bin-common/config/ConfigGroup.hpp
  include/bin-common/config/ConfigListener.hpp
  include/bin-common/config/ConfigValue.hpp
  src/config/Config.cpp
  src/config/ConfigGroup.cpp
  src/config/ConfigListener.cpp
  src/config/ConfigListeners.cpp
  src/config/ConfigListeners.hpp
  src/config/ConfigValue.cpp)

source_group(config FILES ${SOURCE_CONFIG})

set(SOURCE_CLUSTER
  include/bin-common/Cluster.hpp
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
  include/bin-common/AsioPool.hpp
  include/bin-common/Dispatcher.hpp
  include/bin-common/Message.hpp
  include/bin-common/MessageQueue.hpp
  include/bin-common/Ssl.hpp
  src/AsioPool.cpp
  src/Message.cpp
  src/MessageQueue.cpp
  src/Ssl.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-common
  server-cluster-protocol
  Boost::system
  OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbincommon")

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    crypt32.lib)
endif()
