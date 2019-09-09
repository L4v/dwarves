#!/bin/bash
mkdir -p ../build
pushd ../build
g++ -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -g ../code/sdl_dwarves.cpp -o dwarves -lGL -lGLEW `sdl2-config --cflags --libs`
popd

