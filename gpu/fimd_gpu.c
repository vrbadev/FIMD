/**
 * \file fimd_gpu.c
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Source file for the FIMD-GPU library.
 * \copyright GNU Public License.
 */

#include <stdio.h>

#include "compute_lib.h"
#include "fimd_gpu.h"

extern char _binary_shader_comp_start[];
static const char* render_devices = RENDER_DEVICES "\0";

static struct fimd_gpu_inst_s* fimd_gpu_inst_init(compute_lib_instance_t* inst, uint32_t image_width, uint32_t image_height, uint32_t max_markers_count, uint32_t max_sun_pts_count, uint32_t threshold, uint32_t threshold_diff, uint32_t threshold_sun);
static void fimd_gpu_inst_destroy(struct fimd_gpu_inst_s* fimd_gpu_inst);

static compute_lib_instance_t inst;

static unsigned int max_invocations;
static unsigned int max_local_size_x, max_local_size_y, max_local_size_z;
static unsigned int local_size_x = 1, local_size_y = 1;


struct fimd_gpu_inst_s {
    compute_lib_program_t compute_prog;
    compute_lib_image2d_t image_in;
    compute_lib_acbo_t markers_count_acbo, sun_pts_count_acbo;
    compute_lib_ssbo_t markers_ssbo, sun_pts_ssbo;
    unsigned max_markers_count;
    unsigned max_sun_pts_count;
};


void* fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count)
{
    //printf("Initializing compute library instance.\r\n");
    char devs[strlen(render_devices)+1];
    strcpy(devs, render_devices);
    char* token = strtok(devs, ",");
    int init_res = -1;
    while (token != NULL) {
        inst = COMPUTE_LIB_INSTANCE_NEW(token);
        init_res = compute_lib_init(&inst);
        if (init_res != COMPUTE_LIB_ERROR_GPU_DRI_PATH) break;
        token = strtok(NULL, ",");
    }

    if (init_res != GL_NO_ERROR) {
        compute_lib_error_queue_flush(&inst, stderr);
        compute_lib_print_error(init_res, stderr);
        fprintf(stderr, "GPU: Failed to initialize compute_lib instance!\r\n");
        return NULL;
    }

    // get the maximum size of the local work group
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (GLint*) &max_invocations);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (GLint*) &max_local_size_x);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (GLint*) &max_local_size_y);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (GLint*) &max_local_size_z);
    //printf("GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: %d\r\n", max_invocations);
    //printf("GL_MAX_COMPUTE_WORK_GROUP_SIZE: x=%d, y=%d, z=%d\r\n", max_local_size_x, max_local_size_y, max_local_size_z);
    
    local_size_x = max_local_size_x;
    while (local_size_x * (image_width / local_size_x) != image_width) {
        local_size_x /= 2;
    }
    local_size_y = max_invocations / local_size_x;
    while (local_size_y * (image_height / local_size_y) != image_height) {
        local_size_y /= 2;
    }

    //printf("Selected local group sizes: x=%d, y=%d\r\n", local_size_x, local_size_y);
    //printf("Number of invocations: nx=%d, ny=%d\r\n", image_width / local_size_x, image_height / local_size_y);

    //printf("Initializing marker detection GPU instance.\r\n");
    struct fimd_gpu_inst_s* fimd_gpu_inst;
    if ((fimd_gpu_inst = fimd_gpu_inst_init(&inst, image_width, image_height, max_markers_count, max_sun_pts_count, threshold, threshold_diff, threshold_sun)) == NULL) {
        fprintf(stderr, "GPU: Failed to initialize marker detection for OpenGLES!\r\n");
        compute_lib_error_queue_flush(&inst, stderr);
        compute_lib_deinit(&inst);
        return NULL;
    }

    return fimd_gpu_inst;
}


int fimd_gpu_detect(void* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count)
{
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) handle;

    int size = fimd_gpu_inst->image_in.width * fimd_gpu_inst->image_in.height;
    uint8_t *rgba8ui = calloc(4 * size, sizeof(uint8_t));
    for (int i = 0; i < size; i++) { rgba8ui[4 * i] = image[i]; }

    // reset atomic counter buffer objects
    compute_lib_acbo_write_uint_val(&fimd_gpu_inst->markers_count_acbo, 0);
    compute_lib_acbo_write_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, 0);

    // write input image data
    compute_lib_image2d_write(&fimd_gpu_inst->image_in, rgba8ui);

    // dispatch compute shader
    compute_lib_program_dispatch(&fimd_gpu_inst->compute_prog, fimd_gpu_inst->image_in.width, fimd_gpu_inst->image_in.height, 1);

    compute_lib_acbo_read_uint_val(&fimd_gpu_inst->markers_count_acbo, (GLuint*) markers_count);
    compute_lib_acbo_read_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, (GLuint*) sun_pts_count);

    // retrieve detected markers
    if (*markers_count > fimd_gpu_inst->max_markers_count) *markers_count = fimd_gpu_inst->max_markers_count;
    if (*markers_count > 0) compute_lib_ssbo_read(&fimd_gpu_inst->markers_ssbo, (void *) markers, *markers_count);

    // retrieve detected sun points
    if (*sun_pts_count > fimd_gpu_inst->max_sun_pts_count) *sun_pts_count = fimd_gpu_inst->max_sun_pts_count;
    if (*sun_pts_count > 0) compute_lib_ssbo_read(&fimd_gpu_inst->sun_pts_ssbo, (void *) sun_pts, *sun_pts_count);

    free(rgba8ui);

    return 0;
}

