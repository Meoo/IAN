
macro(ian_add_protocol)
  set(opts GEN_JS)
  set(oneArgs TARGET INCLUDE)
  set(multiArgs FILES)
  cmake_parse_arguments(ARG "${opts}" "${oneArgs}" "${multiArgs}" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "ian_add_protocol: TARGET not set")
  endif()

  if(NOT ARG_INCLUDE)
    message(FATAL_ERROR "ian_add_protocol: INCLUDE not set")
  endif()

  set(GENERATED_FILES)
  set(GENERATED_FILES_JS)
  set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/${ARG_TARGET}/include")
  set(GEN_PATH "${INCLUDE_PATH}/${ARG_INCLUDE}")
  set(JS_GEN_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/js")

  foreach(SRC_FBS ${ARG_FILES})
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${SRC_FBS})
    set(GEN_HEADER "${GEN_PATH}/${GEN_HEADER_NAME}")
    add_custom_command(
      OUTPUT ${GEN_HEADER}
      COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c
        -o "${GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
    list(APPEND GENERATED_FILES ${GEN_HEADER})
  endforeach()

  add_custom_target(${ARG_TARGET}-gen
    DEPENDS ${GENERATED_FILES}
    SOURCES ${ARG_FILES})

  add_library(${ARG_TARGET} INTERFACE)

  add_dependencies(${ARG_TARGET} ${ARG_TARGET}-gen)

  target_include_directories(${ARG_TARGET} INTERFACE ${INCLUDE_PATH})
  target_link_libraries(${ARG_TARGET} INTERFACE flatbuffers::flatbuffers)

  if(IAN_BUILD_CLIENT AND ARG_GEN_JS)
    foreach(SRC_FBS ${ARG_FILES})
      string(REGEX REPLACE "\\.fbs$" "_generated.js" GEN_JS_NAME ${SRC_FBS})
      set(GEN_JS "${JS_GEN_PATH}/${GEN_JS_NAME}")
      add_custom_command(
        OUTPUT ${GEN_JS}
        COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -s
          -o "${JS_GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
      list(APPEND GENERATED_FILES_JS ${GEN_JS})
    endforeach()

    add_custom_target(${ARG_TARGET}-gen-js ALL
      DEPENDS ${GENERATED_FILES_JS}
      SOURCES ${IAN_PROTO_FILES})

    ian_client_add_js(${GENERATED_FILES_JS})
  endif()
endmacro()
