
rpc_namespace(cluster_rpc)

rpc_interface(Common)

rpc_function(
  NAME hello_world
  IN
    String test
  OUT
    String test
)

rpc_function(
  NAME hello_world2
)

rpc_interface(Other)

rpc_function(
  NAME bla
  IN
    String test1
    String test2
    String test3
  OUT
    String test1
    String test2
    String test3
)
