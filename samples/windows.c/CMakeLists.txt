cmake_minimum_required (VERSION 3.1)
project(AxTrace4.Test)

set(TEST_SOURCES
    TestMain.cpp     
)

set(TEST_AXTRACE_C_SOURCES
    axtrace.win.c
    axtrace.win.h
)


add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

ADD_EXECUTABLE(Windows.C
    ${TEST_SOURCES}
    ${TEST_AXTRACE_C_SOURCES}
)

