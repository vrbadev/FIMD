cmake_minimum_required(VERSION 3.12)
project(fimd_cpu C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-O3 -g) # -save-temps

# Target: Shared library (libfimd_cpu.so)
add_library(${PROJECT_NAME} SHARED)

set(IM_WIDTH 752)
set(IM_HEIGHT 480)
set(FIMD_THRESHOLD_CENTER 120)
set(FIMD_THRESHOLD_DIFF 60)
set(FIMD_THRESHOLD_SUN 240)
set(FIMD_MAX_MARKERS_COUNT 300)
set(FIMD_MAX_SUN_PTS_COUNT 50000)
set(FIMD_TERM_SEQ 0x00FF)

target_compile_definitions(${PROJECT_NAME} PRIVATE IM_WIDTH=${IM_WIDTH})
target_compile_definitions(${PROJECT_NAME} PRIVATE IM_HEIGHT=${IM_HEIGHT})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_THRESHOLD_CENTER=${FIMD_THRESHOLD_CENTER})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_THRESHOLD_DIFF=${FIMD_THRESHOLD_DIFF})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_THRESHOLD_SUN=${FIMD_THRESHOLD_SUN})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_MAX_MARKERS_COUNT=${FIMD_MAX_MARKERS_COUNT})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_MAX_SUN_PTS_COUNT=${FIMD_MAX_SUN_PTS_COUNT})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_TERM_SEQ=${FIMD_TERM_SEQ})

file(REAL_PATH "${PROJECT_SOURCE_DIR}/generate.py" GEN_SCRIPT_PATH)
file(REAL_PATH "${PROJECT_SOURCE_DIR}/template.c" TEMPLATE_PATH)

message("[Code generation for FIMD-CPU]")

set(FIMD_RADII)  # Predefined set of radii
foreach(RADIUS RANGE 1 10 1)
    list(APPEND FIMD_RADII ${RADIUS})
endforeach()
string(JOIN "," FIMD_RADII_STR ${FIMD_RADII})
list(LENGTH FIMD_RADII FIMD_RADII_COUNT)
message("-- radii (${FIMD_RADII_COUNT}): ${FIMD_RADII_STR}")

target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_RADII=${FIMD_RADII_STR})
target_compile_definitions(${PROJECT_NAME} PRIVATE FIMD_RADII_COUNT=${FIMD_RADII_COUNT})

set(GENERATED_SOURCES)

foreach(FIMD_RADIUS IN LISTS FIMD_RADII)
    set(GEN_SOURCE_PATH "${CMAKE_CURRENT_BINARY_DIR}/fimd_r${FIMD_RADIUS}.c")
    message("-- radius ${FIMD_RADIUS}: ${TEMPLATE_PATH} -> ${GEN_SOURCE_PATH}")
    add_custom_command(
            OUTPUT ${GEN_SOURCE_PATH}
            COMMAND python ${GEN_SCRIPT_PATH} -t ${TEMPLATE_PATH} -o ${GEN_SOURCE_PATH} -r ${FIMD_RADIUS}
            DEPENDS ${TEMPLATE_PATH}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            VERBATIM
    )
    list(APPEND GENERATED_SOURCES ${GEN_SOURCE_PATH})
endforeach()

message("[Code generation done]")

target_sources(${PROJECT_NAME} PRIVATE fimd_cpu.c ${GENERATED_SOURCES})

add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/../"
        COMMAND mkdir -p lib && cp "$<TARGET_FILE:${PROJECT_NAME}>" "lib/libfimd_cpu.so"
        DEPENDS ${PROJECT_NAME}
        VERBATIM
)

# Target: Executable example (fimd_cpu_example)
add_executable(${PROJECT_NAME}_example example.c)
target_link_libraries(${PROJECT_NAME}_example ${PROJECT_NAME})

