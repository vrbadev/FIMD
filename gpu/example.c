/**
 * \file example.c
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Source file for the FIMD-GPU example application.
 * \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>

#include "fimd_gpu.h"

// user settings for FIMD-GPU
// constants - used during initialization:
static unsigned image_width = 752;
static unsigned image_height = 480;
static unsigned max_markers_count = 300;
static unsigned max_sun_points_count = 50000;
// these may be changed between detector executions:
static unsigned threshold = 120;
static unsigned threshold_diff = 60;
static unsigned threshold_sun = 240;
static unsigned radii[] = { 2, 3, 4 }; // tested sequentially in the given order, until detection is successful


int main(int argc, char *argv[]) {
    // Print usage if the number of arguments is incorrect
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_bin_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Try to open the file
    const char* file_path = argv[1];
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("Error when opening file");
        return EXIT_FAILURE;
    }

    // Allocate memory for the image
    unsigned image_size_bytes = image_width * image_height * sizeof(unsigned char);
    unsigned char* image_data = (unsigned char *) malloc(image_size_bytes);
    if (!image_data) {
        perror("Error when allocating memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read the image data
    unsigned read_size = fread(image_data, sizeof(unsigned char), image_width * image_height, file);
    if (read_size != image_width * image_height) {
        fprintf(stderr, "Error when reading file\n");
        free(image_data);
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);

    // Initialize the FIMD-GPU library
    fimd_gpu_t* handle = fimd_gpu_init(image_width, image_height, threshold, threshold_diff, threshold_sun, max_markers_count, max_sun_points_count, sizeof(radii) / sizeof(radii[0]), radii);
    if (!handle) {
        perror("Failed to initialize FIMD-GPU library!");
        return EXIT_FAILURE;
    }

    // Create arrays to store the detected markers and sun points as (x, y)
    unsigned (*markers)[2] = (unsigned(*)[2]) malloc(max_markers_count * 2 * sizeof(unsigned));
    unsigned (*sun_pts)[2] = (unsigned(*)[2]) malloc(max_sun_points_count * 2 * sizeof(unsigned));
    unsigned markers_num = 0;
    unsigned sun_pts_num = 0;

    // Run the detection algorithm for all selected radii
    unsigned result = fimd_gpu_detect(handle, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
    if (result != 0) {
        fprintf(stderr, "FIMD-GPU: ERROR - Return code %d\n\r\n", result);
    } else {
        printf("FIMD-GPU: detected %d marker(s), %d sun pixel(s).\nMarker(s): [", markers_num, sun_pts_num);
        for (int i = 0; i < markers_num; i++) {
            printf("(%u,%u),", markers[i][0], markers[i][1]);
        }
        printf("%s]\n\r\n", markers_num > 0 ? "\b" : "");
    }

    // Deinitialize library
    fimd_gpu_destroy(handle);

    // Free the allocated memory
    free(markers);
    free(sun_pts);
    free(image_data);

    return EXIT_SUCCESS;
}