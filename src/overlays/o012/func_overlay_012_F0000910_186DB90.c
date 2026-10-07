#include "overlays/overlay_012.h"

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
extern void func_800349A4(Overlay12Gfx **displayList, void *resource,
                          s32 mode, s32 flags);
extern f32 func_80024938(f32 x, f32 y, f32 z);
extern f32 sqrtf(f32 value);
extern void func_800084C4(Overlay12Gfx **displayList,
                          Overlay12Vertex **vertices, void *resource,
                          void *base, Overlay12Effect *effect,
                          f32 *previous, f32 scale, u32 primary,
                          u32 secondary, s32 flags);
extern void func_80034DF0(u8 firstR, u8 firstG, u8 firstB,
                          u8 secondR, u8 secondG, u8 secondB);
extern void func_80023CCC(Overlay12Gfx **displayList, s32 *matrix,
                          Overlay12Vertex **vertices, void *resource,
                          s32 x, s32 y, s32 z, s32 arg7, f32 scale,
                          f32 arg9, f32 frame, s32 mode,
                          u8 alpha);
extern void func_80034E48(void);
extern u8 gOverlay12TrianglesA[];
extern u8 gOverlay12TrianglesB[];

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
#define OVERLAY12_EMIT(cursor, first, second) do { \
    Overlay12Gfx *command = (cursor)++; \
    command->w0 = (first); \
    command->w1 = (second); \
} while (0)

/*
 * JFG's bloodSpurtsDraw is the closest masked-skeleton sibling, but its
 * public source is GLOBAL_ASM. This body is reconstructed from Mickey only.
 */
/* Rewritten from the listing (lane c-o012, 2026-10-07): 569 -> 461
 * masked; lane h-8 (2026-10-07): 461 -> 429 masked, byte-exact rows 196 ->
 * 338. The colour words keep their stack homes and are masked into separate
 * locals; the volatile zero is stored after the three pointer loads and
 * never read, beside an unused s16 that puts it on the target's half-word;
 * declaration order reproduces the target's home ladder; the vertex packet
 * is the objects.c vertex/polygon command pair; the quad corners are written
 * through a walking pointer (the target's +0x1E base with negative
 * displacements); the collision vector is read through a pointer taken
 * before the clamp (the target's effect+0x2C base); the lifetime alpha is an
 * if/else; the secondary colour is one expression; in case 2 and the
 * particle loop the three colour PRODUCTS are s32 locals and the shift and
 * mask are written at the call (v1, t0, t1 as shipped). factor is set to
 * 2.0f before the distance call and the scaled distance added after the
 * clamp: the extra definition gives the 2.0f constant web the second
 * reference that ranks it above 1024.0f (f28 as shipped, 1.0f no longer
 * coloured), 429 at -8 -> 180 at delta 0. The four vertex colour writes
 * are one counted loop (IDO unrolls it; the loop weight puts the u8 255 in
 * s0 ahead of the effect pointer), 180 -> 122. */
