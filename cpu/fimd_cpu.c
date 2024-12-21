#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fimd_cpu.h"
#include "cpp_magic.h"


#define FIMD_FN_TEMPLATE(_r_) uint8_t* fimd_r ## _r_ (uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num);
#define FIMD_SWITCH_TEMPLATE(_r_) case _r_: fimd_r ## _r_ (tmp, markers_ptrs, markers_num, sun_pts_ptrs, sun_pts_num); break;

MAP(FIMD_FN_TEMPLATE, EMPTY, FIMD_RADII)

const uint32_t fimd_radii_list[FIMD_RADII_COUNT] = { FIMD_RADII };


int fimd_cpu_detect(unsigned radius, const unsigned char* img_ptr, unsigned markers[][2], unsigned* markers_num, unsigned sun_pts[][2], unsigned* sun_pts_num)
{
    uint32_t image_size = IM_WIDTH * IM_HEIGHT * sizeof(uint8_t);
    uint8_t* tmp = (uint8_t *) malloc(image_size);
    if (!tmp) {
        return -1; // Memory allocation error
    }
    memcpy(tmp, img_ptr, image_size);

    uintptr_t markers_ptrs[FIMD_MAX_MARKERS_COUNT];
    uintptr_t sun_pts_ptrs[FIMD_MAX_SUN_PTS_COUNT];
    *markers_num = 0;
    *sun_pts_num = 0;

    switch (radius) {
        MAP(FIMD_SWITCH_TEMPLATE, EMPTY, FIMD_RADII)
        default:
            free(tmp);
            return -2; // Invalid radius
    }

    uintptr_t pos1d;
    for (int i = 0; i < *markers_num; i++) {
        pos1d = markers_ptrs[i] - ((uintptr_t) tmp);
        markers[i][1] = pos1d / IM_WIDTH;
        markers[i][0] = pos1d % IM_WIDTH;
    }

    for (int i = 0; i < *sun_pts_num; i++) {
        pos1d = sun_pts_ptrs[i] - ((uintptr_t) tmp);
        sun_pts[i][1] = pos1d / IM_WIDTH;
        sun_pts[i][0] = pos1d % IM_WIDTH;
    }

    free(tmp);
    return 0;
}

const unsigned fimd_cpu_image_width() {
    return IM_WIDTH;
}

const unsigned fimd_cpu_image_height() {
    return IM_HEIGHT;
}

const unsigned fimd_cpu_get_radii_count() {
    return FIMD_RADII_COUNT;
}

const unsigned* fimd_cpu_get_radii() {
    return fimd_radii_list;
}

const unsigned fimd_cpu_get_max_markers_count() {
    return FIMD_MAX_MARKERS_COUNT;
}

const unsigned fimd_cpu_get_max_sun_points_count() {
    return FIMD_MAX_SUN_PTS_COUNT;
}

const unsigned fimd_cpu_get_threshold_marker() {
    return FIMD_THRESHOLD_CENTER;
}

const unsigned fimd_cpu_get_threshold_sun() {
    return FIMD_THRESHOLD_SUN;
}

const unsigned fimd_cpu_get_threshold_diff() {
    return FIMD_THRESHOLD_DIFF;
}

const unsigned fimd_cpu_get_termination_sequence() {
    return FIMD_TERM_SEQ;
}





