cmake_minimum_required(VERSION 3.8)
project(interface)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES main.c
				 ../common/src/err_handling.c)
include_directories(../common/include)
set(EXECUTABLE_NAME interface)
add_executable(interface ${SOURCE_FILES})

# Project configuration
add_definitions(-D_EXECUTABLE_NAME="${EXECUTABLE_NAME}")
add_definitions(-D_EDITOR_EXECUTABLE="/usr/bin/atroshenko_editor")