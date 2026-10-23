#ifndef OVERLAY_033_H
#define OVERLAY_033_H

#include "PR/ultratypes.h"
#include "overlays/overlay_033_gbi.inc"

typedef struct Overlay33Locals {
    u8 pad88[2];
    u8 alpha;
    u8 blue;
    u8 green;
    u8 red;
    s16 unused;
    s16 high;
    s16 low;
    u32 height;
    u32 width;
    u32 pad9C;
    Gfx *other;
    Gfx *cursor;
} Overlay33Locals;

extern u8 gOverlay33Initialized[];
extern s32 D_8007A3EC;
extern s32 D_800D1C38;
extern s32 D_800D1C34;
extern volatile s32 gOverlay33Allocation;
extern s32 D_800D2CD0;
extern s32 gOverlay33Handle;
/* The colour-image array read by the list builder and the display-list-end
 * array written at its tail are two distinct overlay objects: the reads
 * carry a zero low half, the store carries 0x3788. */
extern void *D_800D1C28[];
extern void *gOverlay33ActiveBuffer;
extern void *gOverlay33DisplayLists[];
extern void *D_800D2FAC;
/* Section-relative placeholders: the display-list arena (0xC00 bytes per
 * buffer) and its tail. */
extern u8 D_8[];
extern u8 D_1808[];

/* SYMBOL calls whose shipped word is zero: the wrappers' callee, and the
 * module's own release (+0x144) and list builder (+0x19C), which the C
 * reaches through the surface's <generated>Reloc proxies. */
extern void overlay33CallReloc(void);
extern void func_overlay_033_F0000144_188092CReloc(void);
extern void func_overlay_033_F000019C_1880984Reloc(void);

extern void viGetCurrentSize(s32 *width, s32 *height);
extern s32 mmAlloc(s32 size, s32 tag);
extern void mmFree(void *handle);
extern s32 osRecvMesg(s32 *context, s32 *status, s32 mode);
extern void rsp_segment(Gfx **cursor, s32 pass, void *value);
extern void rcpInitSp(Gfx **cursor);
extern void rcpInitDpNoSize(Gfx **cursor);
extern void texDPInit(Gfx **cursor);
extern void camSetNo(void *value);
extern void camSetView(Gfx **cursor, Gfx **other);
extern void trackGetFog(void *value, s16 *low, s16 *high,
                                   s16 *unused, u8 *red, u8 *green,
                                   u8 *blue, u8 *alpha);
extern void sprSetTextureFilter(s32 value);
extern s32 rcpFast3d(u64 *dataStart, u64 *dataEnd, s32 taskType,
                     void *framebuffer);

#endif
