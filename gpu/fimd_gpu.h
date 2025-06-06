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

#include <stdint.h>

struct fimd_gpu_config_s {
    uint32_t image_width;
    uint32_t image_height;
    uint32_t threshold;
    uint32_t threshold_diff;
    uint32_t threshold_sun;
    uint32_t max_markers_count;
    uint32_t max_sun_pts_count;
    uint32_t radii_count;
    uint32_t radii[64];
};

typedef struct fimd_gpu_handle_s {
    void* inst_handle;
    struct fimd_gpu_config_s config;
} fimd_gpu_t;


/**
 * \brief Initializes a new FIMD-GPU instance with the given parameters.
 * \param image_width Width of the input image.
 * \param image_height Height of the input image.
 * \param threshold Threshold value for marker detection.
 * \param threshold_diff Threshold difference value for marker detection.
 * \param threshold_sun Threshold value for sun point detection.
 * \param max_markers_count Maximum number of markers to detect.
 * \param max_sun_pts_count Maximum number of sun points to detect.
 * \param radii_count Number of radii to use for marker detection.
 * \param radii Array of radii to use for marker detection.
 * \return Pointer to the initialized FIMD-GPU instance, or NULL on failure.
 */
fimd_gpu_t* fimd_gpu_init(unsigned image_width, unsigned image_height, unsigned threshold, unsigned threshold_diff, unsigned threshold_sun, unsigned max_markers_count, unsigned max_sun_pts_count, unsigned radii_count, unsigned* radii);

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
unsigned fimd_gpu_detect(fimd_gpu_t* handle, unsigned char* image, unsigned markers[][2], unsigned* markers_count, unsigned sun_pts[][2], unsigned* sun_pts_count);

/**
 * \brief Destroys the FIMD-GPU instance and releases associated resources.
 * \param handle Pointer to the FIMD-GPU instance.
 */
void fimd_gpu_destroy(fimd_gpu_t* handle);
/**
 * \brief Sets the image width for the FIMD-GPU instance.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param image_width New image width.
 */
void fimd_gpu_set_image_width(fimd_gpu_t* handle, unsigned image_width);

/**
 * \brief Sets the image height for the FIMD-GPU instance.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param image_height New image height.
 */
void fimd_gpu_set_image_height(fimd_gpu_t* handle, unsigned image_height);

/**
 * \brief Sets the threshold for marker detection.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param threshold New threshold value.
 */
void fimd_gpu_set_threshold(fimd_gpu_t* handle, unsigned threshold);
/**
 * \brief Sets the threshold difference for marker detection.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param threshold_diff New threshold difference value.
 */
void fimd_gpu_set_threshold_diff(fimd_gpu_t* handle, unsigned threshold_diff);
/**
 * \brief Sets the threshold for sun point detection.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param threshold_sun New threshold value for sun point detection.
 */
void fimd_gpu_set_threshold_sun(fimd_gpu_t* handle, unsigned threshold_sun);
/**
 * \brief Sets the maximum number of markers to detect.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param max_markers_count New maximum number of markers.
 */
void fimd_gpu_set_max_markers_count(fimd_gpu_t* handle, unsigned max_markers_count);
/**
 * \brief Sets the maximum number of sun points to detect.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param max_sun_pts_count New maximum number of sun points.
 */
void fimd_gpu_set_max_sun_pts_count(fimd_gpu_t* handle, unsigned max_sun_pts_count);
/**
 * \brief Sets the radii for marker detection.
 * \param handle Pointer to the FIMD-GPU instance.
 * \param radii_count Number of radii.
 * \param radii Array of radii values.
 */
void fimd_gpu_set_radii(fimd_gpu_t* handle, unsigned radii_count, unsigned* radii);


#endif
