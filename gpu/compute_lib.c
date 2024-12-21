/*
 * \file compute_lib.c
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Source file for the compute shader library.
 * \copyright GNU Public License.
 */

#include "compute_lib.h"

/// Attributes for configuring the EGL context.
static const EGLint egl_config_attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, EGL_NONE };
/// Attributes for creating an OpenGL ES 3.0 context.
static const EGLint egl_ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };


/// OpenGL debug callback function (GLDEBUGPROC) pushing debug messages to the library instance's error queue.
/// \param source The source of error message.
///                 Possible values: GL_DEBUG_SOURCE_API, GL_DEBUG_SOURCE_WINDOW_SYSTEM_, GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_SOURCE_OTHER.
/// \param type The type of error message.
///               Possible values: GL_DEBUG_TYPE_ERROR, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DEBUG_TYPE_PORTABILITY, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_TYPE_MARKER, GL_DEBUG_TYPE_PUSH_GROUP, GL_DEBUG_TYPE_POP_GROUP, GL_DEBUG_TYPE_OTHER.
/// \param id The ID of the error message assigned by OpenGL.
/// \param severity The severity of error message.
///                   Possible values: GL_DEBUG_SEVERITY_NOTIFICATION, GL_DEBUG_SEVERITY_LOW, GL_DEBUG_SEVERITY_MEDIUM, GL_DEBUG_SEVERITY_HIGH.
/// \param length The length of the error message (number of characters).
/// \param message The error message string.
/// \param inst Pointer to the GLES3ComputeLib library instance.
static void compute_lib_gl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, compute_lib_instance_t* inst)
{
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            if (inst->verbosity == GL_DEBUG_SEVERITY_HIGH || inst->verbosity == 1) break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            if (inst->verbosity == GL_DEBUG_SEVERITY_MEDIUM || inst->verbosity == 2) break;
        case GL_DEBUG_SEVERITY_LOW:
            if (inst->verbosity == GL_DEBUG_SEVERITY_LOW || inst->verbosity == 3) break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            if (inst->verbosity == GL_DEBUG_SEVERITY_NOTIFICATION || inst->verbosity == 4) break;
        default:
            return;
    }

    compute_lib_error_t* err = (compute_lib_error_t*) calloc(1, sizeof(compute_lib_error_t));
    err->message_len = length;
    if (message[length-1] == '\n') err->message_len -= 1;
    err->message = (GLchar*) calloc(err->message_len + 1, sizeof(GLchar));
    memcpy(err->message, message, err->message_len);
    err->err_id = inst->error_total_cnt;
    err->id = id;
    err->type = type;
    err->severity = severity;
    err->source = source;

    queue_push(inst->error_queue, (void*) err);
    inst->error_total_cnt++;
    inst->last_error = err;
}


/// Pushes lines from OpenGL program log to the library instance's error queue.
/// \param program Pointer to the GLES3ComputeLib program instance.
/// \return Number of captured OpenGL errors.
static GLuint compute_lib_program_log_to_queue(compute_lib_program_t* program)
{
    if (program->handle == 0) return 0;
    GLint log_len = 0;
    glGetProgramiv(program->handle, GL_INFO_LOG_LENGTH, &log_len);
    GLchar* log_str;
    int processed = 0;
    if (log_len > 0) {
        log_str = (GLchar *) calloc(log_len, sizeof(GLchar));
        glGetProgramInfoLog(program->handle, log_len, NULL, log_str);
        for (int i = 0; i < log_len; i++) {
            if (i == log_len-1 || log_str[i] == '\n') {
                compute_lib_gl_callback(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, i - processed, &(log_str[processed]), program->lib_inst);
                processed = ++i;
            }
        }
        free(log_str);
    }
    return compute_lib_gl_errors_count();
}


