cmake_minimum_required(VERSION 3.15)

project(AxTrace4)

# find Qt6
set(CMAKE_PREFIX_PATH $ENV{QT6_SDK_ROOT}/lib/cmake)
find_package(Qt6 COMPONENTS Widgets OpenGLWidgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# add subdirectory
add_subdirectory(luajit)
add_subdirectory(QtPropertyBrowser)
add_subdirectory(axtrace)

#install
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/run)
