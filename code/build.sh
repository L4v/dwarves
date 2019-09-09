#!/bin/bash
mkdir -p ../build
pushd ../build
g++ -Wall -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -Werror -fno-rtti -Wl,-rpath,'$ORIGIN' -Wno-write-strings -Wno-unused-variable -g ../code/sdl_dwarves.cpp -o dwarves -lGL -lGLEW `sdl2-config --cflags --libs`
popd

