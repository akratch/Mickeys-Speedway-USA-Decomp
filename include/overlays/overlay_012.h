#ifndef OVERLAY_012_H
#define OVERLAY_012_H

#include "PR/ultratypes.h"
#include "game/track.h"
#include "game/math.h"

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

typedef struct Overlay12TrackHeight {
    u8 pad00[0x24];
    s16 height;
} Overlay12TrackHeight;

typedef struct Overlay12Gfx {
    u32 w0;
    u32 w1;
} Overlay12Gfx;

typedef struct Overlay12Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay12Vertex;

extern void *gOverlay12Resources[];
extern s32 gOverlay12EffectColors[];
extern s32 gOverlay12ParticleColors[];
extern u8 gOverlay12QuadTriangles[];
extern u8 gOverlay12TrianglesA[];
extern u8 gOverlay12TrianglesB[];

struct TextureHeader;
struct Sprite;
extern struct TextureHeader *texLoadTexture(s32 textureId);
extern struct Sprite *texLoadSprite(s32 spriteId, s32 flags);
extern void texFreeTexture(struct TextureHeader *resource);
extern void texFreeSprite(struct Sprite *resource);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_8001291C(f32 *previous, f32 *current,
                         f32 *result, s32 mask, s32 flags);
extern f32 sqrtf(f32 value);
extern s32 texAnimateSprite(void *entry, s32 *mode, s32 animationId,
                            void *state, s32 updateRate);
extern void texDPTextureX(Overlay12Gfx **displayList, void *resource,
                          s32 mode, s32 flags);
extern f32 func_80024938(f32 x, f32 y, f32 z);
extern void func_800084C4(Overlay12Gfx **displayList,
                          Overlay12Vertex **vertices, void *resource,
                          void *base, Overlay12Effect *effect,
                          f32 *previous, f32 scale, u32 primary,
                          u32 secondary, s32 flags);
extern void sprSetIA2ColOverride(u8 firstR, u8 firstG, u8 firstB,
                                 u8 secondR, u8 secondG, u8 secondB);
extern void camDoSpriteDirect(Overlay12Gfx **displayList, void **matrix,
                              Overlay12Vertex **vertices, void *resource,
                              s32 x, s32 y, s32 z, s32 arg7, f32 scale,
                              f32 arg9, f32 frame, s32 mode,
                              u8 alpha);
extern void sprClearIA2ColOverride(void);

#define O12_SHIFTL(value, shift, width) ((u32)(((u32)(value) & ((1U << (width)) - 1U)) << (shift)))
#define O12_GFX_VERTEX(packet, addressA, addressB, count, first) { \
    Overlay12Gfx *_g = (Overlay12Gfx *)(packet); \
    _g->w0 = O12_SHIFTL(4, 24, 8) | \
             O12_SHIFTL(((count) << 3) | ((u32)(addressA) & 6) | (first), 16, 8) | \
             O12_SHIFTL(((count) << 3) + ((count) << 1) + 8, 0, 16); \
    _g->w1 = (u32)(addressB); \
}
#define O12_GFX_POLYGON(packet, address, count, textured) { \
    Overlay12Gfx *_g = (Overlay12Gfx *)(packet); \
    _g->w0 = O12_SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
             O12_SHIFTL(5, 24, 8) | O12_SHIFTL((count) * 16, 0, 16); \
    _g->w1 = (u32)(address); \
}
#define OVERLAY12_EMIT(cursor, first, second) { \
    Overlay12Gfx *_g = (Overlay12Gfx *)((cursor)++); \
    _g->w0 = (first); \
    _g->w1 = (second); \
}

#endif
