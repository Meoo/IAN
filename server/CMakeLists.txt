
# Module bundle is included in finalize (modules have to be declared first)


if(IAN_BUILD_SERVER OR IAN_BUILD_SERVER_MODULES)
  add_subdirectory(module-sdk)
endif()

if(IAN_BUILD_SERVER)
  add_subdirectory(common)

  add_subdirectory(node)
  add_subdirectory(front)
  add_subdirectory(game)
  add_subdirectory(master)
endif()



if(IAN_BUILD_SERVER_MODULES)

  set_property(GLOBAL PROPERTY IAN_SERVER_INCLUDES)
  set_property(GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
  set_property(GLOBAL PROPERTY IAN_SERVER_MODULE_INCLUDES)


  # Annoying but I have to cache current dir
  set_property(GLOBAL PROPERTY IAN_SERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR})


  macro(ian_server_module MODPROJ)
    add_library(${MODPROJ} STATIC ${ARGN})
    target_link_libraries(${MODPROJ} PRIVATE server-module-sdk)
    set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULE_LINKS ${MODPROJ})
  endmacro()

  function(ian_server_add_module_header)
    foreach(HEADER_FILE ${ARGV})
      get_filename_component(HEADER_FULLPATH ${HEADER_FILE} ABSOLUTE)
      set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULE_INCLUDES ${HEADER_FULLPATH})
    endforeach()
  endfunction()

  function(ian_server_add_module_class)
    set_property(GLOBAL APPEND PROPERTY IAN_SERVER_MODULES ${ARGV})
  endfunction()


  function(ian_server_finalize)

    # Finally include module bundle when finalizing
    # Links are done inside
    get_property(SRVDIR GLOBAL PROPERTY IAN_SERVER_DIR)
    add_subdirectory(${SRVDIR}/module-bundle)


    message(STATUS "=== IAN server modules ===")

    set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/modules/include")
    set(GEN_PATH "${INCLUDE_PATH}/modules")

    # Somehow includes can be done here
    target_include_directories(server-module-bundle PRIVATE ${INCLUDE_PATH})


    get_property(LIBS GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
    list(LENGTH LIBS LIBS_COUNT)
    message(STATUS "* ${LIBS_COUNT} libraries")


    get_property(MODULES GLOBAL PROPERTY IAN_SERVER_MODULES)
    list(LENGTH MODULES MODULES_COUNT)
    message(STATUS "* ${MODULES_COUNT} module classes")

    file(WRITE "${GEN_PATH}/mod_list.gen" "// IAN generated file, do not edit\n")
    foreach(MODULE ${MODULES})
      file(APPEND "${GEN_PATH}/mod_list.gen" "IAN_MODULE_MACRO(${MODULE})\n")
    endforeach()


    get_property(INCLUDES GLOBAL PROPERTY IAN_SERVER_MODULE_INCLUDES)
    list(LENGTH INCLUDES INCLUDES_COUNT)
    message(STATUS "* ${INCLUDES_COUNT} headers")

    file(WRITE "${GEN_PATH}/mod_headers.gen.h" "// IAN generated file, do not edit\n")
    foreach(INCLUDE ${INCLUDES})
      file(RELATIVE_PATH INC_REL ${GEN_PATH} ${INCLUDE})
      file(APPEND "${GEN_PATH}/mod_headers.gen.h" "#include \"${INC_REL}\"\n")
    endforeach()

  endfunction()

else()
  function(ian_server_module)
  endfunction()

  function(ian_server_add_module_header)
  endfunction()

  function(ian_server_add_module_class)
  endfunction()

  function(ian_server_finalize)
  endfunction()
endif()
