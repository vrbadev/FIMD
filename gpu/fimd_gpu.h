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

void*  fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count);
int fimd_gpu_detect(void* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count);
void fimd_gpu_destroy(void* handle);


#endif
