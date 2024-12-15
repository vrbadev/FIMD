//
// Created by vrbav on 11/21/23.
//

#ifndef MAIN_CPU_H
#define MAIN_CPU_H

#include <stdint.h>

typedef struct {
    uint16_t y;
    uint16_t x;
} md_pt_t;


#define MAX_MARKERS_COUNT 300
#define MAX_SUN_PTS_COUNT 50000

#define THRESHOLD 120
#define THRESHOLD_DIFF 60
#define THRESHOLD_SUN 240

#define IM_WIDTH 752 // Image width in px
#define IM_HEIGHT 480 // Image height in px
#define FAST_RADIUS 4 // FAST radius
#define FAST_THRESHOLD_CENTER 120 // FAST threshold for central brightness test
#define FAST_THRESHOLD_DIFF 60 // FAST threshold for brightness difference test
#define FAST_THRESHOLD_SUN 240 // FAST threshold for sun brighness test
#define FAST_OFFSET (IM_WIDTH*FAST_RADIUS+FAST_RADIUS) // Initial offset from image start address

#define TERM_SEQ 0x00FF
#define ADD_TERM_SEQ(_ptr) (*((uint16_t*) ((_ptr) + FAST_OFFSET)) = TERM_SEQ)
#define CHECK_TERM_SEQ(_ptr) (*((uint16_t*) ((_ptr) + FAST_OFFSET)) == TERM_SEQ)

// FAST boundary points - 1D relative offsets
#define FAST_BOUNDARY_PT00 (0-4*IM_WIDTH)	// (0, -4)
#define FAST_BOUNDARY_PT01 (0+4*IM_WIDTH)	// (0, 4)
#define FAST_BOUNDARY_PT02 (4+0*IM_WIDTH)	// (4, 0)
#define FAST_BOUNDARY_PT03 (-4+0*IM_WIDTH)	// (-4, 0)
#define FAST_BOUNDARY_PT04 (3-3*IM_WIDTH)	// (3, -3)
#define FAST_BOUNDARY_PT05 (-3+3*IM_WIDTH)	// (-3, 3)
#define FAST_BOUNDARY_PT06 (-3-3*IM_WIDTH)	// (-3, -3)
#define FAST_BOUNDARY_PT07 (3+3*IM_WIDTH)	// (3, 3)
#define FAST_BOUNDARY_PT08 (-1-4*IM_WIDTH)	// (-1, -4)
#define FAST_BOUNDARY_PT09 (1+4*IM_WIDTH)	// (1, 4)
#define FAST_BOUNDARY_PT10 (4-1*IM_WIDTH)	// (4, -1)
#define FAST_BOUNDARY_PT11 (-4+1*IM_WIDTH)	// (-4, 1)
#define FAST_BOUNDARY_PT12 (1-4*IM_WIDTH)	// (1, -4)
#define FAST_BOUNDARY_PT13 (-1+4*IM_WIDTH)	// (-1, 4)
#define FAST_BOUNDARY_PT14 (4+1*IM_WIDTH)	// (4, 1)
#define FAST_BOUNDARY_PT15 (-4-1*IM_WIDTH)	// (-4, -1)
#define FAST_BOUNDARY_PT16 (-2-4*IM_WIDTH)	// (-2, -4)
#define FAST_BOUNDARY_PT17 (2+4*IM_WIDTH)	// (2, 4)
#define FAST_BOUNDARY_PT18 (4-2*IM_WIDTH)	// (4, -2)
#define FAST_BOUNDARY_PT19 (-4+2*IM_WIDTH)	// (-4, 2)
#define FAST_BOUNDARY_PT20 (2-4*IM_WIDTH)	// (2, -4)
#define FAST_BOUNDARY_PT21 (-2+4*IM_WIDTH)	// (-2, 4)
#define FAST_BOUNDARY_PT22 (4+2*IM_WIDTH)	// (4, 2)
#define FAST_BOUNDARY_PT23 (-4-2*IM_WIDTH)	// (-4, -2)

// FAST interior points - 1D relative offsets
#define FAST_INTERIOR_PT00 (0+0*IM_WIDTH)	// (0, 0)
#define FAST_INTERIOR_PT01 (1+0*IM_WIDTH)	// (1, 0)
#define FAST_INTERIOR_PT02 (2+0*IM_WIDTH)	// (2, 0)
#define FAST_INTERIOR_PT03 (3+0*IM_WIDTH)	// (3, 0)
#define FAST_INTERIOR_PT04 (-3+1*IM_WIDTH)	// (-3, 1)
#define FAST_INTERIOR_PT05 (-2+1*IM_WIDTH)	// (-2, 1)
#define FAST_INTERIOR_PT06 (-1+1*IM_WIDTH)	// (-1, 1)
#define FAST_INTERIOR_PT07 (0+1*IM_WIDTH)	// (0, 1)
#define FAST_INTERIOR_PT08 (1+1*IM_WIDTH)	// (1, 1)
#define FAST_INTERIOR_PT09 (2+1*IM_WIDTH)	// (2, 1)
#define FAST_INTERIOR_PT10 (3+1*IM_WIDTH)	// (3, 1)
#define FAST_INTERIOR_PT11 (-3+2*IM_WIDTH)	// (-3, 2)
#define FAST_INTERIOR_PT12 (-2+2*IM_WIDTH)	// (-2, 2)
#define FAST_INTERIOR_PT13 (-1+2*IM_WIDTH)	// (-1, 2)
#define FAST_INTERIOR_PT14 (0+2*IM_WIDTH)	// (0, 2)
#define FAST_INTERIOR_PT15 (1+2*IM_WIDTH)	// (1, 2)
#define FAST_INTERIOR_PT16 (2+2*IM_WIDTH)	// (2, 2)
#define FAST_INTERIOR_PT17 (3+2*IM_WIDTH)	// (3, 2)
#define FAST_INTERIOR_PT18 (-2+3*IM_WIDTH)	// (-2, 3)
#define FAST_INTERIOR_PT19 (-1+3*IM_WIDTH)	// (-1, 3)
#define FAST_INTERIOR_PT20 (0+3*IM_WIDTH)	// (0, 3)
#define FAST_INTERIOR_PT21 (1+3*IM_WIDTH)	// (1, 3)
#define FAST_INTERIOR_PT22 (2+3*IM_WIDTH)	// (2, 3)

typedef struct {
    unsigned int markers_num;
    uint8_t* markers_raw[MAX_MARKERS_COUNT];
    unsigned int sun_pts_num;
    uint8_t* sun_pts_raw[MAX_SUN_PTS_COUNT];
} results_t;


uint64_t cpu_process_image(uint8_t* image, int* markers_count, md_pt_t* markers, int* sun_pts_count, md_pt_t* sun_pts);

uint8_t* opt_gen_code(uint8_t* img_ptr, results_t* det_ptr);

#endif //MAIN_CPU_H
