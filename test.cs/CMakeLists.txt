#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

#copy cs project file as configure files
set(TEST_CS_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(TEST_CS_PROJ_FILE ${TEST_CS_BINARY_PATH}/test.cs.csproj)

string(REPLACE "/" "\\"
	TEST_CS_SOURCE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.cs.csproj.in
	${TEST_CS_PROJ_FILE}
)

include_external_msproject(
    test.cs ${TEST_CS_PROJ_FILE}
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

