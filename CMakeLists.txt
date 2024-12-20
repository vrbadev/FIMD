cmake_minimum_required(VERSION 3.12)
project(FIMD C CXX ASM)

add_subdirectory(cpu)
add_subdirectory(gpu)

add_custom_target(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} fimd_cpu gpu)
