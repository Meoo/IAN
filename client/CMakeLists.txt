
if(IAN_BUILD_CLIENT)

  set_property(GLOBAL PROPERTY IAN_SHELL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

  set_property(GLOBAL PROPERTY IAN_CLIENT_MODULES)
  set_property(GLOBAL PROPERTY IAN_CLIENT_AUTOLOAD)


  function(ian_client_module)
    set(opts)
    set(oneArgs NAME ROOT)
    set(multiArgs LOAD)
    cmake_parse_arguments(ARG "${opts}" "${oneArgs}" "${multiArgs}" ${ARGN})

    if(NOT ARG_NAME)
      message(FATAL_ERROR "ian_client_module: NAME not set")
    endif()

    if(NOT ARG_ROOT)
      message(FATAL_ERROR "ian_client_module: ROOT not set")
    endif()


    set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_MODULES ${ARG_NAME})

    get_filename_component(ROOT_FULLPATH ${ARG_ROOT} ABSOLUTE)
    set_property(GLOBAL PROPERTY IAN_CLIENT_MOD_${ARG_NAME}_ROOT ${ROOT_FULLPATH})

    foreach(LOAD_FILE ${ARG_LOAD})
      set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_AUTOLOAD ${ARG_NAME}/${LOAD_FILE})
    endforeach()

  endfunction()



  function(ian_client_finalize)
    message(STATUS "=== IAN client modules ===")

    get_property(SHELL_DIR GLOBAL PROPERTY IAN_SHELL_DIR)
    set(OUT_PATH ${CMAKE_BINARY_DIR}/client)


    set(IAN_GEN_JS_REQUIRES)
    set(IAN_GEN_WP_ALIASES)


    get_property(MODULES GLOBAL PROPERTY IAN_CLIENT_MODULES)
    list(LENGTH MODULES MODULES_COUNT)
    message(STATUS "* ${MODULES_COUNT} modules")
    foreach(MODULE ${MODULES})
      get_property(MOD_ROOT GLOBAL PROPERTY IAN_CLIENT_MOD_${MODULE}_ROOT)
      file(RELATIVE_PATH MOD_ROOT_REL ${OUT_PATH} ${MOD_ROOT})
      set(IAN_GEN_WP_ALIASES "${IAN_GEN_WP_ALIASES}\n      ${MODULE}: path.resolve(__dirname, '${MOD_ROOT_REL}'),")
    endforeach()

    get_property(AUTOLOAD GLOBAL PROPERTY IAN_CLIENT_AUTOLOAD)
    list(LENGTH AUTOLOAD AUTOLOAD_COUNT)
    message(STATUS "* ${AUTOLOAD_COUNT} autoloaded files")
    foreach(LOAD_PACK ${AUTOLOAD})
      set(IAN_GEN_JS_REQUIRES "${IAN_GEN_JS_REQUIRES}\n_ian_require_log('${LOAD_PACK}'); require('${LOAD_PACK}');")
    endforeach()


    configure_file(${SHELL_DIR}/package.json ${OUT_PATH}/package.json @ONLY)
    configure_file(${SHELL_DIR}/webpack.common.js ${OUT_PATH}/webpack.common.js @ONLY)
    configure_file(${SHELL_DIR}/webpack.dev.js ${OUT_PATH}/webpack.dev.js @ONLY)
    configure_file(${SHELL_DIR}/webpack.prod.js ${OUT_PATH}/webpack.prod.js @ONLY)
    configure_file(${SHELL_DIR}/ian.js ${OUT_PATH}/ian.js @ONLY)

  endfunction()

else()
  function(ian_client_module)
  endfunction()

  function(ian_client_finalize)
  endfunction()
endif()
