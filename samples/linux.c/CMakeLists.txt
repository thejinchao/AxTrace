cmake_minimum_required (VERSION 3.1)
project(AxTrace4.Test)

set(TEST_SOURCES
    TestMain.cpp     
)

set(TEST_AXTRACE_C_SOURCES
    axtrace.linux.c
    axtrace.linux.h
)

set(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(linux.c
    ${TEST_SOURCES}
    ${TEST_AXTRACE_C_SOURCES}
)

target_link_libraries(linux.c
	pthread
)
