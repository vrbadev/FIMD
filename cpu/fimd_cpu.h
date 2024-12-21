#ifndef FIMD_CPU_H
#define FIMD_CPU_H

/**
 * \brief Detects markers and sun points in a given image.
 *
 * This function processes a grayscale image to detect markers and sun points based on the specified radius.
 *
 * \param radius The radius used for detection.
 * \param img_ptr Pointer to the image data (grayscale, 8-bit per pixel).
 * \param markers Array to store the detected markers' coordinates. Each marker is represented by a pair of coordinates (x, y).
 * \param markers_num Pointer to an unsigned integer to store the number of detected markers.
 * \param sun_pts Array to store the detected sun points' coordinates. Each sun point is represented by a pair of coordinates (x, y).
 * \param sun_pts_num Pointer to an unsigned integer to store the number of detected sun points.
 * \return An integer indicating the success or failure of the detection process. Returns 0 on success, -1 on memory allocation error, and -2 on invalid radius.
 */
int fimd_cpu_detect(unsigned radius, const unsigned char* img_ptr, unsigned markers[][2], unsigned* markers_num, unsigned sun_pts[][2], unsigned* sun_pts_num);

/**
 * \brief Gets the width of the image used in the FIMD-CPU detection.
 *
 * \return The width of the image in pixels.
 */
const unsigned fimd_cpu_image_width();

/**
 * \brief Gets the height of the image used in the FIMD-CPU detection.
 *
 * \return The height of the image in pixels.
 */
const unsigned fimd_cpu_image_height();

/**
 * \brief Gets the count of radii used in the FIMD-CPU detection.
 *
 * \return The number of different radii used for detection.
 */
const unsigned fimd_cpu_get_radii_count();

/**
 * \brief Gets the array of radii used in the FIMD-CPU detection.
 *
 * \return A pointer to an array of unsigned integers representing the radii.
 */
const unsigned* fimd_cpu_get_radii();

/**
 * \brief Gets the maximum number of markers that can be detected.
 *
 * \return The maximum number of markers.
 */
const unsigned fimd_cpu_get_max_markers_count();

/**
 * \brief Gets the maximum number of sun points that can be detected.
 *
 * \return The maximum number of sun points.
 */
const unsigned fimd_cpu_get_max_sun_points_count();

/**
 * \brief Gets the threshold value for marker detection.
 *
 * \return The threshold value for detecting markers.
 */
const unsigned fimd_cpu_get_threshold_marker();

/**
 * \brief Gets the threshold value for sun point detection.
 *
 * \return The threshold value for detecting sun points.
 */
const unsigned fimd_cpu_get_threshold_sun();

/**
 * \brief Gets the threshold value for the difference used in detection.
 *
 * \return The threshold value for the difference.
 */
const unsigned fimd_cpu_get_threshold_diff();

/**
 * \brief Gets the termination sequence value used in the FIMD-CPU detection.
 *
 * \return The termination sequence value.
 */
const unsigned fimd_cpu_get_termination_sequence();


#endif //FIMD_CPU_H
