#ifndef OVERLAY_064_H
#define OVERLAY_064_H

#include "PR/ultratypes.h"
#include "game/fx_texture_image.h"

/* Overlay 64: the procedural texture generator. */

typedef FxTextureImage O64Image;

/* Resident data and functions. */
extern void *D_800D60B0[4];
extern void *D_800D60C0[4];
extern s32 mathRnd(s32 minimum, s32 maximum);

/* This module's export (ORT row). */
void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image, s32 unused);

#endif
