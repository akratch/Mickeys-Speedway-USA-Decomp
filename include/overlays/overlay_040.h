#ifndef OVERLAY_040_H
#define OVERLAY_040_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/*
 * Overlay 40: one translation unit (ADR 0006). Eight highlight entries drawn
 * as projected frames around tracked objects, a fade blend over track vertex
 * colours, and a full-screen tint. DKR v77/v80 has related menu projection
 * and timed interpolation (weather.c), but object and source checks found no
 * corresponding donor for any function here.
 */

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

typedef struct Overlay40Node {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *effect64;
} Overlay40Node;

typedef struct Overlay40Object {
    u8 pad00[8];
    Overlay40Node *node;
} Overlay40Object;

typedef struct Overlay40FrameRecord {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 color;
} Overlay40FrameRecord;

typedef struct Overlay40Color {
    u8 red;
    u8 green;
    u8 blue;
} Overlay40Color;

typedef struct Overlay40ColorSource {
    Overlay40Color *colors;
    s16 *groups;
} Overlay40ColorSource;

typedef struct Overlay40Vertex {
    u8 pad00[6];
    s8 red;
    s8 green;
    s8 blue;
    u8 pad09;
} Overlay40Vertex;

typedef struct Overlay40FadeRecord {
    Overlay40Vertex *vertices;
    u8 pad04[0x1C];
    s16 count;
    u8 pad22[0xC];
    u8 dirty;
    u8 pad2F;
    Overlay40ColorSource *source;
    u8 pad34[0xC];
} Overlay40FadeRecord;

typedef struct Overlay40FadeContext {
    u8 pad00[4];
    Overlay40FadeRecord *records;
    u8 pad08[0x12];
    s16 count;
} Overlay40FadeContext;

extern s32 D_8007D6B0;
extern Overlay40Entry D_800D6BF8[8];
extern Overlay40Object **D_800D6B00;

extern s16 D_800D6C38;
extern s16 gOverlay40Value1;
extern s16 D_800D6C44;
extern s16 D_800D6C3E;
extern s16 gOverlay40Value4;
extern s16 gOverlay40Value5;
extern s16 gOverlay40Value6;
extern s16 gOverlay40Value7;

extern s16 gOverlay40BlendTimer;
extern s16 gOverlay40BlendCurrent;
extern s16 gOverlay40BlendOutput;
extern s16 gOverlay40BlendTarget;
extern s16 gOverlay40BlendDuration;

extern s16 gO40TintGreenReloc, gO40TintBlueReloc;
extern Gfx gO40TintDisplayListReloc[];
extern void o40TintCallReloc();

/* frontDrawRectangles (resident 0x80039380), reached through the overlay
 * loader's SYMBOL record; the placeholder carries the shipped jal addend. */
extern void frontDrawRectangles(void *displayList, s32 count,
                                Overlay40FrameRecord *records,
                                s32 translucent);
extern s32 camProjectPoint(f32 x, f32 y, f32 z, f32 *screenX,
                           f32 *screenY, s32 flags);

void overlay40AddEntry(volatile s32 id, s32 red, s32 green, s32 blue, s32 unused);
void overlay40RemoveEntry(s32 id);
void overlay40UpdateEntries(s32 amount);
void overlay40DrawEntries(void *displayList);
void overlay40SetValues(s32 value0, s32 value1, s32 value2, s32 value3,
                        s32 value4);
void overlay40Interpolate(s32 amount);
void overlay40DrawTintRectangle(Gfx **displayList);
void overlay40FadeRecords(s32 *enabled, Overlay40FadeContext *context,
                          s32 amount);

#endif
