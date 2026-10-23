#include "overlays/overlay_012.h"

/*
 * Overlay 12: impact effects and particles. One translation unit;
 * overlay12Initialize is unexported and static.
 */

/* Overlay 12, ADR 0006 consolidation: loop-unroll-disabled prefix. */

/* DKR v77/v80 and JFG contain no exact donor for this resource initializer. */
static void overlay12Initialize(void) {
    s32 i;
    Overlay12Entry *entry;

    gOverlay12ResourceF3 = overlay12LoadReloc(0xF3);
    gOverlay12ResourceF2 = overlay12LoadReloc(0xF2);
    gOverlay12ResourceF4 = overlay12LoadReloc(0xF4);
    gOverlay12ResourceF5 = overlay12LoadReloc(0xF5);
    gOverlay12ResourceF6 = overlay12LoadReloc(0xF6);
    gOverlay12Resource39 = overlay12LoadReloc(0x39, 1);

    for (i = 0, entry = gOverlay12Entries; i < 64; i++, entry++) {
        entry->active = 0;
    }

    gOverlay12Flag1536 = 0;
    gOverlay12Ready = 1;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
    /* The zero goes through the loop counter: a literal store loads it in a
     * different order (4 words). */
    i = 0;
    gOverlay12Value1598 = i;
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor for this release path. */
void func_overlay_012_F00000C4_186D344(void) {
    s32 i;
    Overlay12Entry *entry;

    if (gOverlay12Ready != 0) {
        if (gOverlay12Resource0 != 0) {
            texFreeTexture(gOverlay12Resource0);
        }
        if (gOverlay12Resource1 != 0) {
            texFreeTexture(gOverlay12Resource1);
        }
        if (gOverlay12Resource2 != 0) {
            texFreeTexture(gOverlay12Resource2);
        }
        if (gOverlay12Resource3 != 0) {
            texFreeTexture(gOverlay12Resource3);
        }
        if (gOverlay12Resource4 != 0) {
            texFreeTexture(gOverlay12Resource4);
        }
        if (gOverlay12Resource5 != 0) {
            texFreeSprite(gOverlay12Resource5);
        }

        entry = gOverlay12Entries;
        i = 0;
        while (1) {
            i++;
            entry++;
            entry[-1].active = 0;
            if (i < 64) {
                continue;
            }
            break;
        }
        /* Inert allocation aid retained by the exact C; tracked in
         * docs/cleanup-queue.md. */
        if (!i) {
        }
        gOverlay12Flag1536 = 0;
    }

    gOverlay12Ready = 0;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
}

/* Overlay 12, ADR 0006 consolidation: default-O2 tail. */

void func_overlay_012_F00001B4_186D434(f32 x, f32 y, f32 z, f32 x2, f32 y2,
                                       f32 z2, s32 scale, s32 type, f32 value) {
    Overlay12Effect *effect;
    s32 i;

    if (gOverlay12Ready == 0) {
        overlay12Initialize();
    }
    if (gOverlay12EffectCount < 64) {
        effect = gOverlay12Effects;
        for (i = 0; i < 64; i++, effect++) {
            if (effect->active == 0) {
                break;
            }
        }
        if (i < 64) {
            effect->x2 = x2;
            effect->y2 = y2;
            effect->z2 = z2;
            effect->value = value;
            effect->active = 1;
            effect->lifetime = 300;
            effect->x1 = x;
            effect->y1 = y;
            effect->zero = 0.0f;
            effect->z1 = z;
            effect->x0 = x;
            effect->y0 = y;
            effect->z0 = z;
            effect->kind1 = mathRnd(0, 1);
            effect->scaleX = scale << 5;
            effect->scaleY = scale << 5;
            effect->type = type;
            effect->kind2 = mathRnd(0, 2);
            gOverlay12EffectCount++;
        }
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no donor. */
void func_overlay_012_F00002E4_186D564(f32 x, f32 y, f32 z, s32 type,
                                       s32 variant) {
    s32 i;
    Overlay12Particle *particle;

    if (gOverlay12Ready == 0) {
        overlay12Initialize();
    }
    if (gOverlay12ParticleCount < 5) {
        particle = gOverlay12Particles;
        for (i = 0; i < 5; i++, particle++) {
            if (particle->active == 0) {
                break;
            }
        }
        if (i < 5) {
            particle->x = x;
            particle->y = y;
            particle->z = z;
            particle->type = type;
            particle->variant = variant;
            particle->active = 1;
            particle->velocity = 0.0f;
            gOverlay12ParticleCount++;
        }
    }
}

/*
 * JFG's bloodSpurtUpdateAll is the closest masked-skeleton sibling, but its
 * public source is GLOBAL_ASM. This body is reconstructed from Mickey only.
 *
 * Matched 159 -> 0 (lane w2-ovle, 2026-10-02) by discarding the inherited
 * case-1 carriers: y0 and y2 are updated in place with no tempA or gRate
 * local (y2 is then one CSE web in f0, and the rate keeps one web that is
 * spilled to its own home across the collision calls), the three physics
 * constants are literals in this TU's pool, the root sums the x term first,
 * collided is set after both vertices, the particle loop reads its flag
 * into a local of its own (the lowest frame home) and both loops initialise
 * their walking pointer in the for-init.
 */
void func_overlay_012_F00003A8_186D628(s32 updateRate) {
    /*
     * PROVENANCE: JFG's public src/camlight.c uses an output buffer for
     * trackNearestIntersection; Mickey's accesses are indices 0-9.
     * Frame homes follow declaration order, later declarations lower: the
     * mode word sits directly under the buffer (+0xA8 under +0xAC) when it is
     * declared after it, and the late-declared pointer, counter, angles and
     * particle flag fill the rest below it.
     */
    Overlay12TrackHeight *track;
    Overlay12Effect *effect;
    f32 updateRateF = (f32)updateRate;
    f32 minimumHeight;
    s32 pad0;
    f32 tempA;
    f32 tempB;
    f32 tempC;
    f32 xBasis;
    f32 sineRandom;
    f32 cosineRandom;
    f32 sinePitch;
    f32 cosinePitch;
    f32 sineYaw;
    f32 cosineYaw;
    f32 collision[10];
    s32 mode = 1;
    Overlay12Particle *particle;
    s32 i;
    s16 pitch;
    s16 yaw;
    s16 randomAngle;
    s32 active;

    track = (Overlay12TrackHeight *)trackGetTrack();
    minimumHeight = (f32)track->height - 1000.0f;
    for (i = 0, effect = gOverlay12Effects; i < 64; i++, effect++) {
        switch (effect->active) {
        case 1:
            effect->y0 += (effect->y2 * updateRateF) +
                          (-0.05f * updateRateF * updateRateF);
            if (effect->y0 < minimumHeight) {
                effect->active = 0;
                gOverlay12EffectCount--;
            } else {
                effect->x0 += effect->x2 * updateRateF;
                effect->z0 += effect->z2 * updateRateF;
                effect->y2 += -0.1f * updateRateF;
                if ((((i & 1) != 0) && (gOverlay12Value1598 != 0)) ||
                    (((i & 1) == 0) && (gOverlay12Value1598 == 0))) {
                    if (func_8001291C(&effect->x1, &effect->x0, collision,
                                      0x10000, 0) != 0) {
                        effect->collisionX = collision[4];
                        effect->collisionY = collision[5];
                        effect->collisionZ = collision[6];
                        effect->collisionValue = collision[7];
                        effect->x0 = collision[1];
                        effect->y0 = collision[2];
                        effect->z0 = collision[3];
                        effect->scaleY =
                            (255 - (((((u32 *)collision)[9] >> 24) & 7) << 5)) << 5;
                        effect->value *=
                            2.0f + ((f32)(mathRnd(0, 255) - 128) * 0.0078f);

                        if (((s32 *)collision)[0] == 0) {
                            pitch = Arctanf(collision[5],
                                            sqrtf((collision[4] * collision[4]) +
                                                  (collision[6] * collision[6]))) -
                                    0x4000;
                            yaw = Arctanf(-collision[4], -collision[6]);
                            randomAngle = (s16)mathRnd(-0x8000, 0x7FFF);
                            sineRandom = func_8002A8BC(randomAngle);
                            cosineRandom = func_8002A8C0(randomAngle);
                            sinePitch = func_8002A8BC(-pitch);
                            cosinePitch = func_8002A8C0(-pitch);
                            sineYaw = func_8002A8BC(yaw);
                            cosineYaw = func_8002A8C0(yaw);

                            tempA = ((-10.0f * sineRandom) +
                                     (10.0f * cosineRandom)) * effect->value;
                            tempB = ((10.0f * sineRandom) -
                                     (-10.0f * cosineRandom)) * effect->value;
                            tempC = tempB * sinePitch;
                            xBasis = tempB * cosinePitch;
                            effect->vertexY0 = (s16)xBasis;
                            effect->vertexX0 =
                                (s16)((tempA * sineYaw) + (tempC * cosineYaw));
                            effect->vertexZ0 =
                                (s16)((tempC * sineYaw) - (tempA * cosineYaw));

                            tempA = ((10.0f * sineRandom) +
                                     (10.0f * cosineRandom)) * effect->value;
                            tempB = ((10.0f * sineRandom) -
                                     (10.0f * cosineRandom)) * effect->value;
                            tempC = tempB * sinePitch;
                            xBasis = tempB * cosinePitch;
                            effect->vertexY1 = (s16)xBasis;
                            effect->vertexX1 =
                                (s16)((tempA * sineYaw) + (tempC * cosineYaw));
                            effect->vertexZ1 =
                                (s16)((tempC * sineYaw) - (tempA * cosineYaw));
                            effect->collided = 1;
                        } else {
                            effect->collided = 0;
                        }
                        effect->active = 2;
                    } else {
                        effect->x1 = effect->x0;
                        effect->y1 = effect->y0;
                        effect->z1 = effect->z0;
                    }
                }
            }
            break;
        case 2:
            effect->scaleX += (effect->scaleY - effect->scaleX) >> 1;
            if (texAnimateSprite(gOverlay12Resource5, &mode, 12,
                              &effect->zero, updateRate) != 0) {
                effect->active = 3;
            }
            break;
        case 3:
            effect->lifetime -= updateRate;
            if (effect->lifetime < 0) {
                effect->active = 0;
                gOverlay12EffectCount--;
            }
            break;
        }
    }

    gOverlay12Value1598 ^= 1;
    mode = 1;
    for (i = 0, particle = gOverlay12Particles; i < 5; i++, particle++) {
        active = particle->active;
        if ((active == 0) == FALSE) {
            if (texAnimateSprite(gOverlay12Resource5, &mode, 15,
                              &particle->velocity, updateRate) != 0) {
                particle->active = 0;
                gOverlay12ParticleCount--;
            }
        }
    }
}

/*
 * JFG's bloodSpurtsDraw is the closest masked-skeleton sibling, but its
 * public source is GLOBAL_ASM. This body is reconstructed from Mickey only.
 */
/*
 * Matched 2026-10-07 (lane i-2) after rewrites by lanes c-o012 and h-8.
 * Reconstructed from the listing: the GBI-style packet macros are bare
 * braces (a do/while(0) form cut the resource block into extra uopt
 * blocks); the vertex colour writes are one counted loop IDO unrolls;
 * factor is set to 2.0f before the distance call and the scaled distance
 * added after the clamp; case 1 scales the velocities in place through
 * distance; the masked colour words are s32 and the primary colour's first
 * term is cast to u32 (operand order of the ors); the case-2 and particle
 * colour components are masked into their locals before the call; both
 * loops initialise their cursor in the for-init.
 */
void func_overlay_012_F0000910_186DB90(Overlay12Gfx **displayListPtr,
                                       void **matrixPtr,
                                       Overlay12Vertex **verticesPtr) {
    s32 i;
    s32 *color;
    s32 alpha;
    void *resource;
    s32 intensity;
    u32 primary;
    u32 secondary;
    s32 maskedPrimary;
    s32 maskedSecondary;
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
    void *matrix;
    s32 k;

    displayList = *displayListPtr;
    matrix = *matrixPtr;
    vertices = *verticesPtr;
    unused = 0;
    for (i = 0, effect = gOverlay12Effects; i < 64; i++, effect++) {
        if (effect->active != 0) {
            intensity = effect->scaleX;
            component = ((intensity * 255) >> 5) & 0xFF00;
            primary = (u32)component | (component << 8) | (component << 16) | 0xFF;
            color = &gOverlay12EffectColors[effect->type * 3];
            secondary = ((intensity * color[0] << 11) & 0xFF000000) |
                        ((intensity * color[1] * 8) & 0xFF0000) |
                        (((color[2] * intensity) >> 5) & 0xFF00) | 0xFF;
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
                texDPTextureX(&displayList, resource, 0x203, 0);
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
                distance = 0.0f;
            } else {
                distance = 40.0f / distance;
            }
            distance *= effect->value;
            velocityX *= distance;
            velocityY *= distance;
            velocityZ *= distance;
            previous[0] = effect->x0 - velocityX;
            previous[1] = effect->y0 - velocityY;
            previous[2] = effect->z0 - velocityZ;
            func_800084C4(&displayList, &vertices,
                          gOverlay12Resources[2 + effect->kind1],
                          effect->kind1 == 0 ? gOverlay12TrianglesA : gOverlay12TrianglesB,
                          effect, previous, effect->value * 8.0f,
                          primary, secondary, 0x200);
            break;
        case 2:
            color = &gOverlay12EffectColors[effect->type * 3];
            red = ((color[0] * intensity) >> 13) & 0xFF;
            green = ((color[1] * intensity) >> 13) & 0xFF;
            blue = ((color[2] * intensity) >> 13) & 0xFF;
            alpha = ((intensity * 255) >> 13) & 0xFF;
            sprSetIA2ColOverride(alpha, alpha, alpha, red, green, blue);
            camDoSpriteDirect(&displayList, &matrix, &vertices,
                          gOverlay12Resource5,
                          (s32)effect->x0, (s32)effect->y0, (s32)effect->z0,
                          0, effect->value * 1.8f,
                          1.0f, effect->zero, 14, 255);
            sprClearIA2ColOverride();
            break;
        }
    }

    for (i = 0, particle = gOverlay12Particles; i < 5; i++, particle++) {
        if (particle->active != 0) {
            color = &gOverlay12ParticleColors[particle->variant * 3];
            intensity = particle->type;
            red = ((color[0] * intensity) >> 8) & 0xFF;
            green = ((color[1] * intensity) >> 8) & 0xFF;
            blue = ((color[2] * intensity) >> 8) & 0xFF;
            alpha = ((intensity * 255) >> 8) & 0xFF;
            sprSetIA2ColOverride(alpha, alpha, alpha, red, green, blue);
            camDoSpriteDirect(&displayList, &matrix, &vertices,
                          gOverlay12Resource5,
                          (s32)particle->x, (s32)particle->y, (s32)particle->z,
                          0, 4.0f, 1.0f, particle->velocity, 14, 255);
        }
    }
    sprClearIA2ColOverride();
    OVERLAY12_EMIT(displayList, 0xFA000000, 0xFFFFFFFF);
    *displayListPtr = displayList;
    *matrixPtr = matrix;
    *verticesPtr = vertices;
}
