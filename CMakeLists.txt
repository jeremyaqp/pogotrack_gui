cmake_minimum_required(VERSION 3.10)
project(pogotrack_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "/usr/local/lib:/usr/lib")

if(APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../Frameworks:/usr/local/lib:/opt/homebrew/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

qt_add_resources(QT_RESOURCES resources.qrc)

add_executable(pogotrack_gui
    src/main.cpp
    src/ImageDisplay.h
    src/ImageDisplay.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Filter.h
    ${QT_RESOURCES}
)

target_link_libraries(pogotrack_gui
    Qt6::Widgets
    ${OpenCV_LIBS}
)
