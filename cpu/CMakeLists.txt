cmake_minimum_required(VERSION 3.12)
project(fimd_cpu C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-O3 -g -save-temps)

set(IM_WIDTH 752)
set(IM_HEIGHT 480)
set(FIMD_THRESHOLD_CENTER 120)
set(FIMD_THRESHOLD_DIFF 60)
set(FIMD_THRESHOLD_SUN 240)
set(FIMD_MAX_MARKERS_COUNT 300)
set(FIMD_MAX_SUN_PTS_COUNT 50000)
set(FIMD_TERM_SEQ 0x00FF)

add_compile_definitions(IM_WIDTH=${IM_WIDTH})
add_compile_definitions(IM_HEIGHT=${IM_HEIGHT})
add_compile_definitions(FIMD_THRESHOLD_CENTER=${FIMD_THRESHOLD_CENTER})
add_compile_definitions(FIMD_THRESHOLD_DIFF=${FIMD_THRESHOLD_DIFF})
add_compile_definitions(FIMD_THRESHOLD_SUN=${FIMD_THRESHOLD_SUN})
add_compile_definitions(FIMD_MAX_MARKERS_COUNT=${FIMD_MAX_MARKERS_COUNT})
add_compile_definitions(FIMD_MAX_SUN_PTS_COUNT=${FIMD_MAX_SUN_PTS_COUNT})
add_compile_definitions(FIMD_TERM_SEQ=${FIMD_TERM_SEQ})

file(REAL_PATH "${PROJECT_SOURCE_DIR}/generate.py" GEN_SCRIPT_PATH)
file(REAL_PATH "${PROJECT_SOURCE_DIR}/template.c" TEMPLATE_PATH)

message("[Code generation for FIMD-CPU]")
set(FIMD_RADII 2 3 4 5)  # Predefined set of radii
set(GENERATED_SOURCES)

foreach(FIMD_RADIUS IN LISTS FIMD_RADII)
    set(GEN_SOURCE_PATH "${CMAKE_CURRENT_BINARY_DIR}/fimd_r${FIMD_RADIUS}.c")
    message(" -- radius ${FIMD_RADIUS}: ${TEMPLATE_PATH} -> ${GEN_SOURCE_PATH}")
    add_custom_command(
            OUTPUT ${GEN_SOURCE_PATH}
            COMMAND python ${GEN_SCRIPT_PATH} -t ${TEMPLATE_PATH} -o ${GEN_SOURCE_PATH} -r ${FIMD_RADIUS}
            DEPENDS ${TEMPLATE_PATH}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            VERBATIM
    )
    list(APPEND GENERATED_SOURCES ${GEN_SOURCE_PATH})
endforeach()

# Target: Library archive file
add_executable(${PROJECT_NAME} main.c ${GENERATED_SOURCES})

