cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  DataMiningLab1
  VERSION 1.0
  LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "yes/no" NO)
option(ENABLE_POLYHEDRAL_OPTIMIZATION "yes/no . Graphite for GCC and Polly for Clang" YES)
option(ENABLE_LTO_OPTIMIZATION "yes/no . Graphite for GCC and Polly for Clang" YES)
option(${PROJECT_NAME}_BUILD_STANDALONE "yes/no . Build standalone app?" YES)
option(${PROJECT_NAME}_BUILD_DOCS "yes/no . Build docs?" NO) # I didn't do them

if(${BUILD_SHARED_LIBS})
  # https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows
  if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  endif()
endif()

set(FETCHCONTENT_QUIET
    OFF
    CACHE BOOL "Make downloading of packages quiet"
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
# file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE) add_library(${PROJECT_NAME} ${headers} ${sources})
add_library(${PROJECT_NAME} INTERFACE)
set(${PROJECT_NAME}_BUILD_HEADERS_ONLY TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# being a cross-platform target, we enforce standards conformance on MSVC
# target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  if(GNU)
    target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts-diagnostics-depth=10)
  endif()
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/getCPM.cmake)

set(TRY_BOOST_VERSION "1.85.0")
# set(BOOST_NOT_HEADER_ONLY_COMPONENTS_THAT_YOU_NEED "thread;filesystem;system")
# set(BOOST_HEADER_ONLY_COMPONENTS_THAT_YOU_NEED "asio;beast;cobalt;core;date_time;container")
# set(BOOST_MY_OPTIONS "BOOST_ENABLE_PYTHON ON;")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/packages/add_boost.cmake)

include(./cmake/packages/add_fmt.cmake)

target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt)

# include(./cmake/packages/add_certify.cmake)
#
# target_link_libraries(${PROJECT_NAME} PUBLIC certify::certify)

# include(./cmake/packages/add_openssl.cmake)
#
# target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL) target_link_libraries(${PROJECT_NAME}
# PUBLIC OpenSSL::Crypto) target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::applink)

# include(./cmake/packages/add_simdjson.cmake)
#
# target_link_libraries(${PROJECT_NAME} PUBLIC simdjson::simdjson)

# include(./cmake/packages/add_constexpr-to-string.cmake)
#
# target_link_libraries(${PROJECT_NAME} PUBLIC constexpr-to-string::constexpr-to-string)

# ---- Stupid customizations ----

include(./cmake/optimization_flags/lto.cmake)
set_project_lto_opts(${PROJECT_NAME})

include(./cmake/optimization_flags/polyhedral.cmake)
set_project_polyhedral_opts(${PROJECT_NAME})

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

include(cmake/packages/add_packageproject.cmake)

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)
string(TOLOWER ${PROJECT_NAME}/export.h EXPORT_HEADER_LOCATION)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION ${PROJECT_VERSION})

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}" EXPORT_HEADER "${EXPORT_HEADER_LOCATION}"
  COMPATIBILITY "AnyNewerVersion" DISABLE_VERSION_SUFFIX ON
  # cmake-format:off DO *NOT* divide this like to different lines. It will destroy a lot of stuff
  # related to making this project not only installable, but usable via `find_package(the_project)`
  DEPENDENCIES "${ADDBOOSTCMAKE_PACKAGEPROJECT_INSTALL_TARGETS};"
  # cmake-format:on
)

if(BUILD_TESTING)
  include(./cmake/warning.cmake)
  set_project_warnings(${PROJECT_NAME})
  enable_testing()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif()
