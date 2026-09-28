cmake_minimum_required (VERSION 3.15)

if(NOT WIN32)
    message(FATAL_ERROR "This CMake build currently supports Windows.")
endif()

project(AxTrace4.Test)

set(TEST_SOURCES
    TestMain.cpp     
)

set(TEST_AXTRACE_C_SOURCES
    axtrace.win.c
    axtrace.win.h
)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()


add_executable(Windows.C
    ${TEST_SOURCES}
    ${TEST_AXTRACE_C_SOURCES}
)

target_link_libraries(Windows.C
    ws2_32.lib
    shlwapi.lib
)