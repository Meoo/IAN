
project(server-module-bundle)

if(IAN_SERVER_BUNDLE_STATIC)
  set(IAN_BUNDLE_BUILD_MODE STATIC)
else()
  set(IAN_BUNDLE_BUILD_MODE SHARED)
endif()


add_library(${PROJECT_NAME} ${IAN_BUNDLE_BUILD_MODE}
  include/modulebundle/ModuleBundle.hpp
  src/ModuleBundle.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  server-module-sdk)


# Link to all declared modules
get_property(LIBS GLOBAL PROPERTY IAN_SERVER_MODULE_LINKS)
target_link_libraries(server-module-bundle PRIVATE
  ${LIBS})


target_include_directories(${PROJECT_NAME} PUBLIC include)

if(IAN_SERVER_BUNDLE_STATIC)
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_BUNDLE_STATIC)
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC IAN_BUNDLE_SHARED)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE IAN_BUNDLE_BUILD)

if(NOT IAN_SERVER_BUNDLE_STATIC)
  set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ianbundle")
#set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINVER=${IAN_WINDOWS_VERSION} _WIN32_WINNT=${IAN_WINDOWS_VERSION})
endif()
