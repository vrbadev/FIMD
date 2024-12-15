cmake_minimum_required(VERSION 3.12)
project(cpu C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-O3 -g -save-temps)

# Target: Library archive file
add_library(${PROJECT_NAME} SHARED ${SHADER_OBJECTS} cpu.c)


