#############################################################################
# $Id: CMakeLists.txt 644780 2022-02-02 20:26:15Z blastadm $
#############################################################################

cmake_minimum_required(VERSION 3.7)
if ("${PROJECT_NAME}" STREQUAL "")
    project(ncbi_cpp)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/build-system/cmake/CMakeLists.top_builddir.txt)
