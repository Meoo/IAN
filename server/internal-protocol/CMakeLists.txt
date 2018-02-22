
project(server-internal-protocol)


ian_add_protocol(
  NAME ${PROJECT_NAME}
  INCLUDE "proto-in"
  HASH "IAN_PROTO_IN_HASH"
  FILES
    CoreMessage.fbs
    ClusterHandshake.fbs
    Status.fbs
  )
