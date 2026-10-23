#ifndef OVERLAY_034_H
#define OVERLAY_034_H

#include "PR/ultratypes.h"

typedef struct Overlay34Input {
    s16 angleX;
    s16 angleY;
    f32 depth;
    f32 x;
    f32 y;
    f32 z;
    f32 value;
    s16 resourceId;
    u8 mode;
    u8 pad1B;
    s32 word1C;
    s32 word20;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
} Overlay34Input;

typedef struct Overlay34Resource {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay34Resource;

typedef struct Overlay34Record {
    u8 byte00;
    u8 byte01;
    u8 byte02;
    u8 byte03;
    s16 short04;
    s16 short06;
    s16 short08;
    s16 short0A;
    s16 short0C;
    s16 short0E;
    u8 byte10;
    u8 byte11;
    u8 byte12;
    u8 byte13;
    s16 short14;
    s16 short16;
    s16 short18;
    s16 short1A;
    s16 short1C;
    s16 short1E;
    Overlay34Resource *resource;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
    s32 word34;
    s32 word38;
    u8 byte3C;
    u8 byte3D;
    u8 active;
    u8 pad3F;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 direction[2];
    f32 depth;
    f32 value;
} Overlay34Record;

/* The update's and the draw's views of the same 0x68-byte record: each names
 * the 0x3C..0x68 fields for its own use (motion, or colours and segment), so
 * they stay views over the pointer table rather than one merged layout. */
typedef struct Overlay34MotionRecord {
    u8 pad00[0x3C];
    u8 timer;
    u8 lifetime;
    u8 active;
    u8 pad3F;
    f32 x;
    f32 y;
    f32 z;
    f32 previousX;
    f32 previousY;
    f32 previousZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 value;
} Overlay34MotionRecord;

typedef struct Overlay34DrawRecord {
    u8 pad00[0x20];
    void *resource;
    u32 color0A;
    u32 color0B;
    u32 color1A;
    u32 color1B;
    u32 color2A;
    u32 color2B;
    u8 frame;
    u8 frameCount;
    u8 active;
    u8 pad3F;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 direction[2];
    f32 depth;
    f32 value;
} Overlay34DrawRecord;

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern s32 gOverlay34ActiveCount;
extern f32 gOverlay34Value10;

extern void overlay34RemoveRecord(Overlay34Record *record);
extern void overlay34InterpolateColor(s32 position, s32 length,
                                      const u8 *start, const u8 *end, u8 *output);

extern void *mmAlloc(s32 size, u32 colourTag);
extern Overlay34Resource *texLoadTexture(s32 resourceId);
extern void mathOneFloatPY(Overlay34Input *input, f32 direction[3]);
extern void texFreeTexture(Overlay34Resource *resource);
extern void mmFree(void *address);
extern f32 camGetProjZ(f32 x, f32 y, f32 z);
extern void func_800084C4(void *commands, void *vertices, void *resource,
                          void *record, f32 *position,
                          f32 *secondPosition, f32 value, u32 color1,
                          u32 color2, s32 scale);

#endif
