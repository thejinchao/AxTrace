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
)
source_group("ChildWindow" FILES ${AT4_CHILD_WND_SOURCES})

set(AT4_SETTING_WND_SOURCES
	SettingWindow/AT4_SettingDialog.h
	SettingWindow/AT4_SettingDialog.cpp
	SettingWindow/AT4_ExtensionProperty.h
	SettingWindow/AT4_ExtensionProperty.cpp
	SettingWindow/AT4_ScriptVariant.h
	SettingWindow/AT4_ScriptVariant.cpp
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

set(AT4_CPP_SOURCES
    AT4_Main.cpp
    AT4_MainWindow.cpp
	AT4_Incoming.cpp
	AT4_System.cpp
	AT4_Config.cpp
	AT4_MessageQueue.cpp
	AT4_Message.cpp
	AT4_Filter.cpp
	AT4_Session.cpp
	AT4_LogColumn.cpp
	AT4_LogParser.cpp
	AT4_SessionDialog.cpp
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
	AT4_Filter.h
	AT4_Session.h
	AT4_LogColumn.h
	AT4_LogParser.h
	AT4_SessionDialog.h
)

set(AT4_RC_SOURCES
    AT4.qrc
)

set(AT4_PRECOMPILE_FILE
	stdafx.cpp)

foreach(filename ${AT4_CHILD_WND_SOURCES} ${AT4_CPP_SOURCES} ${AT4_SETTING_WND_SOURCES} ${AT4_SCRIPT_EDITOR_WND_SOURCES})
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

include_directories(
	$ENV{CYCLONE_SDK_ROOT}/include
	../luajit
	../QtPropertyBrowser
)

link_directories(
	$ENV{CYCLONE_SDK_ROOT}/lib
)

add_executable(AxTrace4 WIN32 
	${AT4_RC_SOURCES}
	${AT4_CHILD_WND_SOURCES}
	${AT4_SETTING_WND_SOURCES}
	${AT4_SCRIPT_EDITOR_WND_SOURCES}
	${AT4_CPP_SOURCES}
	${AT4_H_SOURCES}
	${AT4_PRECOMPILE_FILE}
	AxTrace.rc
)

target_link_libraries(AxTrace4 
    Qt6::Core
    Qt6::Gui
	Qt6::Widgets
	Qt6::OpenGLWidgets
	QtPropertyBrowser
	lua51
	cyclone 
	ws2_32 
	Shlwapi
)
add_dependencies(AxTrace4 lua51 QtPropertyBrowser)
set_target_properties(AxTrace4 
    PROPERTIES LINK_FLAGS "${CMAKE_CURRENT_BINARY_DIR}/../luajit/_build/lj_vm.obj"
)

#install
install(TARGETS AxTrace4
    DESTINATION .)

install(FILES $ENV{QT6_SDK_ROOT}/bin/Qt6Widgets$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)
	
install(FILES $ENV{QT6_SDK_ROOT}/bin/Qt6Core$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES $ENV{QT6_SDK_ROOT}/bin/Qt6Gui$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES $ENV{QT6_SDK_ROOT}/bin/Qt6OpenGL$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES $ENV{QT6_SDK_ROOT}/bin/Qt6OpenGLWidgets$<$<CONFIG:Debug>:d>.dll
	DESTINATION .)

install(FILES $ENV{QT6_SDK_ROOT}/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll
	DESTINATION plugins/platforms)
