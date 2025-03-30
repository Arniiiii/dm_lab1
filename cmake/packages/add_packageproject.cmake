# find_package(PackageProject.cmake REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/../getCPM.cmake)

CPMAddPackage(NAME PackageProject.cmake VERSION 13.0.3)
