cmake_minimum_required(VERSION 3.2)
project(dwarves)

set(CMAKE_MODULE_PATH $(PROJECT_SOURCE_DIR)/cmake)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

set(EXECUTABLE_NAME "dwarves")
set(CMAKE_CXX_FLAGS "-g")

#file(GLOB HEADERS "*.h")
file(GLOB SOURCES "sdl_dwarves.cpp")

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} GLEW::GLEW)
