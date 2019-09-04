#!/bin/bash
g++ -g sdl_dwarves.cpp -o dwarves -lGL `sdl2-config --cflags --libs`

