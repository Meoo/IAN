
rpc_namespace(cluster_rpc)

rpc_interface(Common)

rpc_function(
  NAME hello_world
  IN
    input:string
  OUT
    output:string
)

rpc_function(
  NAME hello_world2
)

rpc_interface(Other)

rpc_function(
  NAME bla
  IN
    test1:string
    test2:int32
    test3:[ubyte]
  OUT
    test1:string
    test2:int32
    test3:[ubyte]
)
