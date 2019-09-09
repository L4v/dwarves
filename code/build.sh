#!/bin/bash
mkdir -p ../build
pushd ../build
CommonCompilerFlags=(-Wall -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -Werror -fno-rtti -Wl,-rpath,'$ORIGIN'
		     -Wno-unused-function -Wno-write-strings -Wno-unused-variable -g)
CommonLinkerFlags=(-lGL -lGLEW `sdl2-config --cflags --libs`)
g++  ${CommonCompilerFlags[*]} ../code/sdl_dwarves.cpp -o dwarves ${CommonLinkerFlags[*]}
popd