#ifdef NON_MATCHING
void func_overlay_012_F0000910_186DB90(Overlay12Gfx **displayListPtr,
                                       s32 *matrixPtr,
                                       Overlay12Vertex **verticesPtr) {
    s32 i;
    s32 *color;
    s32 alpha;
    void *resource;
    s32 intensity;
    u32 primary;
    u32 secondary;
    u32 maskedPrimary;
    u32 maskedSecondary;
    s32 blue;
    s32 red;
    f32 previous[3];
    f32 distance;
    f32 factor;
    f32 centerX;
    f32 centerY;
    f32 centerZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    s32 green;
    s32 component;
    Overlay12Vertex *quad;
    Overlay12Vertex *vertices;
    Overlay12Effect *effect;
    Overlay12Particle *particle;
    f32 *collision;
    s16 unused2;
    volatile s16 unused;
    Overlay12Gfx *displayList;
    s32 matrix;
    s32 k;

    displayList = *displayListPtr;
    matrix = *matrixPtr;
    vertices = *verticesPtr;
    unused = 0;
    effect = gOverlay12Effects;
    for (i = 0; i < 64; i++, effect++) {
        if (effect->active != 0) {
            intensity = effect->scaleX;
            component = ((intensity * 255) >> 5) & 0xFF00;
            primary = component | (component << 8) | (component << 16) | 0xFF;
            color = &gOverlay12EffectColors[effect->type * 3];
            secondary = (((color[2] * intensity) >> 5) & 0xFF00) |
                        ((intensity * color[0] << 11) & 0xFF000000) |
                        ((intensity * color[1] * 8) & 0xFF0000) | 0xFF;
        }

        if (((effect->active == 2) || (effect->active == 3)) &&
            (effect->collided != 0)) {
            if (effect->lifetime < 120) {
                alpha = (effect->lifetime * 255) / 120;
            } else {
                alpha = 255;
            }
            resource = gOverlay12Resources[4 + effect->kind2];
            maskedPrimary = primary & ~0xFF;
            if (resource != NULL) {
                maskedSecondary = secondary & ~0xFF;
                func_800349A4(&displayList, resource, 0x203, 0);
                OVERLAY12_EMIT(displayList, 0xE7000000, 0);
                OVERLAY12_EMIT(displayList, 0xFA000000, maskedPrimary | alpha);
                OVERLAY12_EMIT(displayList, 0xFB000000, maskedSecondary | alpha);
#define VA ((u32)vertices + 0x80000000)
#define VB ((u32)vertices - 0x80000000)
                O12_GFX_VERTEX(displayList++, VA, VB, 4, 0);
                O12_GFX_POLYGON(displayList++, gOverlay12QuadTriangles + 0x80000000, 2, 1);

                factor = 2.0f;
                distance = -func_80024938(effect->x0, effect->y0, effect->z0);
                if (distance < 0.0f) {
                    distance = -distance;
                }
                distance -= 250.0f;
                if (distance < 0.0f) {
                    distance = 0.0f;
                }
                collision = &effect->collisionX;
                if (distance > 1024.0f) {
                    distance = 1024.0f;
                }
                factor += distance * 0.01f;
                centerX = collision[0] * factor + effect->x0;
                centerY = collision[1] * factor + effect->y0;
                centerZ = collision[2] * factor + effect->z0;

                quad = vertices;
                quad->x = (s16)(effect->vertexX0 + centerX);
                quad->y = (s16)(effect->vertexY0 + centerY);
                quad->z = (s16)(effect->vertexZ0 + centerZ);
                quad++;
                quad->x = (s16)(effect->vertexX1 + centerX);
                quad->y = (s16)(effect->vertexY1 + centerY);
                quad->z = (s16)(effect->vertexZ1 + centerZ);
                quad++;
                quad->x = (s16)(centerX - effect->vertexX1);
                quad->y = (s16)(centerY - effect->vertexY1);
                quad->z = (s16)(centerZ - effect->vertexZ1);
                quad++;
                quad->x = (s16)(centerX - effect->vertexX0);
                quad->y = (s16)(centerY - effect->vertexY0);
                quad->z = (s16)(centerZ - effect->vertexZ0);
                for (k = 0; k < 4; k++) {
                    vertices->r = 255;
                    vertices->g = 255;
                    vertices->b = 255;
                    vertices->a = 255;
                    vertices++;
                }
            }
        }

        switch (effect->active) {
        case 1:
            velocityX = effect->x2;
            velocityY = effect->y2;
            velocityZ = effect->z2;
            distance = sqrtf((velocityX * velocityX) +
                             (velocityY * velocityY) +
                             (velocityZ * velocityZ));
            if (distance == 0.0f) {
                factor = 0.0f;
            } else {
                factor = 40.0f / distance;
            }
            factor *= effect->value;
            previous[0] = effect->x0 - (velocityX * factor);
            previous[1] = effect->y0 - (velocityY * factor);
            previous[2] = effect->z0 - (velocityZ * factor);
            func_800084C4(&displayList, &vertices,
                          gOverlay12Resources[2 + effect->kind1],
                          effect->kind1 == 0 ? gOverlay12TrianglesA : gOverlay12TrianglesB,
                          effect, previous, effect->value * 8.0f,
                          primary, secondary, 0x200);
            break;
        case 2:
            color = &gOverlay12EffectColors[effect->type * 3];
            red = color[0] * intensity;
            green = color[1] * intensity;
            blue = color[2] * intensity;
            alpha = ((intensity * 255) >> 13) & 0xFF;
            func_80034DF0(alpha, alpha, alpha, (red >> 13) & 0xFF, (green >> 13) & 0xFF, (blue >> 13) & 0xFF);
            func_80023CCC(&displayList, &matrix, &vertices,
                          gOverlay12Resource5,
                          (s32)effect->x0, (s32)effect->y0, (s32)effect->z0,
                          0, effect->value * 1.8f,
                          1.0f, effect->zero, 14, 255);
            func_80034E48();
            break;
        }
    }

    particle = gOverlay12Particles;
    for (i = 0; i < 5; i++, particle++) {
        if (particle->active != 0) {
            color = &gOverlay12ParticleColors[particle->variant * 3];
            intensity = particle->type;
            red = color[0] * intensity;
            green = color[1] * intensity;
            blue = color[2] * intensity;
            alpha = ((intensity * 255) >> 8) & 0xFF;
            func_80034DF0(alpha, alpha, alpha, (red >> 8) & 0xFF, (green >> 8) & 0xFF, (blue >> 8) & 0xFF);
            func_80023CCC(&displayList, &matrix, &vertices,
                          gOverlay12Resource5,
                          (s32)particle->x, (s32)particle->y, (s32)particle->z,
                          0, 4.0f, 1.0f, particle->velocity, 14, 255);
        }
    }
    func_80034E48();
    OVERLAY12_EMIT(displayList, 0xFA000000, 0xFFFFFFFF);
    *displayListPtr = displayList;
    *matrixPtr = matrix;
    *verticesPtr = vertices;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o012/func_overlay_012_F0000910_186DB90/func_overlay_012_F0000910_186DB90.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_012_F0000910_186DB90:start
 * symbol: func_overlay_012_F0000910_186DB90
 * score: 122 differing words
 * frame: 0x148
 * relocations: 38
 * first-mismatch: +0xAC
 * summary: Vertex colour writes as an unrolled loop put the u8 255 in s0: 180 to 122 at 0. Open: FP f0/f2/f12 trio, case-1 velocity products.
 * PLATEAU-HANDOFF:func_overlay_012_F0000910_186DB90:end
 */
