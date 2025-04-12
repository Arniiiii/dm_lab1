cmake_minimum_required(VERSION 3.14...3.22)

project(DataMiningLab1Tests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/getCPM.cmake)
# set(CPM_DOWNLOAD_ALL ON) CPMAddPackage( NAME doctest GITHUB_REPOSITORY "doctest/doctest" VERSION
# "2.4.9" )
include(../cmake/packages/add_googletest.cmake)

# find_package(Format.cmake 1.8.3 REQUIRED)

# ---- Create binary ----

file(
  GLOB_RECURSE sources CONFIGURE_DEPENDS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/source/
  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

foreach(source ${sources})
  string(REGEX REPLACE "(\.cc|\.cpp|\.cxx)" "" name ${source})
  string(REPLACE "." "_" name ${name})
  string(REPLACE "/" "_" name ${name})
  add_executable(${PROJECT_NAME}_test_${name} ${CMAKE_CURRENT_SOURCE_DIR}/source/${source})
  target_link_libraries(${PROJECT_NAME}_test_${name} PRIVATE DataMiningLab1::DataMiningLab1)
  target_include_directories(
    ${PROJECT_NAME}_test_${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  # target_link_libraries(${PROJECT_NAME}_test_${name} PRIVATE gtest_main)
  set_target_properties(${PROJECT_NAME}_test_${name} PROPERTIES CXX_STANDARD 23)
  add_test(NAME ${PROJECT_NAME}_test_${name} COMMAND ${PROJECT_NAME}_test_${name})
endforeach()
# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(DataMiningLab1 PUBLIC -Wall -Wpedantic -Wextra)
  elseif(MSVC)
    target_compile_options(DataMiningLab1 PUBLIC /W4)
    # target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add DataMiningLab1Tests ----

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

# if(NOT DEFINED doctest_ADDED OR "${doctest_ADDED}" STREQUAL "")
# include("/usr/lib64/cmake/doctest/doctest.cmake") else()
# include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake) endif()
# doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(DataMiningLab1 PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(DataMiningLab1 PUBLIC -fprofile-arcs -ftest-coverage)
endif()
