cmake_minimum_required(VERSION 3.2)
project(dwarves)

set(CMAKE_MODULE_PATH $(PROJECT_SOURCE_DIR)/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

set(EXECUTABLE_NAME "dwarves")
set(CMAKE_CXX_FLAGS "-g -DSLOW_BUILD=1 -DINTERNAL_BUILD=1")

#file(GLOB HEADERS "code/*.h")
file(GLOB SOURCES "code/sdl_dwarves.cpp")

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} GLEW::GLEW)
