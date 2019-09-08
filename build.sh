#!/bin/bash
g++ -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -g sdl_dwarves.cpp -o dwarves -lGL -lGLEW `sdl2-config --cflags --libs`

