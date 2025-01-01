//$ GEN_OUTPUT.append("""
/**
 * \\file fimd_r%d_armasm.s
 * \\author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \\date December 2024
 * \\brief Generated ARM assembly source file for the FIMD-CPU library.
 * \\copyright GNU Public License.
 */
//$ """ % (FIMD_RADIUS))

//$ GEN_OUTPUT.append("""
#include "fimd_armasm.h"
.syntax unified
.text
.global fimd_rXX_armasm
.p2align 2
.type fimd_rXX_armasm,%function
//$ """.replace("XX", str(FIMD_RADIUS)))

//$ GEN_OUTPUT.append("""
// start of the FIMD function
fimd_rXX_armasm:
.fnstart
	// initial shift of start address in R0
	MOV R12, #(FIMD_OFFSET-1)
	ADD R0, R0, R12
//$ """.replace("XX", str(FIMD_RADIUS)))

//$ GEN_OUTPUT.append("""
// main loop
LOOP:
    LDRH R2, [R0, #(FIMD_OFFSET)] // R2 = *((uint16_t*) (R0 + FIMD_OFFSET));
	CMP R2, #(FIMD_TERM_SEQ)
	IT EQ
	BXEQ LR // if (R2 == FIMD_TERM_SEQ) return;
	// current pixel to R2, next iteration if R2 <= Tm
	LDRB R2, [R0, #1]! // R2 = *((uint8_t*) (++R0));
	CMP R2, #(FIMD_THRESHOLD_CENTER)
	BLE LOOP // if (R2 <= FIMD_THRESHOLD_CENTER) goto LOOP;
	// first boundary test
	// boundary pixel to R3, compare R3-R2 and Td
	LDR R3, =(FIMD_BOUNDARY_PTxx)  // R3 = FIMD_BOUNDARY_PTxx;
	LDRB R3, [R0, R3] // R3 = *((uint8_t*) (R0 + R3));
	SUB R3, R3, R2 // R3 = R3 - R2;
	CMN R3, #(FIMD_THRESHOLD_DIFF) // if (R3 < FIMD_THRESHOLD_DIFF) {
	// go to sun test if R3-R2 >= -Td and R2 >= Td
	ITT GE
	CMPGE R2, #(FIMD_THRESHOLD_SUN)
	BGE SUN_TEST // if (R2 >= FIMD_THRESHOLD_SUN) goto SUN_TEST; }
	// go to marker test if R3-R2 < Td
	CMN R3, #(FIMD_THRESHOLD_DIFF)
	IT LT
	BLT MARKER_TEST // if (R3 < FIMD_THRESHOLD_DIFF) goto MARKER_TEST;
	// otherwise go to next pixel
	B LOOP // goto LOOP;
//$ """.replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % FIMD_BOUNDARY[0]))

//$ GEN_OUTPUT.append("""
// testing for sun potential
SUN_TEST:
	// check current number of detected sun points
	MOV R12, #(FIMD_MAX_MARKERS_COUNT+1)
	LDR R3, [R1, R12, LSL #2]
	CMP R3, #(FIMD_MAX_SUN_PTS_COUNT)
	ITTT EQ
	MOVEQ R2, #(FIMD_TERM_SEQ)
	STREQ R2, [R0, #(FIMD_OFFSET)]
	BEQ LOOP
//$ """)

//$ for i, (y, x) in enumerate(FIMD_BOUNDARY[1:]):
//$     GEN_OUTPUT.append(("""
    // boundary pixel #%d to R3, compare R3-R2 and Td
	LDR R3, =(FIMD_BOUNDARY_PTxx)
	LDRB R3, [R0, R3]
	SUB R3, R3, R2
	CMN R3, #(FIMD_THRESHOLD_DIFF)
	// losing sun potential if R3-R2 < Td
	IT LT
	BLT LOOP
//$     """ % (i+1)).replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
	// sun potential preserved
	MOV R3, #0
//$ """)

//$ for i, (y, x) in enumerate(FIMD_INTERIOR):
//$     GEN_OUTPUT.append(("""
	// interior pixel #%d set to 0
	STRB R3, [R0, #(FIMD_INTERIOR_PTxx)]
//$     """ % (i)).replace("FIMD_INTERIOR_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
	// store index R0 as sun detection
	LDR R3, [R1, R12, LSL #2]
	ADD R3, R3, #1
	STR R3, [R1, R12, LSL #2]
	ADD R12, R12, R3
	STR R0, [R1, R12, LSL #2]
	B LOOP
//$ """)

//$ GEN_OUTPUT.append("""
// testing for marker potential
MARKER_TEST:
//$ """)

//$ for i, (y, x) in enumerate(FIMD_BOUNDARY[1:]):
//$     GEN_OUTPUT.append(("""
    // boundary pixel #%d to R3, compare R3-R2 and Td
	LDR R3, =(FIMD_BOUNDARY_PTxx)
	LDRB R3, [R0, R3]
	SUB R3, R3, R2
	CMN R3, #(FIMD_THRESHOLD_DIFF)
	// losing marker potential if R3-R2 >= Td
	IT GE
	BGE LOOP
//$     """ % (i+1)).replace("FIMD_BOUNDARY_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
	// marker potential preserved, search for peak in interior
	MOV R2, #0
//$ """)

//$ for i, (y, x) in enumerate(FIMD_INTERIOR):
//$     GEN_OUTPUT.append(("""
	// interior pixel #%d to R3, compare with latest peak in R2
	LDRB R3, [R0, #(FIMD_INTERIOR_PTxx)]
	CMP R3, R2
	// store peak value R3 to R2, peak index R0+shift to R12
	ITTT GT
	MOVGT R2, R3
	MOVGT R3, #(FIMD_INTERIOR_PTxx)
	ADDGT R12, R3, R0
	// set interior pixel to 0 (mark as touched)
	MOV R3, #0
	STRB R3, [R0, #(FIMD_INTERIOR_PTxx)]
//$     """ % (i)).replace("FIMD_INTERIOR_PTxx", "(((%d)*(IM_WIDTH))+(%d))" % (y, x)))

//$ GEN_OUTPUT.append("""
	// store peak index R12 as marker detection
	LDR R3, [R1]
	ADD R3, R3, #1
	STR R12, [R1, R3, LSL #2]
	STR R3, [R1]
	CMP R3, #(FIMD_MAX_MARKERS_COUNT)
	ITT GE
	MOVGE R2, #(FIMD_TERM_SEQ)
	STRGE R2, [R0, #(FIMD_OFFSET)]
	B LOOP
//$ """)

//$ GEN_OUTPUT.append("""
.fnend
//$ """)
