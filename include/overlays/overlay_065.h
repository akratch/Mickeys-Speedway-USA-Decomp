#ifndef OVERLAYS_OVERLAY_065_H
#define OVERLAYS_OVERLAY_065_H

#include "PR/ultratypes.h"
#include "n_audio/gbi.h"

#include "game/texture_header.h"

typedef struct Overlay65TrailRecord {
    f32 x[9];
    f32 y[9];
    f32 z[9];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 floorY;
    u8 active;
    u8 red;
    u8 green;
    u8 blue;
} Overlay65TrailRecord;

/* Tier A: triangle packet fields and the seven RGB entries share the module data layout. */
typedef struct O65Triangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} O65Triangle;

typedef struct O65Color {
    u8 red, green, blue;
} O65Color;

typedef struct O65Particle {
    s16 x;
    s16 y;
    s16 z;
    s16 floorY;
    s16 dx;
    s16 dy;
    s16 dz;
    s16 ddx;
    s16 ddy;
    s16 ddz;
    s16 angle;
    s16 angleStep;
    u8 r;
    u8 g;
    u8 b;
    u8 active;
} O65Particle;

typedef struct O65Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O65Vertex;

typedef struct O65Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O65Vec3f;

typedef struct O65Camera {
    s16 angle;
    u8 pad02[10];
    f32 x;
    f32 y;
    f32 z;
} O65Camera;

extern Overlay65TrailRecord D_0[50];
extern s16 D_1900;

void overlay65Initialize(void);
void overlay65UpdateParticles(Gfx **commandList, Mtx **matrixPtr, s32 ticks);
void overlay65Release(void);
void overlay65ResetSlots(void);
void func_overlay_065_F0000C38_18C4EA0(Gfx **commandPtr, Mtx **matrixPtr, s32 updateRate);
void func_overlay_065_F0001A14_18C5C7C(f32 baseX, f32 baseY, f32 baseZ);

#endif
