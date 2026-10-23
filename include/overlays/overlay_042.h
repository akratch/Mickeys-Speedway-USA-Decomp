#ifndef OVERLAY_042_H
#define OVERLAY_042_H

#include "PR/ultratypes.h"

#include "n_audio/mbi.h"

extern Gfx *gOverlay42CommandBuffers[2];
extern s32 gOverlay42BufferIndex;
extern u8 gOverlay42ImageAlpha;
extern Gfx *gOverlay42PublishedStart;
extern Gfx *gOverlay42PublishedEnd;
extern void *gOverlay42PublishedSource;

extern s32 D_8007A20C;
extern s32 D_8007A410;
extern void *D_800D2FA4;
extern void *D_800D2FAC;
extern u16 *D_800D2FA0;

void *mmAlloc(s32 size, s32 tag);
void mmFree(void *allocation);
s32 rcpFast3d(u64 *dataStart, u64 *dataEnd, s32 taskType, void *framebuffer);
s32 viGetTrippleBuffer();
s32 viGetCurrentSize();
s32 rsp_segment();
s32 rcpInitSp();
s32 rcpInitDpNoSize();

void overlay42Init(void);
void overlay42Release(void);
void overlay42Resume(void);
void overlay42DrawCapturedBuffer(void *callbackArgument);
void overlay42Present(void);

#endif
