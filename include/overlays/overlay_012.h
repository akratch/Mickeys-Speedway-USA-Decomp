#ifndef OVERLAY_012_H
#define OVERLAY_012_H

#include "PR/ultratypes.h"

typedef struct Overlay12Entry {
    u8 pad00[0x4A];
    u8 active;
    u8 pad4B[9];
} Overlay12Entry;

typedef struct Overlay12Effect {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 zero;
    f32 value;
    f32 collisionX;
    f32 collisionY;
    f32 collisionZ;
    f32 collisionValue;
    s16 lifetime;
    s16 vertexX0;
    s16 vertexY0;
    s16 vertexZ0;
    s16 vertexX1;
    s16 vertexY1;
    s16 vertexZ1;
    u8 active;
    u8 kind1;
    s16 scaleX;
    s16 scaleY;
    u8 type;
    u8 kind2;
    u8 collided;
    u8 pad53;
} Overlay12Effect;

typedef struct Overlay12Particle {
    f32 x;
    f32 y;
    f32 z;
    f32 velocity;
    u8 pad10[4];
    u8 type;
    u8 variant;
    u8 active;
    u8 pad17;
} Overlay12Particle;

extern void *gOverlay12ResourceF3;
extern void *gOverlay12ResourceF2;
extern void *gOverlay12ResourceF4;
extern void *gOverlay12ResourceF5;
extern void *gOverlay12ResourceF6;
extern void *gOverlay12Resource39;
extern void *gOverlay12Resource0;
extern void *gOverlay12Resource1;
extern void *gOverlay12Resource2;
extern void *gOverlay12Resource3;
extern void *gOverlay12Resource4;
extern void *gOverlay12Resource5;
extern Overlay12Entry gOverlay12Entries[];
extern Overlay12Effect gOverlay12Effects[64];
extern Overlay12Particle gOverlay12Particles[5];
extern u8 gOverlay12Flag1536;
extern s32 gOverlay12Ready;
extern s32 gOverlay12Count;
extern s32 gOverlay12Selection;
extern s32 gOverlay12Value1598;
extern s32 gOverlay12EffectCount;
extern s32 gOverlay12ParticleCount;

void *overlay12LoadReloc();
extern void texFreeTexture(void *resource);
extern void texFreeSprite(void *resource);
extern s32 overlay12Lookup(s32 first, s32 second);

void overlay12Initialize(void);

#endif
