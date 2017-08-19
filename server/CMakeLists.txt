
add_subdirectory(common)
add_subdirectory(node)
add_subdirectory(front)
add_subdirectory(game)
add_subdirectory(master)


set_property(GLOBAL PROPERTY IAN_SERVER_INCLUDES)
set_property(GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
set_property(GLOBAL PROPERTY IAN_SERVER_MODULE_INCLUDES)


function(ian_server_add_module)
  set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULES ${ARGV})
endfunction()

function(ian_server_add_module_lib)
  set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULE_LINKS ${ARGV})
endfunction()

function(ian_server_add_module_header)
  foreach(HEADER_FILE ${ARGV})
    get_filename_component(HEADER_FULLPATH ${HEADER_FILE} ABSOLUTE)
    set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULE_INCLUDES ${HEADER_FULLPATH})
  endforeach()
endfunction()



function(ian_server_finalize)
  message(STATUS "=== IAN server ===")

  set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/modules/include")
  set(GEN_PATH "${INCLUDE_PATH}/modules")

  target_include_directories(server-game PUBLIC ${INCLUDE_PATH})


  get_property(MODULES GLOBAL PROPERTY IAN_SERVER_MODULES)
  list(LENGTH MODULES MODULES_COUNT)
  message(STATUS "* ${MODULES_COUNT} modules")

  file(WRITE "${GEN_PATH}/mod_list.gen" "// IAN generated file, do not edit\n")
  foreach(MODULE ${MODULES})
    file(APPEND "${GEN_PATH}/mod_list.gen" "IAN_MODULE_MACRO(${MODULE})\n")
  endforeach()


  get_property(LIBS GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
  list(LENGTH LIBS LIBS_COUNT)
  message(STATUS "* ${LIBS_COUNT} libraries")

  target_link_libraries(server-game PUBLIC ${LIBS})


  get_property(INCLUDES GLOBAL PROPERTY IAN_SERVER_MODULE_INCLUDES)
  list(LENGTH INCLUDES INCLUDES_COUNT)
  message(STATUS "* ${INCLUDES_COUNT} headers")

  file(WRITE "${GEN_PATH}/mod_headers.gen.h" "// IAN generated file, do not edit\n")
  foreach(INCLUDE ${INCLUDES})
    file(RELATIVE_PATH INC_REL ${GEN_PATH} ${INCLUDE})
    file(APPEND "${GEN_PATH}/mod_headers.gen.h" "#include \"${INC_REL}\"\n")
  endforeach()

endfunction()
