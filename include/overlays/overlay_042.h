#ifndef OVERLAY_042_H
#define OVERLAY_042_H

#include "PR/ultratypes.h"

typedef struct Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
} Gfx;

extern Gfx *gOverlay42Buffers[2];
extern void *gOverlay42Buffer0;
extern void *gOverlay42Buffer1;
extern s32 gOverlay42Buffer2;
extern s32 gOverlay42State0;
extern s32 gOverlay42State1;
extern s32 D_8007A20C;
extern s32 D_8007A410;
extern s32 gOverlay42BufferIndex;

extern u8 gO42BufferIndexBaseReloc[];
extern Gfx *gO42PublishedStartReloc;
extern void *D_800D2FA4;
extern u8 gO42PublishedSourceBaseReloc[];
extern void *D_800D2FAC;
extern u16 *D_800D2FA0;
extern u8 gO42ImageStateReloc[];
extern u8 gO42PublishedEndBaseReloc[];

void overlay42PrepareReloc(void);
void *mmAlloc(s32 size, s32 tag);
void mmFree(void *allocation);
void overlay42ResumeReloc(void);
void rcpFast3d(void *buffer, void *value, s32 mode, s32 index);
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
