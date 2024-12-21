/*
 * \file fimd_gpu.h
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Header file for the FIMD-GPU library.
 * \copyright GNU Public License.
 */

#pragma once

#ifndef FIMD_GPU_H
#define FIMD_GPU_H

#include "compute_lib.h"

extern char _binary_shader_comp_start[];

struct fimd_gpu_s {
    compute_lib_program_t compute_prog;
    compute_lib_image2d_t image_in;
    compute_lib_acbo_t markers_count_acbo, sun_pts_count_acbo;
    compute_lib_ssbo_t markers_ssbo, sun_pts_ssbo;
};

struct fimd_gpu_s* fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count);
int fimd_gpu_detect(struct fimd_gpu_s* self, uint8_t* image, unsigned* markers_count, unsigned markers[][2], unsigned* sun_pts_count, unsigned sun_pts[][2]);
void fimd_gpu_destroy(struct fimd_gpu_s* self);


#endif
