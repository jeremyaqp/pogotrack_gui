cmake_minimum_required(VERSION 3.10)
project(pogotrack_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(pogotrack_gui
    src/main.cpp
    src/ImageDisplay.h
    src/ImageDisplay.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Filter.h
)

target_link_libraries(pogotrack_gui
    Qt6::Widgets
    ${OpenCV_LIBS}
)