void fimd_gpu_destroy(void* handle)
{
    fimd_gpu_inst_destroy((struct fimd_gpu_inst_s*) handle);
    compute_lib_deinit(&inst);
}

static void fimd_gpu_inst_destroy(struct fimd_gpu_inst_s* fimd_gpu_inst)
{
    compute_lib_image2d_destroy(&(fimd_gpu_inst->image_in));
    compute_lib_acbo_destroy(&(fimd_gpu_inst->markers_count_acbo));
    compute_lib_acbo_destroy(&(fimd_gpu_inst->sun_pts_count_acbo));
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->markers_ssbo));
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->sun_pts_ssbo));
    compute_lib_program_destroy(&(fimd_gpu_inst->compute_prog), GL_TRUE);
    free(fimd_gpu_inst);
}

static struct fimd_gpu_inst_s* fimd_gpu_inst_init(compute_lib_instance_t* inst, uint32_t image_width, uint32_t image_height, uint32_t max_markers_count, uint32_t max_sun_pts_count, uint32_t threshold, uint32_t threshold_diff, uint32_t threshold_sun)
{
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) malloc(sizeof(struct fimd_gpu_inst_s));

    fimd_gpu_inst->image_in = COMPUTE_LIB_IMAGE2D_NEW("image_in", GL_TEXTURE0, image_width, image_height, GL_READ_ONLY, 4, GL_UNSIGNED_BYTE);
    fimd_gpu_inst->image_in.resource.value = 0;
    compute_lib_image2d_setup_format(&(fimd_gpu_inst->image_in));

    fimd_gpu_inst->max_sun_pts_count = max_sun_pts_count;
    fimd_gpu_inst->sun_pts_count_acbo = COMPUTE_LIB_ACBO_NEW("sun_pts_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->sun_pts_count_acbo.resource.value = 2;

    fimd_gpu_inst->max_markers_count = max_markers_count;
    fimd_gpu_inst->markers_count_acbo = COMPUTE_LIB_ACBO_NEW("markers_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->markers_count_acbo.resource.value = 3;

    fimd_gpu_inst->markers_ssbo = COMPUTE_LIB_SSBO_NEW("markers_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->markers_ssbo.resource.value = 4;

    fimd_gpu_inst->sun_pts_ssbo = COMPUTE_LIB_SSBO_NEW("sun_pts_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->sun_pts_ssbo.resource.value = 5;

    fimd_gpu_inst->compute_prog = COMPUTE_LIB_PROGRAM_NEW(inst, NULL, local_size_x, local_size_y, 1);

    if(asprintf(&(fimd_gpu_inst->compute_prog.source), _binary_shader_comp_start, threshold, threshold_diff, threshold_sun, max_markers_count, max_sun_pts_count, local_size_x, local_size_y) < 0) {
        fprintf(stderr, "Failed to format shader source!\r\n");
        return NULL;
    }

    if (compute_lib_program_init(&(fimd_gpu_inst->compute_prog)) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to create program!\r\n");
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    if (compute_lib_image2d_init(&(fimd_gpu_inst->image_in), 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init image2d '%s'!\r\n", fimd_gpu_inst->image_in.resource.name);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    if (compute_lib_acbo_init(&(fimd_gpu_inst->sun_pts_count_acbo), NULL, 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init acbo '%s'!\r\n", fimd_gpu_inst->sun_pts_count_acbo.resource.name);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    if (compute_lib_acbo_init(&(fimd_gpu_inst->markers_count_acbo), NULL, 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init acbo '%s'!\r\n", fimd_gpu_inst->markers_count_acbo.resource.name);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    if (compute_lib_ssbo_init(&(fimd_gpu_inst->markers_ssbo), NULL, max_markers_count) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->markers_ssbo.resource.name);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    if (compute_lib_ssbo_init(&(fimd_gpu_inst->sun_pts_ssbo), NULL, max_sun_pts_count) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->sun_pts_ssbo.resource.name);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    return fimd_gpu_inst;
}
