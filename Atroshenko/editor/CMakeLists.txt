cmake_minimum_required(VERSION 3.8)
project(atroshenko_editor)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES main.c
				 ../common/src/err_handling.c)
include_directories(../common/include)
set(EXECUTABLE_NAME atroshenko_editor)
add_executable(atroshenko_editor ${SOURCE_FILES})

# Project configuration
add_definitions(-D_EXECUTABLE_NAME="${EXECUTABLE_NAME}")
