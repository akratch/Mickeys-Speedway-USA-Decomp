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
extern s32 gOverlay42Ready;
extern s32 gOverlay42Active;
extern s32 gOverlay42BufferIndex;

extern u8 gO42BufferIndexBaseReloc[];
extern Gfx *gO42PublishedStartReloc;
extern void *gO42Segment1BaseReloc;
extern u8 gO42PublishedSourceBaseReloc[];
extern void *gO42Segment2BaseReloc;
extern u16 *gO42TextureSourceReloc;
extern u8 gO42ImageStateReloc[];
extern u8 gO42PublishedEndBaseReloc[];

void overlay42PrepareReloc(void);
void *overlay42AllocReloc(s32 size, s32 tag);
void overlay42FreeReloc(void *allocation);
void overlay42ResumeReloc(void);
void overlay42PresentReloc(void *buffer, void *value, s32 mode, s32 index);
s32 overlay42ReadinessReloc();
s32 overlay42GetSizeReloc();
s32 overlay42RspSegmentReloc();
s32 overlay42SetupDisplayAReloc();
s32 overlay42SetupDisplayBReloc();

void overlay42Init(void);
void overlay42Release(void);
void overlay42Resume(void);
void overlay42DrawCapturedBuffer(void *callbackArgument);
void overlay42Present(void);

#endif
