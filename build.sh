#!/bin/bash
QT_PATH="~/Qt/6.10.1/gcc_64/lib/cmake/Qt6"

mkdir -p build
if [ $1 = "debug" ]; then
    cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build/ -DQt6_DIR=$QT_PATH 
    cmake --build build/
else
    cmake -S . -B build/ -DQt6_DIR=$QT_PATH 
    cmake --build build/
fi
