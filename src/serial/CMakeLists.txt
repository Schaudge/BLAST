# $Id: CMakeLists.txt 621588 2020-12-14 19:26:57Z ivanov $

NCBI_project_tags(core)
NCBI_add_library(serial cserial)
NCBI_add_subdirectory(datatool test soap grpc_integration)


