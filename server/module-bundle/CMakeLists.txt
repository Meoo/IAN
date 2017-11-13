
project(server-module-bundle)

add_library(${PROJECT_NAME} SHARED
  include/modulebundle/ModuleBundle.hpp
  src/ModuleBundle.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-module-sdk)


# Link to all declared modules
get_property(LIBS GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
target_link_libraries(server-module-bundle PRIVATE
  ${LIBS})


target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_BUNDLE_SHARED)
target_compile_definitions(${PROJECT_NAME} PRIVATE IAN_BUNDLE_BUILD)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbundle")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()
