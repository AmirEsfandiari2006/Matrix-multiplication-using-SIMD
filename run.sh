#!/bin/bash

SRC_DIR="src"
BUILD_DIR="build"
INCLUDE_DIR="include"
LIB_DIR="lib"

mkdir -p $BUILD_DIR

echo "--- Step 1: Assembling the SIMD Core ---"
nasm -f elf64 $SRC_DIR/math_core.asm -o $BUILD_DIR/math_core.o
if [ $? -ne 0 ]; then echo "Assembly failed!"; exit 1; fi

echo "--- Step 2: Compiling C and Linking with Raylib ---"
gcc $SRC_DIR/main.c $BUILD_DIR/math_core.o -o $BUILD_DIR/engine \
    -I./$INCLUDE_DIR \
    -L./$LIB_DIR \
    -lraylib -lm -lpthread -ldl -lrt -lX11

if [ $? -ne 0 ]; then echo "Compilation failed!"; exit 1; fi

echo "--- Step 3: Running the Engine ---"
./$BUILD_DIR/engine