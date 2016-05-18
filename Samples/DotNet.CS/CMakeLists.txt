#
#		AxTrace
#Copyright(C) thecodeway.com
#
#
cmake_minimum_required (VERSION 3.0)

#copy cs project file as configure files
set(DOTNET_CS_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(DOTNET_CS_PROJ_FILE ${DOTNET_CS_BINARY_PATH}/DotNet.CS.csproj)

string(REPLACE "/" "\\"
	DOTNET_CS_SOURCE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DotNet.CS.csproj.in
	${DOTNET_CS_PROJ_FILE}
)

include_external_msproject(
    DotNet.CS ${DOTNET_CS_PROJ_FILE}
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

set_property(TARGET DotNet.CS PROPERTY FOLDER "Samples")
