#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

set(AXTRACE_SOURCES_CPP
	AT_AboutDlg.cpp     
	AT_ColorButton.cpp  
	AT_Config.cpp       
	AT_Incoming.cpp     
	AT_ListCtrlEx.cpp   
	AT_Main.cpp         
	AT_MainFrame.cpp    
	AT_Message.cpp      
	AT_MessageQueue.cpp 
	AT_OptionsDlg.cpp   
	AT_StylePreview.cpp 
	AT_System.cpp       
	AT_TickFrame.cpp    
	AT_TraceFrame.cpp   
	AT_Util.cpp         
	AT_ValueFrame.cpp 
	AT_Filter.cpp  
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
	AT_ColorButton.h 
	AT_Config.h      
	AT_Global.h      
	AT_Incoming.h    
	AT_Interface.h   
	AT_ListCtrlEx.h  
	AT_MainFrame.h   
	AT_Message.h     
	AT_MessageQueue.h
	AT_OptionsDlg.h  
	AT_StylePreview.h
	AT_System.h      
	AT_TickFrame.h   
	AT_TraceFrame.h  
	AT_Util.h        
	AT_ValueFrame.h  
	AT_Filter.h
	resource.h       
	StdAfx.h         
)

set(AXTRACE_SOURCES_RC
	AxTrace.rc
	res/AxTrace.ico            
	res/AxTrace_TickFrame.ico  
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
