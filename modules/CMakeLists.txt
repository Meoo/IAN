
file(GLOB CHILDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(MODULES)

foreach(CHILD ${CHILDS})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CHILD}
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${CHILD}/CMakeLists.txt)
    list(APPEND MODULES ${CHILD})
  endif()
endforeach()

message(STATUS "=== Modules ===")

list(SORT MODULES)
foreach(MODULE ${MODULES})
  message(STATUS "* ${MODULE}")
  add_subdirectory(${MODULE})
endforeach()
