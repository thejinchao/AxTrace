#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

project(AxTrace)

########
#sub dictionary
########
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_subdirectory(AxTrace)
add_subdirectory(luajit)
add_subdirectory(Samples/Windows.C)
add_subdirectory(Samples/DotNet.CS)
