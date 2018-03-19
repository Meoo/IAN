
macro(ian_add_protocol)
  set(opts CLIENT)
  set(oneArgs NAME INCLUDE HASH RPC)
  set(multiArgs FILES)
  cmake_parse_arguments(ARG "${opts}" "${oneArgs}" "${multiArgs}" ${ARGN})

  if(NOT ARG_NAME)
    message(FATAL_ERROR "ian_add_protocol: NAME not set")
  endif()

  if(NOT ARG_INCLUDE)
    message(FATAL_ERROR "ian_add_protocol: INCLUDE not set")
  endif()

  if(IAN_BUILD_SERVER)
    set(GENERATED_FILES)
    set(FBS_GEN_PATH "${CMAKE_BINARY_DIR}/${ARG_NAME}/fbs")
    set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/${ARG_NAME}/include")
    set(GEN_PATH "${INCLUDE_PATH}/${ARG_INCLUDE}")

    # RPC first because we create a .fbs file
    if(ARG_RPC)
      set(GEN_IF "${GEN_PATH}/Rpc_interfaces.hpp")
      set(GEN_FBS "${FBS_GEN_PATH}/Rpc.fbs")
      add_custom_command(
        OUTPUT ${GEN_IF} ${GEN_FBS}
        COMMAND ${CMAKE_COMMAND}
          -DRPC_DESCRIPTOR="${ARG_RPC}" -DRPC_HEADER_OUT="${GEN_IF}" -DRPC_FBS_OUT="${GEN_FBS}"
          -P ${CMAKE_SOURCE_DIR}/cmake/scripts/protoRpc.cmake
        MAIN_DEPENDENCY ${ARG_RPC}
        DEPENDS
          ${CMAKE_SOURCE_DIR}/cmake/scripts/protoRpc.cmake
          ${CMAKE_SOURCE_DIR}/cmake/scripts/protoRpc_template.hpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
      list(APPEND GENERATED_FILES ${GEN_IF} ${GEN_FBS})
      file(RELATIVE_PATH GEN_FBS_REL ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_FBS})
      list(APPEND ARG_FILES ${GEN_FBS_REL})
    endif()

    # .fbs files
    foreach(SRC_FBS ${ARG_FILES})
      get_filename_component(SRC_FBS_NAME ${SRC_FBS} NAME)
      string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${SRC_FBS_NAME})
      set(GEN_HEADER "${GEN_PATH}/${GEN_HEADER_NAME}")
      add_custom_command(
        OUTPUT ${GEN_HEADER}
        COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c
          -o "${GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" -I "${FBS_GEN_PATH}" "${SRC_FBS}"
        MAIN_DEPENDENCY ${SRC_FBS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
      list(APPEND GENERATED_FILES ${GEN_HEADER})
    endforeach()

    # Hash all .fbs files together
    if(ARG_HASH)
      set(GEN_HEADER "${GEN_PATH}/Hash.h")
      add_custom_command(
        OUTPUT ${GEN_HEADER}
        COMMAND ${CMAKE_COMMAND}
          -DHASH_FILES="${ARG_FILES}" -DHASH_OUT="${GEN_HEADER}" -DHASH_DEF="${ARG_HASH}"
          -P ${CMAKE_SOURCE_DIR}/cmake/scripts/protoHash.cmake
        DEPENDS ${ARG_FILES} ${CMAKE_SOURCE_DIR}/cmake/scripts/protoHash.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
      list(APPEND GENERATED_FILES ${GEN_HEADER})
    endif()

    add_custom_target(${ARG_NAME}-gen
      DEPENDS ${GENERATED_FILES}
      SOURCES ${ARG_FILES})

    add_library(${ARG_NAME} INTERFACE)

    add_dependencies(${ARG_NAME} ${ARG_NAME}-gen)

    target_include_directories(${ARG_NAME} INTERFACE ${INCLUDE_PATH})
    target_link_libraries(${ARG_NAME} INTERFACE flatbuffers::flatbuffers)
  endif()

  if(IAN_BUILD_CLIENT AND ARG_CLIENT)
    set(GENERATED_FILES_JS)
    set(JS_GEN_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/js")

    if(IAN_FLATBUFFERS_USE_TYPESCRIPT)
      set(JS_EXT ts)
      set(FB_FLAG -T)
    else()
      set(JS_EXT js)
      set(FB_FLAG -s)
    endif()

    foreach(SRC_FBS ${ARG_FILES})
      string(REGEX REPLACE "\\.fbs$" "_generated.${JS_EXT}" GEN_JS_NAME ${SRC_FBS})
      set(GEN_JS "${JS_GEN_PATH}/${GEN_JS_NAME}")
      add_custom_command(
        OUTPUT ${GEN_JS}
        COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" ${FB_FLAG}
          -o "${JS_GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
      list(APPEND GENERATED_FILES_JS ${GEN_JS})
    endforeach()

    add_custom_target(${ARG_NAME}-gen-js ALL
      DEPENDS ${GENERATED_FILES_JS}
      SOURCES ${IAN_PROTO_FILES})

    ian_client_module(NAME ${ARG_NAME} ROOT ${JS_GEN_PATH})
  endif()
endmacro()
