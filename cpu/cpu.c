//
// Created by vrbav on 11/21/23.
//

#include "cpu.h"

uint64_t cpu_process_image(uint8_t* image, int* markers_count, md_pt_t* markers, int* sun_pts_count, md_pt_t* sun_pts)
{
    results_t det;
    det.markers_num = 0;
    det.sun_pts_num = 0;

    opt_gen_code(image, &det);

    for (int i = 0; i < det.markers_num; i++) {
        uint64_t pos1d = ((uint64_t) det.markers_raw[i]) - ((uint64_t) image);
        markers[i] = (md_pt_t) {.x = pos1d % IM_WIDTH, .y = pos1d / IM_WIDTH};
    }
    *markers_count = det.markers_num;
    for (int i = 0; i < det.sun_pts_num; i++) {
        uint64_t pos1d = ((uint64_t) det.sun_pts_raw[i]) - ((uint64_t) image);
        sun_pts[i] = (md_pt_t) {.x = pos1d % IM_WIDTH, .y = pos1d / IM_WIDTH};
    }
    *sun_pts_count = det.sun_pts_num;

    return 0;
}

// main function
uint8_t* opt_gen_code(uint8_t* img_ptr, results_t* det_ptr)
{
    // append termination sequence to image end
    *((uint16_t*) ((img_ptr) + (IM_WIDTH * IM_HEIGHT) - 2)) = TERM_SEQ;

    // initial shift by central pixel offset - 1
    img_ptr = (uint8_t*) (img_ptr + (FAST_OFFSET-1));

LOOP:

    // check for the presence of the termination sequence
    if (CHECK_TERM_SEQ(img_ptr)) return img_ptr;

    // load new pixel value from pre-incremented address
    uint8_t pix_val = *((uint8_t*) (++img_ptr));
    if (pix_val <= FAST_THRESHOLD_CENTER) goto LOOP;

    // first boundary pixel test - decide between MARKER_TEST and SUN_TEST
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT00))) <= FAST_THRESHOLD_DIFF) {
        if (pix_val >= FAST_THRESHOLD_SUN) goto SUN_TEST;
    } else {
        goto MARKER_TEST;
    }

    // otherwise go to the next pixel
    goto LOOP;

// testing for sun potential
SUN_TEST:

    // check the current number of the detected sun points
    if (det_ptr->sun_pts_num == MAX_SUN_PTS_COUNT) {
        ADD_TERM_SEQ(img_ptr);
        goto LOOP;
    }

    // boundary pixel 01, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT01))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 02, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT02))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 03, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT03))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 04, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT04))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 05, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT05))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 06, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT06))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 07, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT07))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 08, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT08))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 09, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT09))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 10, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT10))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 11, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT11))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 12, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT12))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 13, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT13))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 14, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT14))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 15, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT15))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 16, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT16))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 17, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT17))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 18, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT18))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 19, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT19))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 20, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT20))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 21, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT21))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 22, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT22))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 23, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FAST_BOUNDARY_PT23))) > FAST_THRESHOLD_DIFF) goto LOOP;

    // interior pixel 00 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT00)) = 0x00;

    // interior pixel 01 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT01)) = 0x00;

    // interior pixel 02 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT02)) = 0x00;

    // interior pixel 03 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT03)) = 0x00;

    // interior pixel 04 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT04)) = 0x00;

    // interior pixel 05 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT05)) = 0x00;

    // interior pixel 06 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT06)) = 0x00;

    // interior pixel 07 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT07)) = 0x00;

    // interior pixel 08 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT08)) = 0x00;

    // interior pixel 09 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT09)) = 0x00;

    // interior pixel 10 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT10)) = 0x00;

    // interior pixel 11 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT11)) = 0x00;

    // interior pixel 12 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT12)) = 0x00;

    // interior pixel 13 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT13)) = 0x00;

    // interior pixel 14 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT14)) = 0x00;

    // interior pixel 15 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT15)) = 0x00;

    // interior pixel 16 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT16)) = 0x00;

    // interior pixel 17 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT17)) = 0x00;

    // interior pixel 18 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT18)) = 0x00;

    // interior pixel 19 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT19)) = 0x00;

    // interior pixel 20 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT20)) = 0x00;

    // interior pixel 21 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT21)) = 0x00;

    // interior pixel 22 set to 0
    *((uint8_t*) (img_ptr + FAST_INTERIOR_PT22)) = 0x00;

    // store current pixel address as sun detection
    det_ptr->sun_pts_raw[det_ptr->sun_pts_num] = img_ptr;
    (det_ptr->sun_pts_num)++;
    goto LOOP;

// testing for marker potential
MARKER_TEST:

    // boundary pixel 01, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT01))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 02, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT02))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 03, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT03))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 04, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT04))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 05, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT05))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 06, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT06))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 07, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT07))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 08, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT08))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 09, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT09))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 10, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT10))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 11, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT11))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 12, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT12))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 13, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT13))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 14, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT14))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 15, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT15))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 16, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT16))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 17, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT17))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 18, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT18))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 19, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT19))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 20, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT20))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 21, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT21))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 22, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT22))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // boundary pixel 23, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FAST_BOUNDARY_PT23))) <= FAST_THRESHOLD_DIFF) goto LOOP;

    // marker potential preserved, search for peak in interior
    uint8_t peak = 0;
    uint8_t* peak_ptr;
    uint8_t* curr_int_ptr;

    // interior pixel 00 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT00);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 01 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT01);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 02 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT02);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 03 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT03);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 04 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT04);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 05 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT05);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 06 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT06);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 07 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT07);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 08 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT08);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 09 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT09);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 10 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT10);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 11 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT11);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 12 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT12);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 13 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT13);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 14 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT14);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 15 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT15);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 16 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT16);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 17 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT17);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 18 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT18);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 19 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT19);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 20 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT20);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 21 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT21);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // interior pixel 22 compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FAST_INTERIOR_PT22);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = curr_int_ptr;
    }
    *curr_int_ptr = 0;

    // store peak address as marker detection
    det_ptr->markers_raw[det_ptr->markers_num] = peak_ptr;
    (det_ptr->markers_num)++;
    if (det_ptr->markers_num == MAX_MARKERS_COUNT) ADD_TERM_SEQ(img_ptr);
    goto LOOP;

}