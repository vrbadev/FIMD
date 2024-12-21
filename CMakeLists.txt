cmake_minimum_required(VERSION 3.12)
project(FIMD C CXX ASM)

# add subprojects FIMD-CPU and FIMD-GPU
add_subdirectory(cpu)
add_subdirectory(gpu)

# include header files "fimd_cpu.h" and "fimd_gpu.h"
include_directories(cpu gpu)

# add dummy target "FIMD"
add_custom_target(${PROJECT_NAME})

# this target compiles shared library files "libfimd_cpu.so" and "libfimd_gpu.so"
add_dependencies(${PROJECT_NAME} fimd_cpu fimd_gpu)

# output shared library absolute paths
file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/cpu/libfimd_cpu.so" FIMD_CPU_LIB_PATH)
file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/gpu/libfimd_gpu.so" FIMD_GPU_LIB_PATH)
