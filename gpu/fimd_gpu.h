/**
 * \file fimd_gpu.h
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Header file for the FIMD-GPU library.
 * \copyright GNU Public License.
 */

#pragma once

#ifndef FIMD_GPU_H
#define FIMD_GPU_H


/**
 * \brief Initializes a new FIMD-GPU instance with the given parameters.
 * \param image_width Width of the input image.
 * \param image_height Height of the input image.
 * \param threshold Threshold value for marker detection.
 * \param threshold_diff Threshold difference value for marker detection.
 * \param threshold_sun Threshold value for sun point detection.
 * \param max_markers_count Maximum number of markers to detect.
 * \param max_sun_pts_count Maximum number of sun points to detect.
 * \return Pointer to the initialized FIMD-GPU instance, or NULL on failure.
 */
void* fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count);

/**
 * \brief Detects markers and sun points in the given image using the GPU.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param image Pointer to the input image data.
 * \param markers Array to store detected markers.
 * \param markers_count Pointer to store the number of detected markers.
 * \param sun_pts Array to store detected sun points.
 * \param sun_pts_count Pointer to store the number of detected sun points.
 * \return 0 on success, non-zero on failure.
 */
unsigned fimd_gpu_detect(void* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count);

/**
 * \brief Destroys the FIMD-GPU instance and releases associated resources.
 * \param handle Pointer to the FIMD-GPU instance.
 */
void fimd_gpu_destroy(void* handle);


#endif