/// Pushes lines from OpenGL shader log to the library instance's error queue.
/// \param program Pointer to the GLES3ComputeLib program instance.
/// \return Number of captured OpenGL errors.
static GLuint compute_lib_program_shader_log_to_queue(compute_lib_program_t* program)
{
    if (program->shader_handle == 0) return 0;
    GLint log_len = 0;
    glGetShaderiv(program->shader_handle, GL_INFO_LOG_LENGTH, &log_len);
    GLchar* log_str;
    int processed = 0;
    if (log_len > 0) {
        log_str = (GLchar *) calloc(log_len, sizeof(GLchar));
        glGetShaderInfoLog(program->shader_handle, log_len, NULL, log_str);
        for (int i = 0; i < log_len; i++) {
            if (i == log_len-1 || log_str[i] == '\n') {
                compute_lib_gl_callback(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, i - processed, &(log_str[processed]), program->lib_inst);
                processed = ++i;
            }
        }
        free(log_str);
    }
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_init(compute_lib_instance_t* inst)
{
    if (inst->initialised == GL_TRUE) {
        return COMPUTE_LIB_ERROR_ALREADY_INITIALISED;
    }

    inst->fd = open(inst->dri_path, O_RDWR);
    if (inst->fd <= 0) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_GPU_DRI_PATH;
    }

    inst->gbm = gbm_create_device(inst->fd);
    if (inst->gbm == NULL) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_CREATE_GBM_CTX;
    }

    inst->dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, inst->gbm, NULL);
    if (inst->dpy == EGL_NO_DISPLAY) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_PLATFORM_DISPLAY;
    }

    if (!eglInitialize(inst->dpy, NULL, NULL)) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_INIT;
    }

    const GLchar* egl_extension_st = eglQueryString(inst->dpy, EGL_EXTENSIONS);
    if (strstr(egl_extension_st, "EGL_KHR_create_context") == NULL) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_EXTENSION_CREATE_CTX;
    }
    if (strstr(egl_extension_st, "EGL_KHR_surfaceless_context") == NULL) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_EXTENSION_KHR_CTX;
    }

    EGLConfig egl_cfg;
    EGLint egl_count;
    if (!eglChooseConfig(inst->dpy, egl_config_attribs, &egl_cfg, 1, &egl_count)) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_CONFIG;
    }
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_BIND_API;
    }

    inst->ctx = eglCreateContext(inst->dpy, egl_cfg, EGL_NO_CONTEXT, egl_ctx_attribs);
    if (inst->ctx == EGL_NO_CONTEXT) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_CREATE_CTX;
    }

    if (!eglMakeCurrent(inst->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, inst->ctx)) {
        compute_lib_deinit(inst);
        return COMPUTE_LIB_ERROR_EGL_MAKE_CURRENT;
    }

    inst->last_error = 0;
    inst->error_total_cnt = 0;
    inst->error_queue = queue_create(64);

#ifdef GL_ES_VERSION_3_2
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback((GLDEBUGPROC) compute_lib_gl_callback, (const void*) inst);
#endif

    inst->initialised = GL_TRUE;

    return COMPUTE_LIB_ERROR_NO_ERROR;
}


void compute_lib_deinit(compute_lib_instance_t* inst)
{
    if (inst->ctx != EGL_NO_CONTEXT && inst->dpy != NULL) {
        eglDestroyContext(inst->dpy, inst->ctx);
    }
    inst->ctx = EGL_NO_CONTEXT;

    if (inst->dpy != EGL_NO_DISPLAY) {
        eglTerminate(inst->dpy);
    }
    inst->dpy = NULL;

    if (inst->gbm != NULL) {
        gbm_device_destroy(inst->gbm);
    }
    inst->gbm = NULL;

    if (inst->fd > 0) {
        close(inst->fd);
    }
    inst->fd = 0;

    if (inst->error_queue != NULL) {
        compute_lib_error_queue_flush(inst, stderr);
        queue_delete(inst->error_queue);
    }

    inst->initialised = GL_FALSE;
}

