cmake_minimum_required(VERSION 4.0)

project(AxTrace4)

# find Qt6
# Download and install Qt6 online installer for open source users: https://www.qt.io/development/download-qt-installer-oss
# Set cmake variable Qt6_DIR="<Qt6_prebuilt_path>/lib/cmake/Qt6"
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# find cyclone
# Download, compile and install cyclone: https://github.com/thejinchao/cyclone
# Set cmake variable cyclone_DIR="<cyclone_sdk_path>/lib/cmake/cyclone"
find_package(cyclone REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# add subdirectory
add_subdirectory(luajit)
add_subdirectory(axtrace)
