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

#endif //FIMD_CPU_H
