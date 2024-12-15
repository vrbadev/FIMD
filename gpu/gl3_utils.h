/// \file gl3_utils.h
/// \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
/// \date May 2023
/// \brief This header file contains auxiliary functions and structs definitions extending OpenGL ES3.X.
/// \copyright GNU Public License.

#pragma once

#ifndef GLES3COMPUTELIB_GL3_UTILS_H
#define GLES3COMPUTELIB_GL3_UTILS_H

#ifdef OPENGL_ES_32
#include <GLES3/gl32.h>
#else
#include <GLES3/gl31.h>
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#endif

/// Structure of RGBA pixel containing 8-bit unsigned channels.
typedef struct rgba_s {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;
} rgba_t;

/// Structure of 2D vector containing 32-bit signed components.
typedef struct ivec2_s {
    GLint x;
    GLint y;
} ivec2_t;

/// Structure of 3D vector containing 32-bit signed components.
typedef struct ivec3_s {
    GLint x;
    GLint y;
    GLint z;
} ivec3_t;

/// Structure of 4D vector containing 32-bit signed components.
typedef struct ivec4_s {
    GLint x;
    GLint y;
    GLint z;
    GLint w;
} ivec4_t;

/// Structure of 2D vector containing 32-bit unsigned components.
typedef struct uvec2_s {
    GLuint x;
    GLuint y;
} uvec2_t;

/// Structure of 3D vector containing 32-bit unsigned components.
typedef struct uvec3_s {
    GLuint x;
    GLuint y;
    GLuint z;
} uvec3_t;

/// Structure of 4D vector containing 32-bit unsigned components.
typedef struct uvec4_s {
    GLuint x;
    GLuint y;
    GLuint z;
    GLuint w;
} uvec4_t;

/// Structure of 2D vector containing 32-bit float components.
typedef struct vec2_s {
    GLfloat x;
    GLfloat y;
} vec2_t;

/// Structure of 3D vector containing 32-bit float components.
typedef struct vec3_s {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} vec3_t;

/// Structure of 4D vector containing 32-bit float components.
typedef struct vec4_s {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
} vec4_t;


/// Gets name of the OpenGL defined constant by its value.
/// \param define_value Value of the OpenGL constant.
/// \return Name of the preprocessor constant.
const GLchar* gl3_get_define_name(GLuint64 define_value);

/// Gets size of OpenGL type in bytes.
/// \param type OpenGL type.
/// \return Size in bytes.
GLsizei gl3_get_type_size(GLenum type);

/// Gets number of pixel components based on the OpenGL image format.
/// \param format OpenGL image format.
/// \return Number of pixel components.
GLuint gl3_get_image_format_num_components(GLenum format);

/// Gets internal format compatible format for image2d qualifiers.
/// \param internal_format Internal format GL constant of the image2d.
/// \return Compatibility format for the image2d.
GLenum gl3_get_image2d_compatibility_format(GLenum internal_format);

/// Gets GLSL format qualifier string based on the internal-compatible format of the image2d.
/// \param compatibility_format Compatibility format GL constant.
/// \return String containing format qualifier for GLSL source.
const GLchar* gl3_get_glsl_image2d_format_qualifier(GLenum compatibility_format);

/// Gets GLSL variable type string (image2d, uimage2d, iimage2d) based on the compatibility format.
/// \param compatibility_format Compatibility format GL constant.
/// \return String containing variable type for GLSL source.
const GLchar* gl3_get_glsl_image2d_type(GLenum compatibility_format);

/// Gets GLSL access type string (readonly, writeonly or empty string).
/// \param access Access type GL constant.
/// \return String containing access type for GLSL source.
const GLchar* gl3_get_glsl_image2d_access(GLenum access);

/// Gets GLSL data type string.
/// \param type Data type GL constant.
/// \return String containing data type for GLSL source.
const GLchar* gl3_get_glsl_data_type(GLenum type);

#endif // GLES3COMPUTELIB_GL3_UTILS_H
