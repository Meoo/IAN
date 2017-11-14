
project(protocols-external)

hunter_add_package(flatbuffers)
find_package(flatbuffers CONFIG REQUIRED)


set(INCLUDE_DIR "proto_external")

set(FBS_FILES
  test.fbs
  test2.fbs)


if(WIN32)
  set(FLATBUFFERS_FLATC_EXECUTABLE "${FLATBUFFERS_ROOT}/bin/flatc.exe")
else()
  set(FLATBUFFERS_FLATC_EXECUTABLE "${FLATBUFFERS_ROOT}/bin/flatc")
endif()

set(GENERATED_FILES)
set(GENERATED_FILES_JS)
set(INCLUDE_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include")
set(GEN_PATH "${INCLUDE_PATH}/${INCLUDE_DIR}")
set(JS_GEN_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/js")

foreach(SRC_FBS ${FBS_FILES})
  string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${SRC_FBS})
  set(GEN_HEADER "${GEN_PATH}/${GEN_HEADER_NAME}")
  add_custom_command(
    OUTPUT ${GEN_HEADER}
    COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c
      -o "${GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
  list(APPEND GENERATED_FILES ${GEN_HEADER})

  string(REGEX REPLACE "\\.fbs$" "_generated.js" GEN_JS_NAME ${SRC_FBS})
  set(GEN_JS "${JS_GEN_PATH}/${GEN_JS_NAME}")
  add_custom_command(
    OUTPUT ${GEN_JS}
    COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -s
      -o "${JS_GEN_PATH}/" -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}")
  list(APPEND GENERATED_FILES_JS ${GEN_JS})
endforeach()

add_custom_target(${PROJECT_NAME}-gen ALL
  DEPENDS ${GENERATED_FILES}
  SOURCES ${FBS_FILES})

add_custom_target(${PROJECT_NAME}-gen-js ALL
  DEPENDS ${GENERATED_FILES_JS}
  SOURCES ${FBS_FILES})

ian_client_add_js(${GENERATED_FILES_JS})

add_library(${PROJECT_NAME} INTERFACE)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-gen)

target_include_directories(${PROJECT_NAME} INTERFACE ${INCLUDE_PATH})
target_link_libraries(${PROJECT_NAME} INTERFACE flatbuffers::flatbuffers)
