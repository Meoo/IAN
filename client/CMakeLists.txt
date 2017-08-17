
set_property(GLOBAL PROPERTY IAN_SHELL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set_property(GLOBAL PROPERTY IAN_CLIENT_BODY_FILES)
set_property(GLOBAL PROPERTY IAN_CLIENT_JS_FILES)
set_property(GLOBAL PROPERTY IAN_CLIENT_CSS_FILES)


function(ian_client_add_body)
  foreach(BODY_FILE ${ARGV})
    get_filename_component(BODY_FULLPATH ${BODY_FILE} ABSOLUTE)
    set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_BODY_FILES ${BODY_FULLPATH})
  endforeach()
endfunction()

function(ian_client_add_js)
  foreach(JS_FILE ${ARGV})
    if (JS_FILE MATCHES "^https?://")
      set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_JS_FILES ${JS_FILE})
    else()
      get_filename_component(JS_FULLPATH ${JS_FILE} ABSOLUTE)
      set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_JS_FILES ${JS_FULLPATH})
    endif()
  endforeach()
endfunction()

function(ian_client_add_css)
  foreach(CSS_FILE ${ARGV})
    if (CSS_FILE MATCHES "^https?://")
      set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_CSS_FILES ${CSS_FILE})
    else()
      get_filename_component(CSS_FULLPATH ${CSS_FILE} ABSOLUTE)
      set_property(GLOBAL APPEND PROPERTY IAN_CLIENT_CSS_FILES ${CSS_FULLPATH})
    endif()
  endforeach()
endfunction()



function(ian_client_finalize)
  message(STATUS "=== IAN client-dev ===")

  get_property(SHELL_DIR GLOBAL PROPERTY IAN_SHELL_DIR)
  set(OUT_PATH ${CMAKE_BINARY_DIR}/client-dev)


  set(IAN_SHELL_HEAD "<title>IAN dev</title>")


  set(IAN_SHELL_BODY)

  get_property(BODY_FILES GLOBAL PROPERTY IAN_CLIENT_BODY_FILES)
  list(LENGTH BODY_FILES BODY_FILES_COUNT)
  message(STATUS "* ${BODY_FILES_COUNT} body parts")
  foreach(BODY_FILE ${BODY_FILES})
    file(READ ${BODY_FILE} BODY_CONTENT)
    set(IAN_SHELL_BODY "${IAN_SHELL_BODY}\n<!--${BODY_FILE}-->\n${BODY_CONTENT}")
  endforeach(BODY_FILE)


  file(RELATIVE_PATH JS_DEV_REL ${OUT_PATH} ${SHELL_DIR}/dev.js)
  set(IAN_SHELL_JS "<script type=\"text/javascript\" src=\"${JS_DEV_REL}\"></script>\n")
  set(JS_LIST)

  get_property(JS_FILES GLOBAL PROPERTY IAN_CLIENT_JS_FILES)
  list(LENGTH JS_FILES JS_FILES_COUNT)
  message(STATUS "* ${JS_FILES_COUNT} JS files")
  foreach(JS_FILE ${JS_FILES})
    if (JS_FILE MATCHES "^https?://")
      list(APPEND JS_LIST ${JS_FILE})
    else()
      file(RELATIVE_PATH JS_REL ${OUT_PATH} ${JS_FILE})
      list(APPEND JS_LIST ${JS_REL})
    endif()
  endforeach(JS_FILE)

  string (REPLACE ";" "\",\n\"" JS_LIST_EX "${JS_LIST}")
  set(IAN_SHELL_JS "${IAN_SHELL_JS}<script type=\"text/javascript\">")
  set(IAN_SHELL_JS "${IAN_SHELL_JS}ian_dev.load([\"${JS_LIST_EX}\"]);")
  set(IAN_SHELL_JS "${IAN_SHELL_JS}</script>")


  set(IAN_SHELL_CSS)

  get_property(CSS_FILES GLOBAL PROPERTY IAN_CLIENT_CSS_FILES)
  list(LENGTH CSS_FILES CSS_FILES_COUNT)
  message(STATUS "* ${CSS_FILES_COUNT} CSS files")
  foreach(CSS_FILE ${CSS_FILES})
    if (CSS_FILE MATCHES "^https?://")
      set(IAN_SHELL_CSS "${IAN_SHELL_CSS}<link rel=\"stylesheet\" type=\"text/css\" href=\"${CSS_FILE}\">\n")
    else()
      file(RELATIVE_PATH CSS_REL ${OUT_PATH} ${CSS_FILE})
      set(IAN_SHELL_CSS "${IAN_SHELL_CSS}<link rel=\"stylesheet\" type=\"text/css\" href=\"${CSS_REL}\">\n")
    endif()
  endforeach(CSS_FILE)


  configure_file(${SHELL_DIR}/shell.html ${OUT_PATH}/index.html @ONLY)

endfunction()

ian_client_add_css(shell.css)
