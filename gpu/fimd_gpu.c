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
        fprintf(stderr, "ERROR: Failed to initialize compute_lib instance!\r\n");
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
        fprintf(stderr, "ERROR: Failed to initialize marker detection for OpenGLES!\r\n");
        compute_lib_error_queue_flush(&inst, stderr);
        compute_lib_deinit(&inst);
        return NULL;
    }

    return fimd_gpu_inst;
}

int compare_markers_xy1d(const void* a, const void* b) {
    uint16_t* pt1 = (uint16_t*) a;
    uint16_t* pt2 = (uint16_t*) b;
    int res = (int) pt1[0] - (int) pt2[0];
    if (res == 0) res = (int) pt1[1] - (int) pt2[1];
    return res;
}

static unsigned fimd_gpu_get_marker_centroids(uint16_t markers_raw[][2], uint32_t init_cnt, uint32_t distance_px, unsigned markers[][2]) {
    ivec3_t filtered_markers[init_cnt];
    uint32_t filtered_cnt = 0;
    uint32_t max_dist2 = (distance_px * distance_px);
    uint32_t min_dist2, dist2;
    int32_t closest_marker;
    uint32_t i, j;
    int32_t x2, y2, n;
    int32_t pt_raw[2];

    qsort(markers_raw, init_cnt, sizeof(markers_raw[0]), compare_markers_xy1d);

    for (i = 0; i < init_cnt; i++) {
        pt_raw[0] = markers_raw[i][1];
        pt_raw[1] = markers_raw[i][0];

        min_dist2 = max_dist2;
        closest_marker = -1;

        for (j = 0; j < filtered_cnt; j++) {
            n = filtered_markers[j].z;
            x2 = filtered_markers[j].x / n;
            y2 = filtered_markers[j].y / n;
            dist2 = (x2 - pt_raw[0])*(x2 - pt_raw[0]) + (y2 - pt_raw[1])*(y2 - pt_raw[1]);
            if (dist2 < min_dist2) {
                min_dist2 = dist2;
                closest_marker = j;
            }
        }

        if (closest_marker != -1) {
            filtered_markers[closest_marker].x += pt_raw[0];
            filtered_markers[closest_marker].y += pt_raw[1];
            filtered_markers[closest_marker].z += 1;
        } else {
            filtered_markers[filtered_cnt].x = pt_raw[0];
            filtered_markers[filtered_cnt].y = pt_raw[1];
            filtered_markers[filtered_cnt].z = 1;
            filtered_cnt++;
        }
    }

    for (i = 0; i < filtered_cnt; i++) {
        markers[i][0] = filtered_markers[i].x / filtered_markers[i].z;
        markers[i][1] = filtered_markers[i].y / filtered_markers[i].z;
    }

    return filtered_cnt;
}

unsigned fimd_gpu_detect(void* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count)
{
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) handle;

    int size = fimd_gpu_inst->image_in.width * fimd_gpu_inst->image_in.height;
    uint8_t *rgba8ui = calloc(4 * size, sizeof(uint8_t));
    for (int i = 0; i < size; i++) { rgba8ui[4 * i] = image[i]; }

    unsigned error = 0;
    uint16_t markers_raw[fimd_gpu_inst->max_markers_count][2];
    uint16_t sun_pts_raw[fimd_gpu_inst->max_sun_pts_count][2];

    // reset atomic counter buffer objects
    error = compute_lib_acbo_write_uint_val(&fimd_gpu_inst->markers_count_acbo, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to reset markers count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }

    error = compute_lib_acbo_write_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to reset sun points count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }


    // write input image data
    error = compute_lib_image2d_write(&fimd_gpu_inst->image_in, rgba8ui);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to write image data to image2d! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }

    // dispatch compute shader
    error = compute_lib_program_dispatch(&fimd_gpu_inst->compute_prog, fimd_gpu_inst->image_in.width, fimd_gpu_inst->image_in.height, 1);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to dispatch compute shader! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }

    error = compute_lib_acbo_read_uint_val(&fimd_gpu_inst->markers_count_acbo, (GLuint*) markers_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to read markers count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }

    error = compute_lib_acbo_read_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, (GLuint*) sun_pts_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to read sun points count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&inst, stderr);
        goto detect_end;
    }

    // retrieve detected markers
    if (*markers_count > fimd_gpu_inst->max_markers_count) *markers_count = fimd_gpu_inst->max_markers_count;
    if (*markers_count > 0) {
        error = compute_lib_ssbo_read(&fimd_gpu_inst->markers_ssbo, (void *) markers_raw, (GLint) *markers_count);
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "ERROR: Failed to read markers ssbo! Code: %d\r\n", error);
            compute_lib_error_queue_flush(&inst, stderr);
            goto detect_end;
        }
        *markers_count = fimd_gpu_get_marker_centroids(markers_raw, *markers_count, 5, markers);
    }

    // retrieve detected sun points
    if (*sun_pts_count > fimd_gpu_inst->max_sun_pts_count) *sun_pts_count = fimd_gpu_inst->max_sun_pts_count;
    if (*sun_pts_count > 0) {
        error = compute_lib_ssbo_read(&fimd_gpu_inst->sun_pts_ssbo, (void *) sun_pts_raw, (GLint) *sun_pts_count);
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "ERROR: Failed to read sun points ssbo! Code: %d\r\n", error);
            compute_lib_error_queue_flush(&inst, stderr);
            goto detect_end;
        }
        for (int i = 0; i < *sun_pts_count; i++) {
            sun_pts[i][0] = sun_pts_raw[i][1];
            sun_pts[i][1] = sun_pts_raw[i][0];
        }
    }

detect_end:
    free(rgba8ui);
    return error;
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
    unsigned error = 0;
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
        fprintf(stderr, "ERROR: Failed to format shader source!\r\n");
        return NULL;
    }

    error = compute_lib_program_init(&(fimd_gpu_inst->compute_prog));
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to create program!\r\n");
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    error = compute_lib_image2d_init(&(fimd_gpu_inst->image_in), 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init image2d '%s'!\r\n", fimd_gpu_inst->image_in.resource.name);
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    error = compute_lib_acbo_init(&(fimd_gpu_inst->sun_pts_count_acbo), NULL, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init acbo '%s'!\r\n", fimd_gpu_inst->sun_pts_count_acbo.resource.name);
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    error = compute_lib_acbo_init(&(fimd_gpu_inst->markers_count_acbo), NULL, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init acbo '%s'!\r\n", fimd_gpu_inst->markers_count_acbo.resource.name);
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    error = compute_lib_ssbo_init(&(fimd_gpu_inst->markers_ssbo), NULL, (GLint) max_markers_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->markers_ssbo.resource.name);
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    error = compute_lib_ssbo_init(&(fimd_gpu_inst->sun_pts_ssbo), NULL, (GLint) max_sun_pts_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->sun_pts_ssbo.resource.name);
        compute_lib_error_queue_flush(inst, stderr);
        fimd_gpu_inst_destroy(fimd_gpu_inst);
        return NULL;
    }

    return fimd_gpu_inst;
}
