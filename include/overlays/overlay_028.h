#ifndef OVERLAY_028_H
#define OVERLAY_028_H

#include "ultra64.h"

typedef union Overlay28Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
    u32 force_alignment;
} Overlay28Gfx;

typedef struct Overlay28Owner Overlay28Owner;
typedef struct Overlay28Work Overlay28Work;

typedef struct Overlay28Entry {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay28Entry;

typedef struct Overlay28ResetState {
    u8 pad00[0x42];
    s16 bufferIndex;
    Overlay28Entry entries[2][17];
} Overlay28ResetState;

typedef struct Overlay28VertexState {
    u8 pad00[0x18];
    f32 baseScale;
    f32 xScale;
    f32 yScale;
    s16 xAngle;
    s16 yAngle;
    s16 xStep;
    s16 yStep;
    u8 pad2C[0x16];
    s16 bufferIndex;
    u8 pad44[0xA];
    Overlay28Entry points[2][17];
} Overlay28VertexState;

typedef struct Overlay28Source {
    u8 pad00[4];
    s16 x;
    s16 y;
    s16 z;
    u8 valueA;
    u8 valueB;
    void *related;
} Overlay28Source;

struct Overlay28Work {
    Overlay28Owner *related;
    f32 x;
    f32 y;
    f32 z;
    f32 valueA;
    void *handle;
    f32 valueB;
    f32 scaleA;
    f32 scaleB;
    s16 angleA;
    s16 angleB;
    s16 stepA;
    s16 stepB;
    s16 stepC;
    s16 stepD;
    u8 object[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad3B;
    void (*reset)();
    s16 intensity;
    s16 bufferIndex;
    u8 vertices[0x154];
};

struct Overlay28Owner {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 angle;
    u8 pad30[9];
    u8 intensityScale;
    u8 pad3A[0x2A];
    Overlay28Work *work;
    void **matrix;
    u8 pad6C[0x25];
    u8 suppressRelease;
};

typedef struct Overlay28Context {
    u8 pad00[0x16A];
    s16 intensity;
} Overlay28Context;

typedef struct Overlay28View {
    s16 pitch;
    s16 yaw;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay28View;

typedef struct Overlay28Transform {
    s16 pitch;
    s16 yaw;
    s16 roll;
    u16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay28Transform;

extern u8 gOverlay28DisplayList28[];
extern f32 ext_o0_2a470(s32 angle);
extern f32 ext_o0_2a46c(s32 angle);
extern void resetColourCycle(void *object);
extern void ext_o0_29e00();
extern void ext_o0_2b90(s32, f32, f32, f32, s32, void **);
extern void ext_o0_2d70(void *, f32, f32, f32);
extern void ext_o0_2c4c(void *, s32);
extern void func_80006EA0(Overlay28Owner *);
extern Overlay28View *ext_o0_241dc(void);
extern f32 ext_o0_6ec00(f32);
extern f32 ext_o0_9ab8(Overlay28Owner *);
extern void ext_o0_2409c(Overlay28Gfx **, void *, Overlay28Transform *, f32, f32);
extern void ext_o0_34554(Overlay28Gfx **, void *, s32, s32);
extern void ext_o0_241bc(Overlay28Gfx **);

void overlay28ResetBuffer();
void overlay28UpdateVertices(Overlay28VertexState *state);

#endif
