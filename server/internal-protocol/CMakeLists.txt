
project(server-internal-protocol)


ian_add_protocol(
  TARGET ${PROJECT_NAME}
  INCLUDE "proto-in"
  FILES
    CoreMessage.fbs
    Status.fbs
  )
