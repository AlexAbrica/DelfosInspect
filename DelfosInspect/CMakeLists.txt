#Generated by VisualGDB project wizard.
#Note: VisualGDB will automatically update this file when you add new sources to the project.

cmake_minimum_required(VERSION 2.7)
project(DelfosInspect)
set(LIBRARIES_FROM_REFERENCES "")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(DelfosInspect DelfosInspect.cpp MainWindow.cpp MainWindow.ui cCamera.h cCamera.cpp mat2qimage.cpp mat2qimage.h cInspectMachine.h cInspectMachine.cpp ClickedLabel.cpp ClickedLabel.h)
target_link_libraries(DelfosInspect wiringpi opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui opencv_features2d opencv_calib3d Qt5::Widgets "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET DelfosInspect PROPERTY CXX_STANDARD 14)
