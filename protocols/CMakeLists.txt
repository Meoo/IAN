
hunter_add_package(flatbuffers)
find_package(flatbuffers CONFIG REQUIRED)


if(WIN32)
  set(FLATBUFFERS_FLATC_EXECUTABLE "${FLATBUFFERS_ROOT}/bin/flatc.exe")
else()
  set(FLATBUFFERS_FLATC_EXECUTABLE "${FLATBUFFERS_ROOT}/bin/flatc")
endif()


function(ian_add_protocol)
  set(opts GEN_JS)
  set(oneArgs TARGET INCLUDE)
  set(multiArgs FILES)
  cmake_parse_arguments(IAN_PROTO "${opts}" "${oneArgs}" "${multiArgs}" ${ARGN})

  if(NOT IAN_PROTO_TARGET)
    message(FATAL_ERROR "ian_add_protocol: TARGET not set")
  endif()

  if(NOT IAN_PROTO_INCLUDE)
    message(FATAL_ERROR "ian_add_protocol: INCLUDE not set")
  endif()

  set(GENERATED_FILES)
  set(GENERATED_FILES_JS)
  set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/${IAN_PROTO_TARGET}/include")
  set(GEN_PATH "${INCLUDE_PATH}/${IAN_PROTO_INCLUDE}")
  set(JS_GEN_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/js")

  foreach(SRC_FBS ${IAN_PROTO_FILES})
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${SRC_FBS})
    set(GEN_HEADER "${GEN_PATH}/${GEN_HEADER_NAME}")
    add_custom_command(
      OUTPUT ${GEN_HEADER}
      COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c
        -o "${GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
    list(APPEND GENERATED_FILES ${GEN_HEADER})
  endforeach()

  add_custom_target(${IAN_PROTO_TARGET}-gen
    DEPENDS ${GENERATED_FILES}
    SOURCES ${IAN_PROTO_FILES})

  add_library(${IAN_PROTO_TARGET} INTERFACE)

  add_dependencies(${IAN_PROTO_TARGET} ${IAN_PROTO_TARGET}-gen)

  target_include_directories(${IAN_PROTO_TARGET} INTERFACE ${INCLUDE_PATH})
  target_link_libraries(${IAN_PROTO_TARGET} INTERFACE flatbuffers::flatbuffers)

  if(IAN_BUILD_CLIENT AND IAN_PROTO_GEN_JS)
    foreach(SRC_FBS ${IAN_PROTO_FILES})
      string(REGEX REPLACE "\\.fbs$" "_generated.js" GEN_JS_NAME ${SRC_FBS})
      set(GEN_JS "${JS_GEN_PATH}/${GEN_JS_NAME}")
      add_custom_command(
        OUTPUT ${GEN_JS}
        COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -s
          -o "${JS_GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
      list(APPEND GENERATED_FILES_JS ${GEN_JS})
    endforeach()

    add_custom_target(${IAN_PROTO_TARGET}-gen-js ALL
      DEPENDS ${GENERATED_FILES_JS}
      SOURCES ${IAN_PROTO_FILES})

    ian_client_add_js(${GENERATED_FILES_JS})
  endif()
endfunction()


add_subdirectory(internal)
add_subdirectory(external)
