#ifndef OVERLAY_066_H
#define OVERLAY_066_H

#include "PR/ultratypes.h"
#include "game/framebuffer_state.h"
#include "n_audio/mbi.h"

/* Overlay 66: the full-screen smoothing filter. */

/* Mutable module command templates and filter state. */
extern s32 gOverlay66Timer;
extern s32 gOverlay66Current;
extern s32 gOverlay66Flag;
extern s32 gOverlay66BssControl;
extern Gfx o66SetupCommands[];
extern Gfx o66NtscTextureTemplate[];
extern Gfx o66WideTextureTemplates[];

/* Resident data and functions. */
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
