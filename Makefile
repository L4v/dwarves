# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hk-47/workspace/op/cpp/games/dwarves

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hk-47/workspace/op/cpp/games/dwarves

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/hk-47/workspace/op/cpp/games/dwarves/CMakeFiles /home/hk-47/workspace/op/cpp/games/dwarves/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/hk-47/workspace/op/cpp/games/dwarves/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named dwarves

# Build rule for target.
dwarves: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 dwarves
.PHONY : dwarves

# fast build rule for target.
dwarves/fast:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/build
.PHONY : dwarves/fast

dwarves.o: dwarves.cpp.o

.PHONY : dwarves.o

# target to build an object file
dwarves.cpp.o:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves.cpp.o
.PHONY : dwarves.cpp.o

dwarves.i: dwarves.cpp.i

.PHONY : dwarves.i

# target to preprocess a source file
dwarves.cpp.i:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves.cpp.i
.PHONY : dwarves.cpp.i

dwarves.s: dwarves.cpp.s

.PHONY : dwarves.s

# target to generate assembly for a file
dwarves.cpp.s:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves.cpp.s
.PHONY : dwarves.cpp.s

dwarves_sdl.o: dwarves_sdl.cpp.o

.PHONY : dwarves_sdl.o

# target to build an object file
dwarves_sdl.cpp.o:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves_sdl.cpp.o
.PHONY : dwarves_sdl.cpp.o

dwarves_sdl.i: dwarves_sdl.cpp.i

.PHONY : dwarves_sdl.i

# target to preprocess a source file
dwarves_sdl.cpp.i:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves_sdl.cpp.i
.PHONY : dwarves_sdl.cpp.i

dwarves_sdl.s: dwarves_sdl.cpp.s

.PHONY : dwarves_sdl.s

# target to generate assembly for a file
dwarves_sdl.cpp.s:
	$(MAKE) -f CMakeFiles/dwarves.dir/build.make CMakeFiles/dwarves.dir/dwarves_sdl.cpp.s
.PHONY : dwarves_sdl.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... dwarves"
	@echo "... edit_cache"
	@echo "... dwarves.o"
	@echo "... dwarves.i"
	@echo "... dwarves.s"
	@echo "... dwarves_sdl.o"
	@echo "... dwarves_sdl.i"
	@echo "... dwarves_sdl.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

