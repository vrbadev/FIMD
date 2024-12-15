#pragma once

#ifndef GPU_H
#define GPU_H

#include <stdio.h>
#include "compute_lib.h"

#define MAX_MARKERS_COUNT 300
#define MAX_SUN_PTS_COUNT 50000

typedef struct {
    uint16_t y;
    uint16_t x;
} md_pt_t;

extern char _binary_shader_comp_start[];

typedef struct mdgpu_s {
    compute_lib_program_t compute_prog;
    compute_lib_image2d_t image_in;//, mask;
    compute_lib_acbo_t markers_count_acbo, sun_pts_count_acbo;
    compute_lib_ssbo_t markers_ssbo, sun_pts_ssbo;
} mdgpu_t;

mdgpu_t* gpu_init(int image_width, int image_height, int threshold, int threshold_diff, int threshold_sun);
uint64_t gpu_process_image(mdgpu_t* self, uint8_t* image, int* markers_count, md_pt_t* markers, int* sun_pts_count, md_pt_t* sun_pts);
void gpu_destroy(mdgpu_t* self);


#endif
