#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern uint8_t* fimd_r2(uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num);
extern uint8_t* fimd_r3(uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num);
extern uint8_t* fimd_r4(uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num);
extern uint8_t* fimd_r5(uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_bin_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* file_path = argv[1];
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("Error when opening file");
        return EXIT_FAILURE;
    }

    uint32_t image_size = IM_WIDTH * IM_HEIGHT * sizeof(uint8_t);
    uint8_t* image_data = (uint8_t *) malloc(image_size);
    uint8_t* tmp = (uint8_t *) malloc(image_size);
    if (!image_data) {
        perror("Error when allocating memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t read_size = fread(image_data, sizeof(uint8_t), IM_WIDTH * IM_HEIGHT, file);
    if (read_size != IM_WIDTH * IM_HEIGHT) {
        fprintf(stderr, "Error when reading file\n");
        free(image_data);
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);

    uintptr_t markers[FIMD_MAX_MARKERS_COUNT];
    uint32_t markers_num = 0;
    uintptr_t sun_pts[FIMD_MAX_SUN_PTS_COUNT];
    uint32_t sun_pts_num = 0;

    memcpy(tmp, image_data, image_size);
    fimd_r2(tmp, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=2: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        uintptr_t pos1d = markers[i] - ((uintptr_t) tmp);
        printf("(%lu,%lu),", pos1d / IM_WIDTH, pos1d % IM_WIDTH);
    }
    printf("\b]\n");

    memcpy(tmp, image_data, image_size);
    fimd_r3(tmp, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=3: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        uintptr_t pos1d = markers[i] - ((uintptr_t) tmp);
        printf("(%lu,%lu),", pos1d / IM_WIDTH, pos1d % IM_WIDTH);
    }
    printf("\b]\n");

    memcpy(tmp, image_data, image_size);
    fimd_r4(tmp, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=4: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        uintptr_t pos1d = markers[i] - ((uintptr_t) tmp);
        printf("(%lu,%lu),", pos1d / IM_WIDTH, pos1d % IM_WIDTH);
    }
    printf("\b]\n");

    memcpy(tmp, image_data, image_size);
    fimd_r5(tmp, markers, &markers_num, sun_pts, &sun_pts_num);
    printf("FIMD-CPU r=5: detected %d markers, %d sun points.\nMarkers: [", markers_num, sun_pts_num);
    for (int i = 0; i < markers_num; i++) {
        uintptr_t pos1d = markers[i] - ((uintptr_t) tmp);
        printf("(%lu,%lu),", pos1d / IM_WIDTH, pos1d % IM_WIDTH);
    }
    printf("\b]\n");

    free(tmp);
    free(image_data);

    return EXIT_SUCCESS;
}