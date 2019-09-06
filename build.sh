#!/bin/bash
g++ -g sdl_dwarves.cpp -o dwarves -lGL -lGLEW `sdl2-config --cflags --libs`

