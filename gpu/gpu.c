//
// Created by vrbav on 11/19/23.
//

#include "gpu.h"

static const char* render_devices = RENDER_DEVICES "\0";

static mdgpu_t* mdgpu_init(compute_lib_instance_t* inst, uint32_t image_width, uint32_t image_height, uint32_t max_markers_count, uint32_t max_sun_pts_count, uint32_t threshold, uint32_t threshold_diff, uint32_t threshold_sun);
static void mdgpu_destroy(mdgpu_t* mdgpu);

static compute_lib_instance_t inst;

static unsigned int max_invocations;
static unsigned int max_local_size_x, max_local_size_y, max_local_size_z;
static unsigned int local_size_x = 1, local_size_y = 1;


mdgpu_t* gpu_init(int image_width, int image_height, int threshold, int threshold_diff, int threshold_sun)
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
    mdgpu_t* mdgpu;
    if ((mdgpu = mdgpu_init(&inst, image_width, image_height, MAX_MARKERS_COUNT, MAX_SUN_PTS_COUNT, threshold, threshold_diff, threshold_sun)) == NULL) {
        fprintf(stderr, "GPU: Failed to initialize marker detection for OpenGLES!\r\n");
        compute_lib_error_queue_flush(&inst, stderr);
        compute_lib_deinit(&inst);
        return NULL;
    }

    return mdgpu;
}


uint64_t gpu_process_image(mdgpu_t* self, uint8_t* image, int* markers_count, md_pt_t* markers, int* sun_pts_count, md_pt_t* sun_pts) 
{
    int size = self->image_in.width * self->image_in.height;
    uint8_t *rgba8ui = calloc(4 * size, sizeof(uint8_t));
    for (int i = 0; i < size; i++) { rgba8ui[4 * i] = image[i]; }

    // reset atomic counter buffer objects
    compute_lib_acbo_write_uint_val(&self->markers_count_acbo, 0);
    compute_lib_acbo_write_uint_val(&self->sun_pts_count_acbo, 0);

    // write input image data
    compute_lib_image2d_write(&self->image_in, rgba8ui);

    // dispatch compute shader
    //uint64_t start = rdtsc();
    compute_lib_program_dispatch(&self->compute_prog, self->image_in.width, self->image_in.height, 1);
    //uint64_t stop = rdtsc();

    compute_lib_acbo_read_uint_val(&self->markers_count_acbo, (GLuint*) markers_count);
    compute_lib_acbo_read_uint_val(&self->sun_pts_count_acbo, (GLuint*) sun_pts_count);

    // retrieve detected markers
    if (*markers_count > MAX_MARKERS_COUNT) *markers_count = MAX_MARKERS_COUNT;
    if (*markers_count > 0) compute_lib_ssbo_read(&self->markers_ssbo, (void *) markers, *markers_count);

    // retrieve detected sun points
    if (*sun_pts_count > MAX_SUN_PTS_COUNT) *sun_pts_count = MAX_SUN_PTS_COUNT;
    if (*sun_pts_count > 0) compute_lib_ssbo_read(&self->sun_pts_ssbo, (void *) sun_pts, *sun_pts_count);

    free(rgba8ui);

    return 0;
}

void gpu_destroy(mdgpu_t* self)
{
    mdgpu_destroy(self);
    compute_lib_deinit(&inst);
}

static void mdgpu_destroy(mdgpu_t* mdgpu)
{
    compute_lib_image2d_destroy(&(mdgpu->image_in));
    //compute_lib_image2d_destroy(&(mdgpu->mask));
    compute_lib_acbo_destroy(&(mdgpu->markers_count_acbo));
    compute_lib_acbo_destroy(&(mdgpu->sun_pts_count_acbo));
    compute_lib_ssbo_destroy(&(mdgpu->markers_ssbo));
    compute_lib_ssbo_destroy(&(mdgpu->sun_pts_ssbo));
    compute_lib_program_destroy(&(mdgpu->compute_prog), GL_TRUE);
    free(mdgpu);
}

static mdgpu_t* mdgpu_init(compute_lib_instance_t* inst, uint32_t image_width, uint32_t image_height, uint32_t max_markers_count, uint32_t max_sun_pts_count, uint32_t threshold, uint32_t threshold_diff, uint32_t threshold_sun)
{
    mdgpu_t* mdgpu = (mdgpu_t*) malloc(sizeof(mdgpu_t));
    //compute_lib_image2d_write(&self->mask, NULL);

    mdgpu->image_in = COMPUTE_LIB_IMAGE2D_NEW("image_in", GL_TEXTURE0, image_width, image_height, GL_READ_ONLY, 4, GL_UNSIGNED_BYTE);
    mdgpu->image_in.resource.value = 0;
    compute_lib_image2d_setup_format(&(mdgpu->image_in));

    //mdgpu->mask = COMPUTE_LIB_IMAGE2D_NEW("mask", GL_TEXTURE1, image_width, image_height, GL_READ_ONLY, 1, GL_UNSIGNED_BYTE);
    //mdgpu->mask.resource.value = 1;
    //compute_lib_image2d_setup_format(&(mdgpu->mask));

    mdgpu->sun_pts_count_acbo = COMPUTE_LIB_ACBO_NEW("sun_pts_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    mdgpu->sun_pts_count_acbo.resource.value = 2;

    mdgpu->markers_count_acbo = COMPUTE_LIB_ACBO_NEW("markers_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    mdgpu->markers_count_acbo.resource.value = 3;

    mdgpu->markers_ssbo = COMPUTE_LIB_SSBO_NEW("markers_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    mdgpu->markers_ssbo.resource.value = 4;

    mdgpu->sun_pts_ssbo = COMPUTE_LIB_SSBO_NEW("sun_pts_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    mdgpu->sun_pts_ssbo.resource.value = 5;

    mdgpu->compute_prog = COMPUTE_LIB_PROGRAM_NEW(inst, NULL, local_size_x, local_size_y, 1);

    if(asprintf(&(mdgpu->compute_prog.source), _binary_shader_comp_start, threshold, threshold_diff, threshold_sun, max_markers_count, max_sun_pts_count, local_size_x, local_size_y) < 0) {
        fprintf(stderr, "Failed to format shader source!\r\n");
        return NULL;
    }

    if (compute_lib_program_init(&(mdgpu->compute_prog)) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to create program!\r\n");
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    if (compute_lib_image2d_init(&(mdgpu->image_in), 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init image2d '%s'!\r\n", mdgpu->image_in.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    /*if (compute_lib_image2d_init(&(mdgpu->mask), 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init image2d '%s'!\r\n", mdgpu->mask.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }*/

    if (compute_lib_acbo_init(&(mdgpu->sun_pts_count_acbo), NULL, 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init acbo '%s'!\r\n", mdgpu->sun_pts_count_acbo.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    if (compute_lib_acbo_init(&(mdgpu->markers_count_acbo), NULL, 0) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init acbo '%s'!\r\n", mdgpu->markers_count_acbo.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    if (compute_lib_ssbo_init(&(mdgpu->markers_ssbo), NULL, max_markers_count) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init ssbo '%s'!\r\n", mdgpu->markers_ssbo.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    if (compute_lib_ssbo_init(&(mdgpu->sun_pts_ssbo), NULL, max_sun_pts_count) != GL_NO_ERROR) {
        fprintf(stderr, "Failed to init ssbo '%s'!\r\n", mdgpu->sun_pts_ssbo.resource.name);
        mdgpu_destroy(mdgpu);
        return NULL;
    }

    return mdgpu;
}
