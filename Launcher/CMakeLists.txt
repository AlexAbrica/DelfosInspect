#Generated by VisualGDB project wizard.
#Note: VisualGDB will automatically update this file when you add new sources to the project.

cmake_minimum_required(VERSION 2.7)
project(Launcher)
set(LIBRARIES_FROM_REFERENCES "")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(Launcher Launcher.cpp MainWindow.cpp MainWindow.ui cManager.cpp cManager.h)
target_link_libraries(Launcher opencv_core Qt5::Widgets "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET Launcher PROPERTY CXX_STANDARD 14)
