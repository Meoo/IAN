
project(server-internal-protocol)


ian_add_protocol(
  NAME ${PROJECT_NAME}
  INCLUDE "proto-in"
  FILES
    CoreMessage.fbs
    ClusterHandshake.fbs
    Status.fbs
  )
