cmake_minimum_required(VERSION 3.14...3.22)

project(
  DataMiningLab1Standalone
  VERSION 1.0
  LANGUAGES CXX
)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 OUTPUT_NAME "DataMiningLab1")

target_link_libraries(${PROJECT_NAME} PRIVATE DataMiningLab1::DataMiningLab1)

include(../cmake/packages/add_argparse.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)

include(../cmake/packages/add_fmt.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

include(../cmake/packages/add_magic_enum.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)

include(../cmake/packages/add_quill.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE quill::quill)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_project()`.

include(../cmake/packages/add_packageproject.cmake)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  COMPATIBILITY "AnyNewerVersion" DISABLE_VERSION_SUFFIX ON
  DEPENDENCIES
    "DataMiningLab1 1.0;fmt ${fmt_VERSION};magic_enum ${magic_enum_VERSION};argparse ${argparse_VERSION}"
)
