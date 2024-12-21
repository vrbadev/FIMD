#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    uint32_t image_size = IM_WIDTH * IM_HEIGHT * sizeof(uint8_t);
    uint8_t* image_data = (uint8_t *) malloc(image_size);
    if (!image_data) {
        perror("Error when allocating memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read the image data
    size_t read_size = fread(image_data, sizeof(uint8_t), IM_WIDTH * IM_HEIGHT, file);
    if (read_size != IM_WIDTH * IM_HEIGHT) {
        fprintf(stderr, "Error when reading file\n");
        free(image_data);
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);

    // Create arrays to store the detected markers and sun points as (x, y)
    unsigned markers[FIMD_MAX_MARKERS_COUNT][2];
    unsigned sun_pts[FIMD_MAX_SUN_PTS_COUNT][2];
    unsigned markers_num = 0;
    unsigned sun_pts_num = 0;

    // Run the detection algorithm for selected radii (must be compiled in the shared library!)

    fimd_cpu_detect(2, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=2: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        printf("(%u,%u),", markers[i][0], markers[i][1]);
    }
    printf("\b]\n");

    fimd_cpu_detect(3, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=3: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        printf("(%u,%u),", markers[i][0], markers[i][1]);
    }
    printf("\b]\n");

    fimd_cpu_detect(4, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=4: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        printf("(%u,%u),", markers[i][0], markers[i][1]);
    }
    printf("\b]\n");

    fimd_cpu_detect(5, image_data, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=5: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        printf("(%u,%u),", markers[i][0], markers[i][1]);
    }
    printf("\b]\n");

    // Free the allocated memory
    free(image_data);

    return EXIT_SUCCESS;
}