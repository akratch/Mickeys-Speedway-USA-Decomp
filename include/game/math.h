#ifndef _GAME_MATH_H_
#define _GAME_MATH_H_

#include "PR/ultratypes.h"

/*
 * A 4x4 single-precision matrix, row-major.
 *
 * Field evidence -- func_8002AF6C (ROM 0x2BB6C) reads the four operands
 * of its first output component at 0x00/0x04/0x08/0x0C and of its second at
 * 0x10/0x14/0x18/0x1C, so a row is four contiguous floats and the stride from
 * one row to the next is 0x10. Row-major, 0x40 bytes.
 */
typedef f32 MtxF[4][4];

#endif /* _GAME_MATH_H_ */