GLuint compute_lib_error_queue_flush(compute_lib_instance_t* inst, FILE* out)
{
    if (inst->error_queue == NULL) {
        return 0;
    }
    GLuint i = 0;
    compute_lib_error_t* err;
    while (inst->error_queue->size > 0) {
        err = (compute_lib_error_t*) queue_pop(inst->error_queue);
        if (out != NULL) {
            fprintf(out, "gpu: GL error #%d: %s (0x%X), severity: %s (0x%X), message = %.*s\n", err->err_id, gl3_get_define_name(err->type), err->type, gl3_get_define_name(err->severity), err->severity, err->message_len, err->message);
        }
        free(err->message);
        free(err);
        i++;
    }
    return i;
}

void compute_lib_print_error(GLint err_code, FILE* out)
{
    switch (err_code) {
        case COMPUTE_LIB_ERROR_NO_ERROR:
            fprintf(out, "compute_lib_init error: no error.\r\n");
            break;
        case COMPUTE_LIB_ERROR_GROUP_INIT_FN:
            fprintf(out, "gpu error: occurs at: GLint compute_lib_init(compute_lib_instance_t* inst);\r\n");
            break;
        case COMPUTE_LIB_ERROR_ALREADY_INITIALISED:
            fprintf(out, "compute_lib_init error: already initialised!\r\n");
            break;
        case COMPUTE_LIB_ERROR_GPU_DRI_PATH:
            fprintf(out, "compute_lib_init error: could not open GPU direct rendering infrastructure!\r\n");
            break;
        case COMPUTE_LIB_ERROR_CREATE_GBM_CTX:
            fprintf(out, "compute_lib_init: error: could not create GBM context!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_PLATFORM_DISPLAY:
            fprintf(out, "compute_lib_init error: could not get platform display!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_INIT:
            fprintf(out, "compute_lib_init error: could not initialise EGL!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_EXTENSION_CREATE_CTX:
            fprintf(out, "compute_lib_init error: could not locate extension: EGL_KHR_create_context!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_EXTENSION_KHR_CTX:
            fprintf(out, "compute_lib_init error: could not locate extension: EGL_KHR_surfaceless_context!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_CONFIG:
            fprintf(out, "compute_lib_init error: could not choose EGL configuration!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_BIND_API:
            fprintf(out, "compute_lib_init error: could not bind EGL_OPENGL_ES_API!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_CREATE_CTX:
            fprintf(out, "compute_lib_init error: could not create EGL context!\r\n");
            break;
        case COMPUTE_LIB_ERROR_EGL_MAKE_CURRENT:
            fprintf(out, "compute_lib_init error: could not make current EGL context!\r\n");
            break;
        case COMPUTE_LIB_ERROR_GROUP_GL_ERROR:
            fprintf(out, "gpu error: occured at GL library, see inst->error_queue!\r\n");
            break;
        default:
            fprintf(out, "gpu error: undefined error (%d)!\r\n", err_code);
            break;
    }
}

GLuint compute_lib_gl_errors_count(void)
{
    GLuint cnt = 0;
    while (glGetError() != GL_NO_ERROR) {
        cnt++;
    }
    return cnt;
}

GLuint compute_lib_program_init(compute_lib_program_t* program)
{
    GLuint errors_cnt;

    program->shader_handle = glCreateShader(GL_COMPUTE_SHADER);
    if ((errors_cnt = glGetError()) != GL_NO_ERROR) {
        goto process_errors;
    }

    glShaderSource(program->shader_handle, 1, (const GLchar* const *) &(program->source), NULL);
    if ((errors_cnt = glGetError()) != GL_NO_ERROR) {
        goto process_errors;
    }

    GLsizei len;
    GLint is_compiled;
    glCompileShader(program->shader_handle);
    glGetShaderiv(program->shader_handle, GL_COMPILE_STATUS, &is_compiled);
    if ((errors_cnt = glGetError()) != GL_NO_ERROR || is_compiled != GL_TRUE) {
        errors_cnt += !is_compiled;
        goto process_errors;
    }

    program->handle = glCreateProgram();
    if (program->handle == 0) {
        goto process_errors;
    }

    glAttachShader(program->handle, program->shader_handle);
    if ((errors_cnt = glGetError()) != GL_NO_ERROR) {
        goto process_errors;
    }

    GLint is_linked;
    glLinkProgram(program->handle);
    glGetProgramiv(program->handle, GL_LINK_STATUS, &is_linked);
    if ((errors_cnt = glGetError()) != GL_NO_ERROR || is_linked != GL_TRUE) {
        errors_cnt += !is_linked;
        goto process_errors;
    }

process_errors:
    if (errors_cnt != GL_NO_ERROR) {
#ifndef GL_ES_VERSION_3_2
        compute_lib_program_log_to_queue(program);
        compute_lib_program_shader_log_to_queue(program);
#endif
        return compute_lib_program_destroy(program, GL_FALSE) + errors_cnt;
    }

    return GL_NO_ERROR;
}

GLuint compute_lib_program_dispatch(compute_lib_program_t* program, GLuint size_x, GLuint size_y, GLuint size_z)
{
    glUseProgram(program->handle);
    glDispatchCompute(size_x / program->local_size_x, size_y / program->local_size_y, size_z / program->local_size_z);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_program_print_resources(compute_lib_program_t* program, FILE* out)
{
    GLint i;
    GLint count;

    GLint size;
    GLenum type;
    GLint name_max_len = 0;
    GLsizei name_len;
    GLuint index;
    GLint location;

    GLenum binding_prop = GL_BUFFER_BINDING, location_prop = GL_LOCATION;
    GLuint binding;

    // get maximum name length
    glGetProgramiv(program->handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_len);
    if (name_len > name_max_len) name_max_len = name_len;

    glGetProgramiv(program->handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_len);
    if (name_len > name_max_len) name_max_len = name_len;

    glGetProgramInterfaceiv(program->handle, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &name_len);
    if (name_len > name_max_len) name_max_len = name_len;

    glGetProgramInterfaceiv(program->handle, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &name_len);
    if (name_len > name_max_len) name_max_len = name_len;

    GLchar name[name_max_len];

    // get active attributes
    glGetProgramiv(program->handle, GL_ACTIVE_ATTRIBUTES, &count);
    fprintf(out, "Active Attributes: %d\n", count);

    for (i = 0; i < count; i++) {
        glGetActiveAttrib(program->handle, (GLuint)i, name_max_len, &name_len, &size, &type, name);
        fprintf(out, "Attribute #%d Type: %s (0x%04X) Name: %s\n", i, gl3_get_define_name(type), type, name);
    }

    // get active uniforms
    glGetProgramiv(program->handle, GL_ACTIVE_UNIFORMS, &count);
    fprintf(out, "Active Uniforms: %d\n", count);

    for (i = 0; i < count; i++) {
        glGetActiveUniform(program->handle, (GLuint)i, name_max_len, &name_len, &size, &type, name);
        index = glGetProgramResourceIndex(program->handle, GL_UNIFORM, name);
        location = glGetUniformLocation(program->handle, name);
        if (type == GL_UNSIGNED_INT_ATOMIC_COUNTER) {
            location_prop = GL_ATOMIC_COUNTER_BUFFER_INDEX;
            glGetProgramResourceiv(program->handle, GL_UNIFORM, index, 1, &location_prop, sizeof(location), NULL, &location);
        }
        fprintf(out, "Uniform #%d Type: %s (0x%04X) Name: %s Index: %d Location: %d Size: %d\n", i, gl3_get_define_name(type), type, name, index, location, size);
    }

    // get active SSBOs
    glGetProgramInterfaceiv(program->handle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &count);
    fprintf(out, "Active SSBOs: %d\n", count);

    for (i = 0; i < count; i++) {
        glGetProgramResourceName(program->handle, GL_SHADER_STORAGE_BLOCK, i, name_max_len, &name_len, name);
        index = glGetProgramResourceIndex(program->handle, GL_SHADER_STORAGE_BLOCK, name);
        glGetProgramResourceiv(program->handle, GL_SHADER_STORAGE_BLOCK, index, 1, &binding_prop, sizeof(binding), NULL, &binding);
        fprintf(out, "SSBO #%d: Index: %d Name: %.*s Binding: %d\r\n", i, index, name_len, name, binding);
    }

    return compute_lib_gl_errors_count();
}

GLchar* compute_lib_program_glsl_layout(compute_lib_program_t* program)
{
    char* str;
    int res = asprintf(&str, "layout (local_size_x = %d, local_size_y = %d, local_size_z = %d) in", program->local_size_x, program->local_size_y, program->local_size_z);
    if (res < 0) return NULL;
    return str;
}

GLuint compute_lib_program_destroy(compute_lib_program_t* program, GLboolean free_source)
{
    if (free_source) {
        free(program->source);
    }
    if (program->shader_handle != 0) {
        glDeleteShader(program->shader_handle);
    }
    program->shader_handle = 0;
    if (program->handle != 0) {
        glDeleteProgram(program->handle);
    }
    program->handle = 0;
    return compute_lib_gl_errors_count();
}


GLuint compute_lib_resource_find(compute_lib_program_t* program, compute_lib_resource_t* resource)
{
    if (resource->value >= 0) {
        return 0;
    }
    GLenum binding_prop = GL_BUFFER_BINDING;
    GLuint index;
    switch (resource->type) {
        case GL_IMAGE_2D:
            resource->value = glGetUniformLocation(program->handle, resource->name);
            break;
        case GL_ATOMIC_COUNTER_BUFFER:
            resource->value = (GLint) glGetProgramResourceIndex(program->handle, GL_UNIFORM, resource->name);
            break;
        case GL_SHADER_STORAGE_BUFFER:
            index = glGetProgramResourceIndex(program->handle, GL_SHADER_STORAGE_BLOCK, resource->name);
            glGetProgramResourceiv(program->handle, GL_SHADER_STORAGE_BLOCK, index, 1, &binding_prop, sizeof(GLint), NULL, &(resource->value));
            break;
        default:
            return (GLuint) -1;
    }
    return compute_lib_gl_errors_count();
}



GLuint compute_lib_framebuffer_init(compute_lib_framebuffer_t* framebuffer)
{
    glGenFramebuffers(1, &(framebuffer->handle));
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_framebuffer_destroy(compute_lib_framebuffer_t* framebuffer)
{
    if (framebuffer->handle != 0) {
        glDeleteFramebuffers(1, &(framebuffer->handle));
    }
    return compute_lib_gl_errors_count();
}

void compute_lib_image2d_setup_format(compute_lib_image2d_t* image2d)
{
    switch (image2d->type) {
        case GL_UNSIGNED_BYTE:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R8UI;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG8UI;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB8UI;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA8UI;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_BYTE:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R8I;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG8I;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB8I;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA8I;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_UNSIGNED_SHORT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R16UI;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG16UI;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB16UI;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA16UI;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_SHORT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R16I;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG16I;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB16I;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA16I;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_UNSIGNED_INT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R32UI;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG32UI;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB32UI;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA32UI;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_INT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R32I;
                    image2d->format = GL_RED_INTEGER;
                    break;
                case 2:
                    image2d->internal_format = GL_RG32I;
                    image2d->format = GL_RG_INTEGER;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB32I;
                    image2d->format = GL_RGB_INTEGER;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA32I;
                    image2d->format = GL_RGBA_INTEGER;
                    break;
            }
            break;
        case GL_HALF_FLOAT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R16F;
                    image2d->format = GL_RED;
                    break;
                case 2:
                    image2d->internal_format = GL_RG16F;
                    image2d->format = GL_RG;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB16F;
                    image2d->format = GL_RGB;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA16F;
                    image2d->format = GL_RGBA;
                    break;
            }
            break;
        case GL_FLOAT:
            switch (image2d->num_components) {
                case 1:
                    image2d->internal_format = GL_R32F;
                    image2d->format = GL_RED;
                    break;
                case 2:
                    image2d->internal_format = GL_RG32F;
                    image2d->format = GL_RG;
                    break;
                case 3:
                    image2d->internal_format = GL_RGB32F;
                    image2d->format = GL_RGB;
                    break;
                case 4:
                    image2d->internal_format = GL_RGBA32F;
                    image2d->format = GL_RGBA;
                    break;
            }
            break;
    }
    image2d->compatibility_format = gl3_get_image2d_compatibility_format(image2d->internal_format);
}

GLuint compute_lib_image2d_init(compute_lib_image2d_t* image2d, GLenum framebuffer_attachment)
{
    glGenTextures(1, &(image2d->handle));
    glActiveTexture(image2d->texture);
    glBindTexture(GL_TEXTURE_2D, image2d->handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, image2d->texture_wrap);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, image2d->texture_wrap);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, image2d->texture_filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, image2d->texture_filter);
    glTexStorage2D(GL_TEXTURE_2D, 1, image2d->internal_format, image2d->width, image2d->height);
    glBindImageTexture(image2d->resource.value, image2d->handle, 0, GL_FALSE, 0, image2d->access, image2d->compatibility_format);
    size_t type_size = gl3_get_type_size(image2d->type);
    image2d->px_size = type_size * image2d->num_components;
    image2d->data_size = image2d->px_size * image2d->width * image2d->height;
    if (framebuffer_attachment != 0) {
        image2d->framebuffer.attachment = framebuffer_attachment;
        compute_lib_framebuffer_init(&(image2d->framebuffer));
        glBindFramebuffer(GL_FRAMEBUFFER, image2d->framebuffer.handle);
        glFramebufferTexture2D(GL_FRAMEBUFFER, framebuffer_attachment, GL_TEXTURE_2D, image2d->handle, 0);
    }
    return compute_lib_gl_errors_count();
}

GLchar* compute_lib_image2d_glsl_layout(compute_lib_image2d_t* image2d)
{
    char* str;
    int res = asprintf(&str, "layout(%s, binding=%d) %s uniform highp %s %s", gl3_get_glsl_image2d_format_qualifier(image2d->compatibility_format), image2d->resource.value, gl3_get_glsl_image2d_access(image2d->access), gl3_get_glsl_image2d_type(image2d->compatibility_format), image2d->resource.name);
    if (res < 0) return NULL;
    return str;
}

GLuint compute_lib_image2d_destroy(compute_lib_image2d_t* image2d)
{
    compute_lib_framebuffer_destroy(&(image2d->framebuffer));
    glDeleteTextures(1, &(image2d->handle));
    return compute_lib_gl_errors_count();
}

/// Allocates memory to store 2D image data (based on the image format and dimensions).
/// \param image2d Pointer to the GLES3ComputeLib 2D image instance.
/// \return Pointer to the allocated memory.
static inline void* compute_lib_image2d_alloc(compute_lib_image2d_t* image2d)
{
    return malloc(image2d->data_size);
}

GLuint compute_lib_image2d_reset(compute_lib_image2d_t* image2d, void* px_data)
{
    GLint i;
    void* image_data = compute_lib_image2d_alloc(image2d);
    for (i = 0; i < image2d->data_size; i += image2d->px_size) {
        memcpy(image_data + i, px_data, image2d->px_size);
    }
    glBindTexture(GL_TEXTURE_2D, image2d->handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image2d->width, image2d->height, image2d->format, image2d->type, image_data);
    free(image_data);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_image2d_reset_patch(compute_lib_image2d_t* image2d, void* px_data, GLint x_min, GLint x_max, GLint y_min, GLint y_max)
{
    GLint patch_width = x_max - x_min;
    GLint patch_height = y_max - y_min;
    GLint x, y;
    void* image_data = compute_lib_image2d_alloc(image2d);
    for (y = 0; y < patch_height; y++) {
        for (x = 0; x < patch_width; x++) {
            memcpy(image_data + (image2d->px_size * ((y_min + y) * image2d->width + x_min)) + x*image2d->px_size, px_data, image2d->px_size);
        }
    }
    glBindTexture(GL_TEXTURE_2D, image2d->handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x_min, y_min, patch_width, patch_height, image2d->format, image2d->type, image_data);
    free(image_data);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_image2d_write(compute_lib_image2d_t* image2d, void* image_data)
{
    glBindTexture(GL_TEXTURE_2D, image2d->handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image2d->width, image2d->height, image2d->format, image2d->type, image_data);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_image2d_read(compute_lib_image2d_t* image2d, void* image_data)
{
    if (image2d->framebuffer.handle != 0) {
        glBindTexture(GL_TEXTURE_2D, image2d->handle);
        glBindFramebuffer(GL_FRAMEBUFFER, image2d->framebuffer.handle);
        glFramebufferTexture2D(GL_FRAMEBUFFER, image2d->framebuffer.attachment, GL_TEXTURE_2D, image2d->handle, 0);
        glReadPixels(0, 0, image2d->width, image2d->height, image2d->format, image2d->type, image_data);
    }
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_image2d_read_patch(compute_lib_image2d_t* image2d, void* image_data, GLint x_min, GLint x_max, GLint y_min, GLint y_max, GLboolean render)
{
    if (image2d->framebuffer.handle != 0) {
        glBindTexture(GL_TEXTURE_2D, image2d->handle);
        if (render) {
            glBindFramebuffer(GL_FRAMEBUFFER, image2d->framebuffer.handle);
            glFramebufferTexture2D(GL_FRAMEBUFFER, image2d->framebuffer.attachment, GL_TEXTURE_2D, image2d->handle, 0);
        }
        GLint patch_width = x_max - x_min;
        GLint patch_height = y_max - y_min;
        GLint y;
        void* tmp = malloc(image2d->px_size * patch_width * patch_height);
        glReadPixels(x_min, y_min, patch_width, patch_height, image2d->format, image2d->type, tmp);
        for (y = 0; y < patch_height; y++) {
            memcpy(image_data + (image2d->px_size * ((y_min + y) * image2d->width + x_min)), tmp + (image2d->px_size * (y * patch_width)), image2d->px_size * patch_width);
        }
        free(tmp);
    }
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_init(compute_lib_acbo_t* acbo, void* data, GLint len)
{
    glGenBuffers(1, &(acbo->handle));
    if (len > 0) compute_lib_acbo_write(acbo, data, len);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_destroy(compute_lib_acbo_t* acbo)
{
    glDeleteBuffers(1, &(acbo->handle));
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_write(compute_lib_acbo_t* acbo, void* data, GLint len)
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo->handle);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, gl3_get_type_size(acbo->type)*len, data, acbo->usage);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, acbo->resource.value, acbo->handle);
    //glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, gl3_get_type_size(acbo->type)*len, data);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_write_uint_val(compute_lib_acbo_t* acbo, GLuint value)
{
    compute_lib_acbo_write(acbo, (uint8_t*) &value, 1);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_read(compute_lib_acbo_t* acbo, void* data, GLint len)
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo->handle);
    GLsizei size = gl3_get_type_size(acbo->type)*len;
    memcpy(data, glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, size, GL_MAP_READ_BIT), size);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_acbo_read_uint_val(compute_lib_acbo_t* acbo, GLuint* value)
{
    compute_lib_acbo_read(acbo, (void*) value, 1);
    return compute_lib_gl_errors_count();
}


GLuint compute_lib_ssbo_init(compute_lib_ssbo_t* ssbo, void* data, GLint len)
{
    glGenBuffers(1, &(ssbo->handle));
    if (len > 0) compute_lib_ssbo_write(ssbo, data, len);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_ssbo_destroy(compute_lib_ssbo_t* ssbo)
{
    glDeleteBuffers(1, &(ssbo->handle));
    return compute_lib_gl_errors_count();
}

GLchar* compute_lib_ssbo_glsl_layout(compute_lib_ssbo_t* ssbo)
{
    char* str;
    int res = asprintf(&str, "layout(std430, binding=%d) buffer %s { %s %s_data[]; }", ssbo->resource.value, ssbo->resource.name, gl3_get_glsl_data_type(ssbo->type), ssbo->resource.name);
    if (res < 0) return NULL;
    return str;
}

GLuint compute_lib_ssbo_write(compute_lib_ssbo_t* ssbo, void* data, GLint len)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo->handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gl3_get_type_size(ssbo->type)*len, data, ssbo->usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo->resource.value, ssbo->handle);
    //glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, gl3_get_type_size(ssbo->type)*len, data);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_ssbo_read(compute_lib_ssbo_t* ssbo, void* data, GLint len)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo->handle);
    GLint size = gl3_get_type_size(ssbo->type)*len;
    memcpy(data, glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, GL_MAP_READ_BIT), size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_uniform_init(compute_lib_program_t* program, compute_lib_uniform_t* uniform)
{
    glGetUniformIndices(program->handle, 1, (const GLchar **) &(uniform->name), &(uniform->index));
    uniform->location = glGetUniformLocation(program->handle, uniform->name);
    glGetActiveUniform(program->handle, uniform->index, 0, NULL, &(uniform->size), &(uniform->type), NULL);
    return compute_lib_gl_errors_count();
}

GLuint compute_lib_uniform_write(compute_lib_program_t* program, compute_lib_uniform_t* uniform, void* data)
{
    glUseProgram(program->handle);
    switch (uniform->type) {
        case GL_FLOAT:
            glUniform1fv(uniform->location, uniform->size, (const GLfloat*) data);
            break;
        case GL_UNSIGNED_INT:
            glUniform1uiv(uniform->location, uniform->size, (const GLuint*) data);
            break;
        case GL_INT:
            glUniform1iv(uniform->location, uniform->size, (const GLint*) data);
            break;
        case GL_FLOAT_VEC2:
            glUniform2fv(uniform->location, uniform->size, (const GLfloat*) data);
            break;
        case GL_UNSIGNED_INT_VEC2:
            glUniform2uiv(uniform->location, uniform->size, (const GLuint*) data);
            break;
        case GL_INT_VEC2:
            glUniform2iv(uniform->location, uniform->size, (const GLint*) data);
            break;
        case GL_FLOAT_VEC3:
            glUniform3fv(uniform->location, uniform->size, (const GLfloat*) data);
            break;
        case GL_UNSIGNED_INT_VEC3:
            glUniform3uiv(uniform->location, uniform->size, (const GLuint*) data);
            break;
        case GL_INT_VEC3:
            glUniform3iv(uniform->location, uniform->size, (const GLint*) data);
            break;
        case GL_FLOAT_VEC4:
            glUniform4fv(uniform->location, uniform->size, (const GLfloat*) data);
            break;
        case GL_UNSIGNED_INT_VEC4:
            glUniform4uiv(uniform->location, uniform->size, (const GLuint*) data);
            break;
        case GL_INT_VEC4:
            glUniform4iv(uniform->location, uniform->size, (const GLint*) data);
            break;
        case GL_FLOAT_MAT2:
            glUniformMatrix2fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT3:
            glUniformMatrix3fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT4:
            glUniformMatrix4fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT2x3:
            glUniformMatrix2x3fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT3x2:
            glUniformMatrix3x2fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT2x4:
            glUniformMatrix2x4fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT4x2:
            glUniformMatrix4x2fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT3x4:
            glUniformMatrix3x4fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
        case GL_FLOAT_MAT4x3:
            glUniformMatrix4x3fv(uniform->location, uniform->size, GL_FALSE, (const GLfloat*) data);
            break;
    }
    glUseProgram(0);
    return compute_lib_gl_errors_count();
}



