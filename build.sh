#!/bin/bash

mkdir -p obj
set -x
g++ -Wall -std=c++11 -fexceptions -O2  -c grid.cpp -o obj/grid.o
g++ -Wall -std=c++11 -fexceptions -O2  -c main.cpp -o obj/main.o
g++  -o sudoku obj/grid.o obj/main.o  -s

