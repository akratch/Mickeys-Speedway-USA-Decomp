#ifndef OVERLAY31_BUILD_LOOKUP_TABLES_H
#define OVERLAY31_BUILD_LOOKUP_TABLES_H

#include "PR/ultratypes.h"

/* Tier D: two 16-byte triangles per record (flags, three vertex indices,
 * three s16 texture coordinate pairs), from this function's stores. */
typedef struct Overlay31IndexRecord {
    u8 a0;
    u8 a1;
    u8 a2;
    u8 a3;
    s16 b4;
    s16 b6;
    s16 b8;
    s16 bA;
    s16 bC;
    s16 bE;
    u8 c10;
    u8 c11;
    u8 c12;
    u8 c13;
    s16 d14;
    s16 d16;
    s16 d18;
    s16 d1A;
    s16 d1C;
    s16 d1E;
} Overlay31IndexRecord;

/* Tier B: resident .data identities (selector 0xFFD +0x3D00 and +0x3D38). */
extern Overlay31IndexRecord *gOverlay31IndexRows[7][2];
extern f32 *gOverlay31FloatRows[7];

extern void *overlay31AllocateReloc(s32 size, s32 tag); /* 0:+0x2AE30 */
extern f32 func_8002A8BC_o031Reloc(s32 angle); /* 0:+0x2A46C */
extern f32 func_8002A8C0_o031Reloc(s32 angle); /* 0:+0x2A470 */

#endif
