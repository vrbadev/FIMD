//$ GEN_OUTPUT.append("""
#include <stdint.h>

#define FIMD_RADIUS 0 // placeholder
#define FIMD_BOUNDARY_PTxx 0 // placeholder
#define FIMD_INTERIOR_PTxx 0 // placeholder
#define FIMD_OFFSET ((IM_WIDTH * FIMD_RADIUS) + FIMD_RADIUS)
#define ADD_TERM_SEQ(_ptr) (*((uint16_t*) ((_ptr) + FIMD_OFFSET)) = FIMD_TERM_SEQ)
#define CHECK_TERM_SEQ(_ptr) *((uint16_t*) ((_ptr) + FIMD_OFFSET)) == FIMD_TERM_SEQ
//$ """.replace("FIMD_RADIUS 0", "FIMD_RADIUS %d" % (FIMD_RADIUS)))

//$ GEN_OUTPUT.append("""
uint8_t* FIMD_FUNC(uint8_t* img_ptr, uintptr_t* markers, uint32_t* markers_num, uintptr_t* sun_pts, uint32_t* sun_pts_num)
{
    // append termination sequence to image end
    *((uint16_t*) ((img_ptr) + (IM_WIDTH * IM_HEIGHT) - 2)) = FIMD_TERM_SEQ;

    // initial shift by central pixel offset - 1
    img_ptr = (uint8_t*) (img_ptr + (FIMD_OFFSET-1));
//$ """.replace("FIMD_FUNC", "fimd_r%d" % (FIMD_RADIUS)))

//$ GEN_OUTPUT.append("""
LOOP:
    // check for the presence of the termination sequence
    if (CHECK_TERM_SEQ(img_ptr)) return img_ptr;

    // load new pixel value from pre-incremented address
    uint8_t pix_val = *((uint8_t*) (++img_ptr));
    if (pix_val <= FIMD_THRESHOLD_CENTER) goto LOOP;

    // first boundary pixel test - decide between MARKER_TEST and SUN_TEST
    if ((pix_val - *((uint8_t*) (img_ptr + FIMD_BOUNDARY_PTxx))) <= FIMD_THRESHOLD_DIFF) {
        if (pix_val >= FIMD_THRESHOLD_SUN) goto SUN_TEST;
    } else {
        goto MARKER_TEST;
    }

    // otherwise go to the next pixel
    goto LOOP;
//$ """.replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % FIMD_BOUNDARY[0]))

//$ GEN_OUTPUT.append("""
// testing for sun potential
SUN_TEST:
    // check the current number of the detected sun points
    if (*sun_pts_num == FIMD_MAX_SUN_PTS_COUNT) {
        ADD_TERM_SEQ(img_ptr);
        goto LOOP;
    }
//$ """)

//$ for i, (y, x) in enumerate(FIMD_BOUNDARY[1:]):
//$     GEN_OUTPUT.append(("""
    // boundary pixel %d, compare difference from central pixel
    if ((pix_val - *((uint8_t*) (img_ptr + FIMD_BOUNDARY_PTxx))) > FIMD_THRESHOLD_DIFF) goto LOOP;
//$     """ % (i+1)).replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ for i, (y, x) in enumerate(FIMD_INTERIOR):
//$     GEN_OUTPUT.append(("""
    // interior pixel %d set to 0
    *((uint8_t*) (img_ptr + FIMD_INTERIOR_PTxx)) = 0x00;
//$     """ % (i)).replace("FIMD_INTERIOR_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
    // store current pixel address as sun detection
    sun_pts[*sun_pts_num] = (uintptr_t) img_ptr;
    (*sun_pts_num)++;
    goto LOOP;
//$ """)

//$ GEN_OUTPUT.append("""
// testing for marker potential
MARKER_TEST:
//$ """)

//$ for i, (y, x) in enumerate(FIMD_BOUNDARY[1:]):
//$     GEN_OUTPUT.append(("""
    // boundary pixel %d, compare difference from central pixel
    if (pix_val - (*((uint8_t*) (img_ptr + FIMD_BOUNDARY_PTxx))) <= FIMD_THRESHOLD_DIFF) goto LOOP;
//$     """ % (i+1)).replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
    // marker potential preserved, search for peak in interior
    uint8_t peak = 0;
    uintptr_t peak_ptr = 0;
    uint8_t* curr_int_ptr = 0;
//$ """)

//$ for i, (y, x) in enumerate(FIMD_INTERIOR):
//$     GEN_OUTPUT.append(("""
    // interior pixel %d compare with latest peak
    curr_int_ptr = (uint8_t*) (img_ptr + FIMD_INTERIOR_PTxx);
    if (*curr_int_ptr > peak) {
        peak = *curr_int_ptr;
        peak_ptr = (uintptr_t) curr_int_ptr;
    }
    *curr_int_ptr = 0;
//$     """ % (i)).replace("FIMD_INTERIOR_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))


//$ GEN_OUTPUT.append("""
    // store peak address as marker detection
    markers[*markers_num] = peak_ptr;
    (*markers_num)++;
    if (*markers_num == FIMD_MAX_MARKERS_COUNT) ADD_TERM_SEQ(img_ptr);
    goto LOOP;
}
//$ """)