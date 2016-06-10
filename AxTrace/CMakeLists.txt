#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

set(AXTRACE_SOURCES_CPP
	AT_AboutDlg.cpp     
	AT_Config.cpp       
	AT_Incoming.cpp     
	AT_ListCtrlEx.cpp   
	AT_Main.cpp         
	AT_MainFrame.cpp    
	AT_Message.cpp      
	AT_MessageQueue.cpp 
	AT_System.cpp       
	AT_LogFrame.cpp   
	AT_Util.cpp         
	AT_ValueFrame.cpp 
	AT_Filter.cpp  
	AT_Math.cpp
	AT_2DFrame.cpp  
	AT_2DView.cpp
	AT_2DScene.cpp
	AT_2DCamera.cpp
	AT_Canvas.cpp
	AT_Color.cpp
)

set(AXTRACE_SOURCES_CPP_PRECOMPILED 
	StdAfx.cpp
)

set(AXTRACE_SOURCES_UTF
	utf/ConvertUTF.c
	utf/ConvertUTF.h
)

source_group("Source Files\\utf" FILES ${AXTRACE_SOURCES_UTF})

set(AXTRACE_SOURCES_INC
	AT_AboutDlg.h    
	AT_ChildFrame.h  
	AT_Config.h      
	AT_Global.h      
	AT_Incoming.h    
	AT_Interface.h   
	AT_ListCtrlEx.h  
	AT_MainFrame.h   
	AT_Message.h     
	AT_MessageQueue.h
	AT_System.h      
	AT_LogFrame.h  
	AT_Util.h        
	AT_ValueFrame.h  
	AT_Filter.h
	AT_Math.h
	AT_2DFrame.h
	AT_2DView.h
	AT_2DScene.h
	AT_2DCamera.h
	AT_Canvas.h
	AT_Color.h
	AtlOpengl.h
	resource.h       
	StdAfx.h         
)

set(AXTRACE_SOURCES_RC
	AxTrace.rc
	res/AxTrace.ico            
	res/AxTrace_TraceFrame.ico 
	res/MainToolBar.bmp        
)
source_group("Resource Files" FILES ${AXTRACE_SOURCES_RC})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/WTL
	${CMAKE_CURRENT_SOURCE_DIR}/../luajit
	$ENV{CYCLONE_SDK_ROOT}/include
)

link_directories(
	$ENV{CYCLONE_SDK_ROOT}/lib
)

add_definitions(-D_UNICODE -DUNICODE -D_ATL_NO_OLD_NAMES)

foreach(src_file ${AXTRACE_SOURCES_CPP})
	set_source_files_properties(
		${src_file}
		PROPERTIES
		COMPILE_FLAGS "/Yustdafx.h"
	)
endforeach()
    
set_source_files_properties(${AXTRACE_SOURCES_CPP_PRECOMPILED}
	PROPERTIES
	COMPILE_FLAGS "/Ycstdafx.h"
)

ADD_EXECUTABLE(AxTrace WIN32 
	${AXTRACE_SOURCES_CPP_PRECOMPILED}
	${AXTRACE_SOURCES_CPP}
	${AXTRACE_SOURCES_UTF}
	${AXTRACE_SOURCES_RC}
	${AXTRACE_SOURCES_INC}
)

target_link_libraries(AxTrace
	ws2_32
	cyclone
	libluajit
	winmm
)

set_target_properties(AxTrace PROPERTIES LINK_FLAGS "/link ${CMAKE_CURRENT_SOURCE_DIR}/../luajit/lj_vm_x86.obj")
