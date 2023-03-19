#!/bin/bash
export BUILD_DIR=build

# Windows build
x86_64-w64-mingw32-g++ -o $BUILD_DIR/exercise3.exe $SOURCE_DIR/exercise3.cpp -static-libstdc++ -static-libgcc -static

# Linux build
g++ -o $BUILD_DIR/exercise3.o $SOURCE_DIR/exercise3.cpp