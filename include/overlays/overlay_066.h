#ifndef OVERLAY_066_H
#define OVERLAY_066_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/* Overlay 66: the full-screen smoothing filter. */

/* Module data, still owned by the raw image (phase 3 of
 * docs/OVERLAY_STRUCTURE_SPRINT.md). */
extern s32 gOverlay66Timer;
extern s32 gOverlay66Current;
extern s32 gOverlay66Flag;
extern s32 gOverlay66BssControl;
extern Gfx D_0[];
extern Gfx D_50[];
extern Gfx D_88[];

/* Resident data and functions. */
extern u16 *D_800D2FA0;
extern u16 *D_800D2FAC;
extern u16 *D_800D2FA8;
extern void osInvalDCache(void *vaddr, s32 nbytes);
extern void osWritebackDCacheAll(void);
extern s32 viGetVideoMode(void);
extern void texDPInit(Gfx **displayList);
extern void rsp_segment(Gfx **displayList, s32 segment, void *base);
extern void viGetCurrentSize(u32 *width, u32 *height);

/* This module's exports (ORT rows). */
void overlay66Select(s32 selection);
s32 overlay66GetCurrent(void);
void func_overlay_066_F0000040_18C64A8(Gfx **commands);

#endif
