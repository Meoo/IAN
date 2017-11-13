
function(ian_module_version MOD VER)
  set_property(GLOBAL PROPERTY IAN_MODULE_VERSION_${MOD} ${VER})
endfunction()

function(ian_module_version_required MOD VER)
  get_property(MODVER GLOBAL PROPERTY IAN_MODULE_VERSION_${MOD})
  if(NOT DEFINED ${MODVER})
    message(FATAL_ERROR "Module ${MOD} version ${VER} required (module not available)")
  endif()
  if(${MODVER} VERSION_LESS ${VER})
    message(FATAL_ERROR "Module ${MOD} version ${VER} required (current version is ${MODVER})")
  endif()
endfunction()


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
