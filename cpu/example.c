/**
 * \file example.c
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Source file for the FIMD-CPU example application.
 * \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>

#include "fimd_cpu.h"


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
    unsigned image_width = fimd_cpu_image_width();
    unsigned image_height = fimd_cpu_image_height();
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

    // Create arrays to store the detected markers and sun points as (x, y)
    unsigned markers[fimd_cpu_get_max_markers_count()][2];
    unsigned sun_pts[fimd_cpu_get_max_sun_points_count()][2];
    unsigned markers_num = 0;
    unsigned sun_pts_num = 0;

    // Run the detection algorithm for all available radii (must be compiled in the shared library!)
    for (unsigned i = 0; i < fimd_cpu_get_radii_count(); i++) {
        unsigned radius = fimd_cpu_get_radii()[i];
        int result = fimd_cpu_detect(radius, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
        if (result != 0) {
            fprintf(stderr, "FIMD-CPU r=%u: ERROR - Return code %d\n\r\n", radius, result);
        } else {
            printf("FIMD-CPU r=%u: detected %d markers, %d sun points.\nMarkers: [", radius, markers_num, sun_pts_num);
            for (int i = 0; i < markers_num; i++) {
                printf("(%u,%u),", markers[i][0], markers[i][1]);
            }
            printf("\b]\n\r\n");
        }
    }

    // Free the allocated memory
    free(image_data);

    return EXIT_SUCCESS;
}