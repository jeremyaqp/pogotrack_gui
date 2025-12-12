#!/bin/bash
QT_PATH="~/Qt/6.10.1/gcc_64/lib/cmake/Qt6"

mkdir -p build
cmake -S . -B build/ -DQt6_DIR=$QT_PATH
cmake --build build/
