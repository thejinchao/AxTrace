#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

set(TEST_SOURCES_CPP
	TestMain.cpp     
	TestThreadPool.cpp  
	TestUtil.cpp       
)

set(TEST_SOURCES_CPP_PRECOMPILED 
	StdAfx.cpp
)

set(TEST_SOURCES_INC
	StdAfx.h    
	TestThreadPool.h  
	TestUtil.h  
)

set(TEST_AXTRACE_C_SOURCES
	axtrace.win.c
	axtrace.win.h
)

foreach(src_file ${TEST_SOURCES_CPP})
	set_source_files_properties(
		${src_file}
		PROPERTIES
		COMPILE_FLAGS "/Yustdafx.h"
	)
endforeach()
    
set_source_files_properties(${TEST_SOURCES_CPP_PRECOMPILED}
	PROPERTIES
	COMPILE_FLAGS "/Ycstdafx.h"
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

ADD_EXECUTABLE(test.win.c
	${TEST_SOURCES_CPP_PRECOMPILED}
	${TEST_SOURCES_CPP}
	${TEST_SOURCES_INC}
	${TEST_AXTRACE_C_SOURCES}
)

	