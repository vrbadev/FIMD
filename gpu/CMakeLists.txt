cmake_minimum_required(VERSION 3.12)
project(fimd_gpu C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_compile_definitions(_GNU_SOURCE)
add_compile_options(-O3)

# Target: Shared library (libfimd_gpu.so)
add_library(${PROJECT_NAME} STATIC)

message("[Available render devices and cards]")
file(GLOB RENDER_DEVICES "/dev/dri/renderD*" "/dev/dri/card*")
string(JOIN "," RENDER_DEVICES_STR ${RENDER_DEVICES})
message("-- ${RENDER_DEVICES_STR}")
add_compile_definitions(RENDER_DEVICES="${RENDER_DEVICES_STR}")

message("[Including GLES library]")
include(cmake/FindOpenGLES.cmake)
message("-- Found version: " ${GLES_VERSION_STRING})
message("-- Shared object location: " ${GLES_LIBRARIES})
message("-- Include directory location: " ${GLES_INCLUDE_DIRS})

message("[Including EGL library]")
include(cmake/FindEGL.cmake)
message("-- Found version: " ${EGL_VERSION_STRING})
message("-- Shared object location: " ${EGL_LIBRARIES})
message("-- Include directory location: " ${EGL_INCLUDE_DIRS})

message("[Including GBM library]")
include(cmake/FindGBM.cmake)
message("-- Found version: " ${GBM_VERSION})
message("-- Shared object location: " ${GBM_LIBRARIES})
message("-- Include directory location: " ${GBM_INCLUDE_DIRS})

include_directories(${EGL_INCLUDE_DIRS} ${GBM_INCLUDE_DIRS} ${GLES_INCLUDE_DIRS})

message("[Completed all cmake includes]")


# Find all GLSL shader sources and add object targets to be appended to the final library binary
message("[Object files from shader source files]")
file(GLOB_RECURSE SHADER_FILES_PATHS CONFIGURE_DEPENDS "*.comp" "*.vert" "*.frag" "*.tess" "*.geom" "*.glsl")
set(SHADER_OBJECTS)
set(index "1")
foreach(SHADER_FILE_PATH ${SHADER_FILES_PATHS})
    file(RELATIVE_PATH SHADER_FILE ${PROJECT_SOURCE_DIR} ${SHADER_FILE_PATH})
    get_filename_component(SHADER_OBJ ${SHADER_FILE_PATH} NAME_WE)
    set(SHADER_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_OBJ}.o")
    message("-- [#" ${index} "] " ${SHADER_FILE} " -> " ${SHADER_OBJ})
    add_custom_command(
            OUTPUT ${SHADER_OBJ} ${SHADER_OBJ}.txt
            COMMAND ld -r -z noexecstack -b binary -o ${SHADER_OBJ} ${SHADER_FILE} && nm ${SHADER_OBJ} > ${SHADER_OBJ}.txt
            DEPENDS ${SHADER_FILE}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            VERBATIM
    )
    #file(READ ${SHADER_OBJ}.txt SHADER_OBJ_CONTENT)
    #string(REGEX REPLACE "\n" ", " SHADER_OBJ_CONTENT ${SHADER_OBJ_CONTENT})
    #message(" -> Content: " ${SHADER_OBJ_CONTENT})
    list(APPEND SHADER_OBJECTS ${SHADER_OBJ})
    math(EXPR index "${index}+1")
endforeach()

target_sources(${PROJECT_NAME} PRIVATE ${SHADER_OBJECTS} fimd_gpu.c gl3_utils.c queue.c compute_lib.c)

target_link_libraries(${PROJECT_NAME} PUBLIC ${EGL_LIBRARIES} ${GBM_LIBRARIES} ${GLES_LIBRARIES})

