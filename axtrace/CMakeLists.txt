set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


set(AT4_DATA_SOURCES
	Data/AT4_LogData.h
	Data/AT4_LogData.cpp
	Data/AT4_ValueData.h
	Data/AT4_ValueData.cpp
)
source_group("Data" FILES ${AT4_DATA_SOURCES})

set(AT4_CHILD_WND_SOURCES
	ChildWindow/AT4_ChildWnd_Log.h
	ChildWindow/AT4_ChildWnd_Log.cpp
	ChildWindow/AT4_ChildWnd_Value.h
	ChildWindow/AT4_ChildWnd_Value.cpp
	ChildWindow/AT4_ChildWnd_Scene2D.h
	ChildWindow/AT4_ChildWnd_Scene2D.cpp
	ChildWindow/AT4_ChildInterface.h
	ChildWindow/AT4_ChildInterface.cpp
	ChildWindow/AT4_Camera2D.h
	ChildWindow/AT4_Camera2D.cpp
	ChildWindow/AT4_Scene2D.h
	ChildWindow/AT4_Scene2D.cpp
	ChildWindow/AT4_LogWndColumn.h
	ChildWindow/AT4_LogWndColumn.cpp
)
source_group("ChildWindow" FILES ${AT4_CHILD_WND_SOURCES})

set(AT4_SETTING_WND_SOURCES
	SettingWindow/AT4_SettingDialog.h
	SettingWindow/AT4_SettingDialog.cpp
)
source_group("SettingWindow" FILES ${AT4_SETTING_WND_SOURCES})

set(AT4_SCRIPT_EDITOR_WND_SOURCES
	ScriptEditorWindow/AT4_CodeEditor.h
	ScriptEditorWindow/AT4_CodeEditor.cpp
	ScriptEditorWindow/AT4_LuaHighlighter.h
	ScriptEditorWindow/AT4_LuaHighlighter.cpp
	ScriptEditorWindow/AT4_ScriptEditDialog.h
	ScriptEditorWindow/AT4_ScriptEditDialog.cpp
)
source_group("ScriptEditorWindow" FILES ${AT4_SCRIPT_EDITOR_WND_SOURCES})


set(AT4_SESSION_SOURCES
	Session/AT4_Session.h
	Session/AT4_Session.cpp
	Session/AT4_SessionDialog.h
	Session/AT4_SessionDialog.cpp
)
source_group("Session" FILES ${AT4_SESSION_SOURCES})

set(AT4_CPP_SOURCES
    AT4_Main.cpp
    AT4_MainWindow.cpp
	AT4_Incoming.cpp
	AT4_System.cpp
	AT4_Config.cpp
	AT4_MessageQueue.cpp
	AT4_Message.cpp
	AT4_LuaVirtualMachine.cpp
	AT4_LogParser.cpp
)

set(AT4_H_SOURCES
    stdafx.h
	AT4_MainWindow.h
	AT4_Incoming.h
	AT4_Interface.h
	AT4_System.h
	AT4_Config.h
	AT4_MessageQueue.h
	AT4_Message.h
	AT4_LuaVirtualMachine.h
	AT4_LogParser.h
)

set(AT4_RC_SOURCES
    AT4.qrc
)

set(AT4_PRECOMPILE_FILE
	stdafx.cpp)

foreach(filename ${AT4_CHILD_WND_SOURCES} ${AT4_CPP_SOURCES} ${AT4_DATA_SOURCES}
	${AT4_SETTING_WND_SOURCES} ${AT4_SCRIPT_EDITOR_WND_SOURCES} ${AT4_SESSION_SOURCES})
	get_filename_component(ext ${filename} LAST_EXT ABSOLUTE)
	string(TOLOWER ${ext} ext_lower)
	if(${ext_lower} STREQUAL ".cpp" )
		set_source_files_properties(
			${filename}
			PROPERTIES
			COMPILE_FLAGS "/Yustdafx.h"
		)
	endif()
endforeach()
    
set_source_files_properties(${AT4_PRECOMPILE_FILE}
	PROPERTIES
	COMPILE_FLAGS "/Ycstdafx.h"
)

add_executable(AxTrace4 WIN32 
	${AT4_RC_SOURCES}
	${AT4_DATA_SOURCES}
	${AT4_CHILD_WND_SOURCES}
	${AT4_SETTING_WND_SOURCES}
	${AT4_SCRIPT_EDITOR_WND_SOURCES}
	${AT4_SESSION_SOURCES}
	${AT4_CPP_SOURCES}
	${AT4_H_SOURCES}
	${AT4_PRECOMPILE_FILE}
	AxTrace.rc
)

target_include_directories(AxTrace4
PRIVATE
	${LuaJIT_DIR}/include
)

target_link_directories(AxTrace4
PRIVATE
	${LuaJIT_DIR}/lib
)

target_link_libraries(AxTrace4 
    Qt6::Core
    Qt6::Gui
	Qt6::Widgets
	Qt6::OpenGLWidgets
	cyclone::cyclone
	lua51
	ws2_32 
	Shlwapi
)

#install
install(TARGETS AxTrace4
    DESTINATION .)

install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}/Qt6Widgets$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)
	
install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}/Qt6Core$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}/Qt6Gui$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}/Qt6OpenGL$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}/Qt6OpenGLWidgets$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}/platforms/qwindows$<$<CONFIG:Debug>:d>.dll
	DESTINATION plugins/platforms)
