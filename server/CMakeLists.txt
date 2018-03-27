
if(IAN_BUILD_SERVER)
  add_subdirectory(cluster-protocol)
  add_subdirectory(common)
  add_subdirectory(front)
  add_subdirectory(game)
  add_subdirectory(master)
  add_subdirectory(node)
endif()
