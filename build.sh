#!/usr/bin/bash

CXX=g++
CXFLAGS="-c -Wall -Wextra -pedantic -Isrc $(pkg-config allegro-5 allegro_primitives-5 --cflags)"

LD=g++
LDFLAGS=
LDLIBS="-lm $(pkg-config allegro-5 allegro_primitives-5 --libs)"

EXECUTABLE="nbodysim"

mkdir -p build
rm -f build/*.o

$CXX $CXFLAGS src/main.cpp -o build/main.o
$CXX $CXFLAGS src/quadtree.cpp -o build/quadtree.o
$CXX $CXFLAGS src/bounding_box.cpp -o build/bounding_box.o
$CXX $CXFLAGS src/vector.cpp -o build/vector.o
$CXX $CXFLAGS src/physics.cpp -o build/physics.o

$LD $LDFLAGS build/main.o \
        build/quadtree.o \
        build/bounding_box.o \
        build/vector.o \
        build/physics.o \
        $LDLIBS \
        -o $EXECUTABLE
