# find_package(PackageProject.cmake REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/../getCPM.cmake)

CPMAddPackage(NAME PackageProject.cmake VERSION 13.0.5 URL "https://github.com/Arniiiii/PackageProject.cmake/archive/refs/tags/13.0.5.tar.gz")
