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
 * masked, aligned residual 492 -> 419, size delta 0 -> -8. The colour
 * words keep their stack homes and are masked into separate locals; the
 * volatile zero is stored after the three pointer loads and never read;
 * declaration order reproduces the target's home ladder; the vertex packet
 * is the objects.c vertex/polygon command pair, its two segment-base adds
 * spelt with different operators so they stay two constants; case 2 and the
 * particle loop compute the three colour bytes before the alpha byte; the
 * billboard's alpha parameter is a u8 (the 255 shares the vertex-byte web);
 * the polygon list is one of two .data triangle tables. Open: the FP
 * constant ranking (2.0f must outrank 1024.0f; see the shard). */
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
    u32 blueTerm;
    u32 redTerm;
    f32 previous[3];
    f32 distance;
    f32 factor;
    f32 centerX;
    f32 centerY;
    f32 centerZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u32 greenTerm;
    s32 component;
    Overlay12Vertex *quad;
    Overlay12Vertex *vertices;
    Overlay12Effect *effect;
    Overlay12Particle *particle;
    s32 pad0;
    volatile s16 unused;
    Overlay12Gfx *displayList;
    s32 matrix;
    s32 pad1;

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
            blueTerm = ((color[2] * intensity) >> 5) & 0xFF00;
            redTerm = (intensity * color[0] << 11) & 0xFF000000;
            greenTerm = (intensity * color[1] * 8) & 0xFF0000;
            secondary = blueTerm | redTerm | greenTerm | 0xFF;
        }

        if (((effect->active == 2) || (effect->active == 3)) &&
            (effect->collided != 0)) {
            alpha = 255;
            if (effect->lifetime < 120) {
                alpha = (effect->lifetime * 255) / 120;
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

                distance = -func_80024938(effect->x0, effect->y0, effect->z0);
                if (distance < 0.0f) {
                    distance = -distance;
                }
                distance -= 250.0f;
                if (distance < 0.0f) {
                    distance = 0.0f;
                }
                if (distance > 1024.0f) {
                    distance = 1024.0f;
                }
                factor = 2.0f + (distance * 0.01f);
                centerX = effect->collisionX * factor + effect->x0;
                centerY = effect->collisionY * factor + effect->y0;
                centerZ = effect->collisionZ * factor + effect->z0;

                quad = vertices;
                quad[0].x = (s16)(effect->vertexX0 + centerX);
                quad[0].y = (s16)(effect->vertexY0 + centerY);
                quad[0].z = (s16)(effect->vertexZ0 + centerZ);
                quad[1].x = (s16)(effect->vertexX1 + centerX);
                quad[1].y = (s16)(effect->vertexY1 + centerY);
                quad[1].z = (s16)(effect->vertexZ1 + centerZ);
                quad[2].x = (s16)(centerX - effect->vertexX1);
                quad[2].y = (s16)(centerY - effect->vertexY1);
                quad[2].z = (s16)(centerZ - effect->vertexZ1);
                quad[3].x = (s16)(centerX - effect->vertexX0);
                quad[3].y = (s16)(centerY - effect->vertexY0);
                quad[3].z = (s16)(centerZ - effect->vertexZ0);
                vertices->r = 255;
                vertices->g = 255;
                vertices->b = 255;
                vertices->a = 255;
                vertices++;
                vertices->r = 255;
                vertices->g = 255;
                vertices->b = 255;
                vertices->a = 255;
                vertices++;
                vertices->r = 255;
                vertices->g = 255;
                vertices->b = 255;
                vertices->a = 255;
                vertices++;
                vertices->r = 255;
                vertices->g = 255;
                vertices->b = 255;
                vertices->a = 255;
                vertices++;
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
            redTerm = ((color[0] * intensity) >> 13) & 0xFF;
            greenTerm = ((color[1] * intensity) >> 13) & 0xFF;
            blueTerm = ((color[2] * intensity) >> 13) & 0xFF;
            alpha = ((intensity * 255) >> 13) & 0xFF;
            func_80034DF0(alpha, alpha, alpha, redTerm, greenTerm, blueTerm);
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
            redTerm = ((color[0] * intensity) >> 8) & 0xFF;
            greenTerm = ((color[1] * intensity) >> 8) & 0xFF;
            blueTerm = ((color[2] * intensity) >> 8) & 0xFF;
            alpha = ((intensity * 255) >> 8) & 0xFF;
            func_80034DF0(alpha, alpha, alpha, redTerm, greenTerm, blueTerm);
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
 * score: 461 differing words
 * frame: 0x148
 * relocations: 38
 * first-mismatch: +0x54
 * summary: Natural rewrite from the listing 569 to 461 at -8. Open: FP constant ranking, 2.0f must outrank 1024.0f.
 * PLATEAU-HANDOFF:func_overlay_012_F0000910_186DB90:end
 */
