#!/bin/bash
mkdir -p ../build
pushd ../build
CommonCompilerFlags=(-Wall -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -Werror -fno-rtti -Wl,-rpath,'$ORIGIN'
		     -Wno-unused-function -Wno-write-strings -Wno-unused-variable -g -Wno-null-dereference)
CommonLinkerFlags=(-lGL -lGLEW `sdl2-config --cflags --libs` -ldl)
#g++  ${CommonCompilerFlags[*]} -shared -Wl,-soname -fPIC ../code/dwarves.cpp -o libdwarves.so -uGameUpdateAndRender -uGameGetSoundSamples ${CommonLinkerFlags[*]}
#g++  ${CommonCompilerFlags[*]} -L. -ldwarves ../code/sdl_dwarves.cpp -o dwarves ${CommonLinkerFlags[*]}

clang++ ${CommonCompilerFlags[*]} -shared -o dwarves.so -fpic ../code/dwarves.cpp #-Wl,-Map,output.map,--gc-sections

clang++ ${CommonCompilerFlags[*]} ../code/sdl_dwarves.cpp ${CommonLinkerFlags[*]} -Wl,-Map,linux32_output.map,--gc-sections -o dwarves

popd

