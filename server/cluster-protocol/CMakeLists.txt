
project(server-cluster-protocol)


ian_add_protocol(
  NAME ${PROJECT_NAME}
  INCLUDE "proto-cluster"
  HASH "IAN_PROTO_CLUSTER_HASH"
  RPC clusterRpc.cmake
  FILES
    ClusterHandshake.fbs
    ClusterMessage.fbs
    Status.fbs
  )
