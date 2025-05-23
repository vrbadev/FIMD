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

static unsigned int max_invocations;
static unsigned int max_local_size_x, max_local_size_y, max_local_size_z;

struct fimd_gpu_inst_s {
    compute_lib_instance_t compute_lib;
    compute_lib_program_t compute_prog;
    compute_lib_ssbo_t image_in_ssbo;
    compute_lib_acbo_t markers_count_acbo, sun_pts_count_acbo;
    compute_lib_ssbo_t configuration_ssbo, markers_ssbo, sun_pts_ssbo;
    uint32_t local_size_x, local_size_y;
};


fimd_gpu_t* fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count, unsigned radii_count, unsigned* radii)
{
    int compute_lib_error = 0;
    unsigned error = 0;

    fimd_gpu_t* handle = (fimd_gpu_t*) malloc(sizeof(struct fimd_gpu_handle_s));
    if (!handle) {
        fprintf(stderr, "ERROR: Failed to allocate memory for FIMD-GPU handle!\r\n");
        return NULL;
    }

    handle->config = (struct fimd_gpu_config_s) {
        .image_width = image_width,
        .image_height = image_height,
        .max_markers_count = max_markers_count,
        .max_sun_pts_count = max_sun_pts_count,
        .threshold = threshold,
        .threshold_diff = threshold_diff,
        .threshold_sun = threshold_sun,
        .radii_count = radii_count,
        .radii = {0}
    };
    for (unsigned i = 0; i < radii_count; i++) {
        handle->config.radii[i] = radii[i];
    };

    handle->inst_handle = (struct fimd_gpu_inst_s*) malloc(sizeof(struct fimd_gpu_inst_s));
    if (!handle->inst_handle) {
        fprintf(stderr, "ERROR: Failed to allocate memory for FIMD-GPU instance!\r\n");
        fimd_gpu_destroy(handle);
        return NULL;
    }
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) handle->inst_handle;

    // initialize library context for the first available render device
    char devs[strlen(render_devices)+1];
    strcpy(devs, render_devices);
    char* token = strtok(devs, ",");
    while (token != NULL) {
        fimd_gpu_inst->compute_lib = COMPUTE_LIB_INSTANCE_NEW(token);
        compute_lib_error = compute_lib_init(&fimd_gpu_inst->compute_lib);
        if (compute_lib_error != COMPUTE_LIB_ERROR_GPU_DRI_PATH) break;
        token = strtok(NULL, ",");
    }

    if (compute_lib_error != GL_NO_ERROR) {
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        compute_lib_print_error(compute_lib_error, stderr);
        fimd_gpu_destroy(handle);
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
    
    fimd_gpu_inst->local_size_x = max_local_size_x;
    while (fimd_gpu_inst->local_size_x * (image_width / fimd_gpu_inst->local_size_x) != image_width) {
        fimd_gpu_inst->local_size_x /= 2;
    }
    fimd_gpu_inst->local_size_y = max_invocations / fimd_gpu_inst->local_size_x;
    while (fimd_gpu_inst->local_size_y * (image_height / fimd_gpu_inst->local_size_y) != image_height) {
        fimd_gpu_inst->local_size_y /= 2;
    }

    // initialize shader program and resources
    fimd_gpu_inst->image_in_ssbo = COMPUTE_LIB_SSBO_NEW("image_in_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->image_in_ssbo.resource.value = 1;

    fimd_gpu_inst->sun_pts_count_acbo = COMPUTE_LIB_ACBO_NEW("sun_pts_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->sun_pts_count_acbo.resource.value = 2;

    fimd_gpu_inst->markers_count_acbo = COMPUTE_LIB_ACBO_NEW("markers_count", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->markers_count_acbo.resource.value = 3;

    fimd_gpu_inst->configuration_ssbo = COMPUTE_LIB_SSBO_NEW("configuration_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->configuration_ssbo.resource.value = 4;

    fimd_gpu_inst->markers_ssbo = COMPUTE_LIB_SSBO_NEW("markers_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->markers_ssbo.resource.value = 5;

    fimd_gpu_inst->sun_pts_ssbo = COMPUTE_LIB_SSBO_NEW("sun_pts_buffer", GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
    fimd_gpu_inst->sun_pts_ssbo.resource.value = 6;

    fimd_gpu_inst->compute_prog = COMPUTE_LIB_PROGRAM_NEW(&fimd_gpu_inst->compute_lib, NULL, fimd_gpu_inst->local_size_x, fimd_gpu_inst->local_size_y, 1);

    if(asprintf(&(fimd_gpu_inst->compute_prog.source), _binary_shader_comp_start, "asprintf:\n", fimd_gpu_inst->local_size_x, fimd_gpu_inst->local_size_y) < 0) {
        fprintf(stderr, "ERROR: Failed to format shader source!\r\n");
        return NULL;
    }

    error = compute_lib_program_init(&(fimd_gpu_inst->compute_prog));
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to create program!\r\n");
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }
    
    error = compute_lib_ssbo_init(&(fimd_gpu_inst->image_in_ssbo), NULL, (GLint) image_width * image_height);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->image_in_ssbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    error = compute_lib_acbo_init(&(fimd_gpu_inst->sun_pts_count_acbo), NULL, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init acbo '%s'!\r\n", fimd_gpu_inst->sun_pts_count_acbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    error = compute_lib_acbo_init(&(fimd_gpu_inst->markers_count_acbo), NULL, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init acbo '%s'!\r\n", fimd_gpu_inst->markers_count_acbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    error = compute_lib_ssbo_init(&(fimd_gpu_inst->configuration_ssbo), NULL, (GLint) sizeof(handle->config) / sizeof(uint32_t));
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->configuration_ssbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    error = compute_lib_ssbo_init(&(fimd_gpu_inst->markers_ssbo), NULL, (GLint) max_markers_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->markers_ssbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    error = compute_lib_ssbo_init(&(fimd_gpu_inst->sun_pts_ssbo), NULL, (GLint) max_sun_pts_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to init ssbo '%s'!\r\n", fimd_gpu_inst->sun_pts_ssbo.resource.name);
        compute_lib_error_queue_flush(&fimd_gpu_inst->compute_lib, stderr);
        fimd_gpu_destroy(handle);
        return NULL;
    }

    return handle;
}

int compare_markers_xy1d(const void* a, const void* b) {
    uint16_t* pt1 = (uint16_t*) a;
    uint16_t* pt2 = (uint16_t*) b;
    int res = (int) pt1[0] - (int) pt2[0];
    if (res == 0) res = (int) pt1[1] - (int) pt2[1];
    return res;
}

static unsigned fimd_gpu_get_marker_centroids(uint32_t markers_raw[], uint32_t init_cnt, uint32_t distance_px, unsigned markers[][2]) {
    uvec3_t filtered_markers[init_cnt];
    uint32_t filtered_cnt = 0;
    uint32_t max_dist2 = (distance_px * distance_px);
    uint32_t min_dist2, dist2;
    int32_t closest_marker;
    int32_t i, j;
    uint32_t x2, y2, n;
    uint32_t pt_raw[2];

    qsort(markers_raw, init_cnt, sizeof(markers_raw[0]), compare_markers_xy1d);

    for (i = 0; i < init_cnt; i++) {
        pt_raw[0] = (markers_raw[i] >> 16) & 0x0000FFFF;
        pt_raw[1] = markers_raw[i] & 0x0000FFFF;

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

unsigned fimd_gpu_detect(fimd_gpu_t* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count)
{
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) handle->inst_handle;

    int size = handle->config.image_width * handle->config.image_height;

    unsigned error = 0;
    uint32_t markers_raw[handle->config.max_markers_count];
    uint32_t sun_pts_raw[handle->config.max_sun_pts_count];

    // write configuration
    error = compute_lib_ssbo_write(&fimd_gpu_inst->configuration_ssbo, (void *) &handle->config, (GLint) sizeof(handle->config) / sizeof(uint32_t));
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to write configuration ssbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    // reset atomic counter buffer objects
    error = compute_lib_acbo_write_uint_val(&fimd_gpu_inst->markers_count_acbo, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to reset markers count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    error = compute_lib_acbo_write_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, 0);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to reset sun points count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }
    
    error = compute_lib_ssbo_write(&fimd_gpu_inst->image_in_ssbo, image, (GLint) size / 4);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to write image data to ssbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    // dispatch compute shader
    error = compute_lib_program_dispatch(&fimd_gpu_inst->compute_prog, handle->config.image_width, handle->config.image_height, 1);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to dispatch compute shader! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    // retrieve detected markers
    error = compute_lib_acbo_read_uint_val(&fimd_gpu_inst->markers_count_acbo, (GLuint*) markers_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to read markers count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    if (*markers_count > handle->config.max_markers_count) *markers_count = handle->config.max_sun_pts_count;
    if (*markers_count > 0) {
        error = compute_lib_ssbo_read(&fimd_gpu_inst->markers_ssbo, (void *) markers_raw, (GLint) *markers_count);
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "ERROR: Failed to read markers ssbo! Code: %d\r\n", error);
            compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
            goto detect_end;
        }
        *markers_count = fimd_gpu_get_marker_centroids(markers_raw, *markers_count, 5, markers);
    }

    // retrieve detected sun points
    error = compute_lib_acbo_read_uint_val(&fimd_gpu_inst->sun_pts_count_acbo, (GLuint*) sun_pts_count);
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Failed to read sun points count acbo! Code: %d\r\n", error);
        compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
        goto detect_end;
    }

    if (*sun_pts_count > handle->config.max_sun_pts_count) *sun_pts_count = handle->config.max_sun_pts_count;
    if (*sun_pts_count > 0) {
        error = compute_lib_ssbo_read(&fimd_gpu_inst->sun_pts_ssbo, (void *) sun_pts_raw, (GLint) *sun_pts_count);
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "ERROR: Failed to read sun points ssbo! Code: %d\r\n", error);
            compute_lib_error_queue_flush(&((struct fimd_gpu_inst_s *) handle->inst_handle)->compute_lib, stderr);
            goto detect_end;
        }
        for (int i = 0; i < *sun_pts_count; i++) {
            sun_pts[i][1] = sun_pts_raw[i] & 0x0000FFFF;
            sun_pts[i][0] = (sun_pts_raw[i] >> 16) & 0x0000FFFF;
        }
    }

detect_end:
    return error;
}

void fimd_gpu_destroy(fimd_gpu_t* handle)
{
    struct fimd_gpu_inst_s* fimd_gpu_inst = (struct fimd_gpu_inst_s*) handle->inst_handle;
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->image_in_ssbo));
    compute_lib_acbo_destroy(&(fimd_gpu_inst->markers_count_acbo));
    compute_lib_acbo_destroy(&(fimd_gpu_inst->sun_pts_count_acbo));
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->configuration_ssbo));
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->markers_ssbo));
    compute_lib_ssbo_destroy(&(fimd_gpu_inst->sun_pts_ssbo));
    compute_lib_program_destroy(&(fimd_gpu_inst->compute_prog), GL_TRUE);
    compute_lib_deinit(&fimd_gpu_inst->compute_lib);
    free(fimd_gpu_inst);
    free(handle);
}

