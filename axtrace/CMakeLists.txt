set(AT4_CPP_SOURCES
    AT4_Main.cpp
    AT4_MainWindow.cpp
    AT4_ChildWnd_Log.cpp
	AT4_Incoming.cpp
	AT4_System.cpp
	AT4_Config.cpp
	AT4_MessageQueue.cpp
	AT4_ChildWnd_Value.cpp
	AT4_Message.cpp
	AT4_Filter.cpp
	AT4_ChildWnd_Scene2D.cpp
	AT4_Scene2D.cpp
	AT4_Camera2D.cpp
	AT4_SettingDialog.cpp
	AT4_LuaHighlighter.cpp
	AT4_Session.cpp
	AT4_LogColumn.cpp
	AT4_LogData.cpp
	AT4_ScriptEditDialog.cpp
	AT4_CodeEditor.cpp
	AT4_ScriptVariant.cpp
	AT4_ExtensionProperty.cpp
	AT4_SessionDialog.cpp
	AT4_ChildInterface.cpp)

set(AT4_H_SOURCES
    stdafx.h
	AT4_MainWindow.h
    AT4_ChildWnd_Log.h
	AT4_Incoming.h
	AT4_Interface.h
	AT4_System.h
	AT4_Config.h
	AT4_MessageQueue.h
	AT4_ChildWnd_Value.h
	AT4_Message.h
	AT4_Filter.h
	AT4_ChildInterface.h
	AT4_ChildWnd_Scene2D.h
	AT4_Scene2D.h
	AT4_Camera2D.h
	AT4_SettingDialog.h
	AT4_LuaHighlighter.h
	AT4_Session.h
	AT4_LogColumn.h
	AT4_LogData.h
	AT4_ScriptEditDialog.h
	AT4_CodeEditor.h
	AT4_ScriptVariant.h
	AT4_ExtensionProperty.h
	AT4_SessionDialog.h)

set(AT4_RC_SOURCES
    AT4.qrc
)

set(AT4_PRECOMPILE_FILE
	stdafx.cpp)

foreach(src_file ${AT4_CPP_SOURCES})
	set_source_files_properties(
		${src_file}
		PROPERTIES
		COMPILE_FLAGS "/Yustdafx.h"
	)
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
