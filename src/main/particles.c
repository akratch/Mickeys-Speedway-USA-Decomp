/*
 * Resident particle system -- ROM 0x3D5F0-0x43470.
 *
 * PROVENANCE: The translation-unit attribution and the non-placeholder names
 * below come from Jet Force Gemini's public decompilation, src/particles.c and
 * its built particles.c.o. Mickey's own function order, masked-skeleton
 * similarity, and call graph establish the correspondence. JFG address-based
 * placeholders are not imported; Mickey's existing placeholders remain.
 * Adapted bodies carry a point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "game/particles.h"

void mmFree(void *ptr);
void func_800347A0(void *resource);
void func_800359D4(void *resource);
void modFreeModel(void *resource);
void func_80036544(void *texture, void *state, s32 speed, f32 *frame, s32 updateRate);
void mathOneFloatPY(void *rotation, void *vector);
void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
void *piRomLoad(s32 assetId);
ParticleTexture *func_80034448(s32 resourceId);
ParticleSprite *func_800355A0(s32 resourceId, s32 arg1);
void *func_8001F520(s32 resourceId, s32 arg1);
s32 mathRnd(s32 minimum, s32 maximum);
f32 sqrtf(f32 value);
void mtxf_transform_dir(void *matrix, f32 *input, f32 *output, ParticleEmitterHeader *header);
void camSetNo(s32 camera);
void func_800221E8(void **dList, s32 arg1);
void camPushModelMtx(Gfx **dList, s32 renderContext, ParticleRenderTransform *transform, f32 scale, f32 extra);
void camRestoreModelMtx(Gfx **dList);
void func_80023A08(Gfx **dList, s32 renderContext, void **vertices, CircularParticle *particle,
                   ParticleSpriteResource *resource, s32 flags, s32 intensity);
void func_80034DF0(u8 red, u8 green, u8 blue, u8 alternateRed, u8 alternateGreen, u8 alternateBlue);
void func_80034E48(void);
void func_800349A4(Gfx **dList, void *texture, s32 mode, s32 flags);
struct ParticleRenderGroup;
void func_8003D4FC(void **dList, void **vertices, struct ParticleRenderGroup *group);
f32 func_8002A8BC(s16 angle);
f32 func_8002A8C0(s16 angle);
s32 func_8003CE10(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool, s32 mode);
void func_8003D25C(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool);
void func_8003F154(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config);
void func_8003F5F8(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config);
void func_80041CE4(Gfx **dList, ParticleLineVertex **vertices);
void func_80041F48(s32 arg0, ParticleTrigger *trigger);
s32 func_80040878(CircularParticle *particle, s32 updateRate);
CircularParticle *func_8004054C(s32 type, s32 direction);
void func_80040B88(ParticleEmitterObject *object, ParticleTriggerSlot *trigger);
void func_80041040(ParticleLineEntry *particle, s32 updateRate);
void func_80041388(ParticleModelEntry *particle, s32 updateRate);
void func_800367A4(void *texture, void *state, s16 speed, f32 *frame, s32 updateRate);
void func_8003EC8C(ParticleObject *object, s32 index);
void func_8003E7B8(ParticleObject *object, s32 index);
void func_8003EF80(ParticleObject *object, ParticleTriggerSlot *trigger);
s32 func_8003E8D8(ParticleTypeDescriptor *descriptor, ParticleConfig *config, ParticleTriggerSlot *trigger);
s32 func_8003EB08(ParticleTypeDescriptor *descriptor, ParticleConfig *config);
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
void func_8003CA20(void);
void func_8003CB3C(void);
void func_8003CD28(ParticleResourceList **listPtr);
void func_8003CCE4(void);
CircularParticle *func_8003FB98(ParticleEmitterObject *object, ParticleTrigger *trigger, s32 resourceId);
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry);
void func_80041FEC(BasicParticle *particle);
void func_80040740(CircularParticle *particle);
void func_800420E0(BasicParticle *particle);
void func_800421F4(BasicParticle *particle);
void func_8004233C(BasicParticle *particle);
void func_800423EC(BasicParticle *particle);

/* PROVENANCE: body adapted from DKR src/particles.c:reset_particles. */
void reset_particles(void) {
    func_8003CB3C();
    func_8003CCE4();
    func_8003CA20();
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005D930.s; body reconstructed from Mickey evidence. */
void func_8003CA20(void) {
    s32 pad;
    s16 *types;
    s16 *typePtr;
    s32 i;
    s32 count;

    if (D_8007C884 != NULL) {
        types = piRomLoad(0x3A);
        count = D_8007C888;
        i = 0;
        typePtr = types;
        if (count > 0) {
            do {
                switch (*typePtr & 0xC000) {
                    case 0x8000:
                        func_800359D4(D_8007C884[i]);
                        break;
                    case 0xC000:
                        func_800347A0(D_8007C884[i]);
                        break;
                    case 0:
                        modFreeModel(D_8007C884[i]);
                        break;
                }
                i++;
                typePtr++;
                count = D_8007C888;
            } while (i < count);
        }
        mmFree(types);
        mmFree(D_8007C884);
        D_8007C884 = NULL;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005DA70.s; body reconstructed from Mickey evidence. */
void func_8003CB3C(void) {
    CircularParticlePool **pool;
    CircularParticlePool **poolEnd;
    ParticleLineEntry *line;
    ParticleModelEntry *model;
    s32 i;

    pool = D_800D4120, poolEnd = (CircularParticlePool **)&D_800D4134;
    do {
        func_8003CD28((ParticleResourceList **)pool);
        pool++;
    } while (pool < poolEnd);

    D_8007C8C4 = 0;
    if (D_8007C894 != NULL) {
        line = D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active && line->texture != NULL) {
                    func_800347A0(line->texture);
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
        mmFree(D_8007C894);
        D_8007C894 = NULL;
    }

    if (D_8007C898 != NULL) {
        model = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (model->active && model->resource != NULL) {
                    func_800347A0(model->resource);
                }
                i++;
                model++;
            } while (i < D_8007C890);
        }
        mmFree(D_8007C898);
        D_8007C898 = NULL;
    }

    if (D_8007CA60 != NULL) {
        mmFree(D_8007CA60);
        D_8007CA60 = NULL;
    }
    if (D_8007CA98 != NULL) {
        mmFree(D_8007CA98);
        D_8007CA98 = NULL;
    }
    if (D_8007C8BC != NULL) {
        mmFree(D_8007C8BC);
        D_8007C8BC = NULL;
        D_8007C8C0 = 0;
    }
}
void func_8003CCE4(void) {
    if (D_8007C89C[0] != NULL) {
        mmFree(D_8007C89C[0]);
        D_8007C89C[0] = NULL;
        D_8007C89C[1] = NULL;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005DC7C.s; body reconstructed from Mickey evidence. */
void func_8003CD28(ParticleResourceList **listPtr) {
    ParticleResourceList *list;
    ParticleResourceEntry *entry;
    s32 i;

    list = *listPtr;
    if (list != NULL) {
        entry = list->entries;
        i = 0;
        if (list->count > 0) {
            do {
                switch (entry->type) {
                    case 2:
                        func_800359D4(entry->resource);
                        break;
                    case 0:
                    case 1:
                        func_800347A0(entry->resource);
                        break;
                    case 3:
                        modFreeModel(entry->resource);
                        break;
                }
                i++;
                entry++;
            } while (i < list->count);
        }
        mmFree(list);
        *listPtr = NULL;
    }
}
/* PROVENANCE: structure cross-checked against JFG's assembly-only
 * asm/nonmatchings/particles/func_8005DD88.s sibling; body reconstructed
 * from Mickey evidence. */
s32 func_8003CE10(Gfx **dList, s32 renderContext, void **vertices, CircularParticlePool *pool, s32 mode) {
    s32 i;
    s32 blue;
    s32 green;
    s32 red;
    s32 alternateRed;
    s32 alternateGreen;
    s32 alternateBlue;
    s32 intensity;
    s32 renderFlags;
    s32 fade;
    s32 flags;
    s32 count;
    s32 requiredFlags;
    s32 excludedFlags;
    void *firstVertex;
    s32 colorMode;
    CircularParticle *particle;
    ParticleSpriteResource *resource;
    Gfx *command;

    if (pool == NULL) {
        return 0;
    }

    particle = pool->particles;
    i = 0;
    if (mode == 1) {
        requiredFlags = 0;
        excludedFlags = 0x8000;
    } else if (mode == 0) {
        requiredFlags = 0x8000;
        excludedFlags = 0;
    } else {
        requiredFlags = 0;
        excludedFlags = 0;
    }
    firstVertex = *vertices;
    if (pool->count > 0) {
        do {
            if (particle->type == 2) {
                flags = particle->flags;
                if ((flags & requiredFlags) == requiredFlags && !(flags & excludedFlags)) {
                    renderFlags = 2;
                    colorMode = particle->colorMode;
                    resource = particle->resource;
                    red = particle->red;
                    green = particle->green;
                    blue = particle->blue;
                    if (colorMode != 0) {
                        alternateRed = particle->alternateRed;
                        alternateGreen = particle->alternateGreen;
                        alternateBlue = particle->alternateBlue;
                    }
                    intensity = (particle->intensity >> 8) & 0xFF;
                    if (D_8007C854 != 0) {
                        if (flags & 0x800) {
                            fade = (particle->alpha * D_8007C85C) >> 8;
                        } else {
                            fade = D_8007C85C;
                        }
                    } else {
                        if (flags & 0x800) {
                            fade = particle->alpha;
                        } else {
                            fade = 0xFF;
                        }
                    }
                    if (fade < 0xFE) {
                        red = (red * fade) >> 8;
                        green = (green * fade) >> 8;
                        blue = (blue * fade) >> 8;
                        if (colorMode != 0) {
                            alternateRed = (alternateRed * fade) >> 8;
                            alternateGreen = (alternateGreen * fade) >> 8;
                            alternateBlue = (alternateBlue * fade) >> 8;
                        }
                    }
                    if (resource->flags & 0x200) {
                        if (colorMode == 0) {
                            alternateRed = (resource->alternateRed * red) >> 8;
                            alternateGreen = (resource->alternateGreen * green) >> 8;
                            alternateBlue = (resource->alternateBlue * blue) >> 8;
                            red = (resource->red * red) >> 8;
                            green = (resource->green * green) >> 8;
                            blue = (resource->blue * blue) >> 8;
                        }
                        if (colorMode != 2) {
                            func_80034DF0(red, green, blue, alternateRed, alternateGreen, alternateBlue);
                        } else {
                            func_80034DF0(alternateRed, alternateGreen, alternateBlue, red, green, blue);
                        }
                    } else if ((red << 24 | green << 16 | blue << 8 | intensity) != 0) {
                        gDPPipeSync((*dList)++);
                        command = (*dList)++;
                        command->words.w0 = 0xFA000000;
                        command->words.w1 = (red & 0xFFFFU) << 24 | (green & 0xFF) << 16 |
                                            (blue & 0xFF) << 8 | (intensity & 0xFF);
                    }
                    if (intensity != 0xFF) {
                        renderFlags = 6;
                    }
                    func_80023A08(dList, renderContext, vertices, particle, resource, renderFlags, intensity);
                    if (resource->flags & 0x200) {
                        func_80034E48();
                    }
                }
            }
            i++;
            particle++;
        } while (i < pool->count);
    }
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor((*dList)++, 0xFF, 0xFF, 0xFF, 0);
    return ((u8 *)*vertices - (u8 *)firstVertex) / 10;
}
/* Workbench: allocation-mismatch, 68 differing words, size_delta 0; first mismatch +0x50.
 * Target and candidate are 168 instructions with the exact frame and relocation surface.
 * Forced-color-oracle lever 19 needs an authenticated globalcolor trace; assembly fallback stays canonical. */
#ifdef NON_MATCHING
/*
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * func_8005E100 sibling; body reconstructed from Mickey evidence and the
 * resident render-command pattern in src/main/menu.c.
 */
void func_8003D25C(Gfx **dList, s32 renderContext, void **vertices, CircularParticlePool *pool) {
    ParticleRenderTransform transform;
    CircularParticle *particle;
    ParticleRenderResource *resource;
    Gfx *command;
    s32 count;
    s32 i;
    s32 color;
    u8 intensity;
    u8 red;
    u8 green;
    u8 blue;

    if (pool != NULL) {
        particle = pool->particles;
        gDPPipeSync((*dList)++);
        i = 0;
        count = pool->count;
        if (count > 0) {
            do {
                if (particle->type == 3) {
                    resource = particle->resource;
                    transform.rotationZ = particle->rotationZ;
                    transform.rotationX = particle->rotationX;
                    transform.rotationY = particle->rotationY;
                    transform.x = particle->renderX;
                    transform.y = particle->renderY;
                    transform.z = particle->renderZ;
                    transform.scale = particle->scale;
                    camPushModelMtx(dList, renderContext, &transform, 1.0f, 0.0f);
                    gDPPipeSync((*dList)++);
                    if (particle->flags & 0x800) {
                        intensity = particle->alpha;
                        red = particle->red;
                        green = particle->green;
                        blue = particle->blue;
                        gDPSetPrimColor((*dList)++, 0, 0, (red * intensity) >> 8, (green * intensity) >> 8,
                                        (blue * intensity) >> 8, 0xFF);
                    } else {
                        gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    command = (*dList)++;
                    command->words.w0 = (((s32)resource->triangles[resource->triangleIndex] + 0x80000000) &
                                         0xFFFFFF) |
                                        0xBF000000;
                    command->words.w1 = (s32)resource->vertices + 0x80000000;
                    gSPDisplayList((*dList)++, (s32)resource->header->displayList + 0x80000000);
                    gSP1Triangle((*dList)++, 0, 0, 0, 0);
                    count = pool->count;
                }
                i++;
                particle++;
            } while (i < count);
        }
        gDPPipeSync((*dList)++);
        gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        camRestoreModelMtx(dList);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D25C.s")
#endif
typedef struct ParticleRenderDescriptor {
    s16 rotation0;
    s16 rotation1;
    s16 rotation2;
    s16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 textureScale;
    s16 type;
    u8 pad2E[0x1E];
    void *texture;
    u32 flags;
    u32 geometryFlags;
    u8 pad58[0x0C];
    s16 alphaWord;
    u8 pad66[9];
    u8 intensity;
    u8 pad70;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad74[4];
} ParticleRenderDescriptor;

typedef struct ParticleRenderGroup {
    u8 pad00[4];
    s32 pointCount;
    s32 updateRate;
    s16 *points;
    u8 pad10[4];
    ParticleRenderDescriptor *entries;
    s32 entryCount;
    s32 active;
    u8 pad20[0x0C];
    s32 materialDefault;
    s32 materialOpaque;
    s32 materialTranslucent;
} ParticleRenderGroup;

/*
 * PROVENANCE: the particle-buffer grouping and rotated-point idioms were
 * cross-checked against Jet Force Gemini's public particles.c assembly;
 * Mickey's descriptor fields, globals, thresholds, and call sequence remain
 * authoritative for this reconstruction.
 */
/* Reconstructed from Mickey's call, field, and control-flow evidence.
 * Matches: 1068/1068 words, frame 0x138, all 14 relocation tuples exact.
 * The texture/scale carry-over is written as three statements -- the call,
 * then the two carrier assignments. Folding either assignment into the call's
 * argument list (the m2c spelling) costs 73 register-only words from +0x2D0:
 * the embedded assignment keeps one extra value live across the call and
 * rotates ugen's integer temp ring by one for the rest of the function. */
void func_8003D4FC(void **dListArg, void **verticesArg, ParticleRenderGroup *group) {
    s32 vertexCount;
    s32 primitiveCount;
    s32 index;
    s32 scanIndex;
    s32 emitted;
    s32 red;
    s32 green;
    s32 blue;
    s32 vertexAlpha;
    s32 pointCount;
    s32 updateRate;
    f32 xScaled;
    f32 yScaled;
    f32 rotatedZ;
    f32 sine1;
    f32 cosine1;
    f32 sine0;
    f32 point[3];
    f32 cosine0;
    f32 scale;
    s32 vertexAddress;
    s32 textureType;
    f32 previousScale;
    ParticleRenderDescriptor *entry;
    ParticleVertex *vertices;
    s16 colorAlpha;
    s16 currentAlpha;
    Gfx *displayList;
    void *texture;
    void *previousTexture;
    ParticleVertex *vertexStart;
    s32 material;
    s16 *points;

    currentAlpha = 0xFF;
    previousTexture = NULL;
    previousScale = 0.0f;
    if ((group != NULL) && (group->active != 0)) {
        scanIndex = 0;
        displayList = (Gfx *)*dListArg;
        vertices = (ParticleVertex *)*verticesArg;
        entry = group->entries;
        while ((scanIndex < group->entryCount) && (entry->type == 0x80)) {
            scanIndex++;
            entry++;
        }

        updateRate = group->updateRate;
        pointCount = group->pointCount;
        func_800349A4(&displayList, NULL, 0x12, 0);
        material = group->materialDefault;
        gDPSetPrimColor(displayList++, 0, 0, 255, 255, 255, 255);

        if (scanIndex < group->entryCount) {
            index = scanIndex;
            texture = entry->texture;
            scale = entry->textureScale;
            colorAlpha = (entry->alphaWord >> 8) & 0xFF;
            vertexStart = vertices;
            vertexCount = 0;
            primitiveCount = 0;
            while (index < group->entryCount) {
                if ((entry->type == 0x80) || (texture != entry->texture) ||
                    (scale != entry->textureScale) ||
                    (vertexCount + pointCount >= 0x18) ||
                    (colorAlpha != ((entry->alphaWord >> 8) & 0xFF))) {
                    vertexAddress = (s32)vertexStart + 0x80000000;
                    if (colorAlpha != currentAlpha) {
                        gDPPipeSync(displayList++);
                        gDPSetPrimColor(displayList++, 0, 0, 255, 255, 255, colorAlpha);
                        currentAlpha = colorAlpha;
                    }
                    if ((texture != previousTexture) || (scale != previousScale)) {
                        func_800349A4(&displayList, texture, 0x12, (s32)(scale * 65536.0f));
                        previousTexture = texture;
                        previousScale = scale;
                    }
                    if (texture != NULL) {
                        textureType = *(u16 *)((u8 *)texture + 6);
                        if (textureType != 0x20) {
                            if (textureType != 0x40) {
                                material = group->materialTranslucent;
                            } else {
                                material = group->materialDefault;
                            }
                        } else {
                            material = group->materialOpaque;
                        }
                    }
                    gDma1p(displayList++, 4, vertexAddress,
                            (vertexCount << 3) + (vertexCount << 1) + 8,
                            (vertexCount << 3) | (vertexAddress & 6));
                    gDma1p(displayList++, 5, material + 0x80000000,
                            primitiveCount << 4, ((primitiveCount - 1) << 4) | 1);
                    vertexCount = 0;
                    primitiveCount = 0;
                    while ((index < group->entryCount) && (entry->type == 0x80)) {
                        index++;
                        entry++;
                    }
                    if (index < group->entryCount) {
                        texture = entry->texture;
                        scale = entry->textureScale;
                        colorAlpha = (entry->alphaWord >> 8) & 0xFF;
                        vertexStart = vertices;
                    }
                } else {
                    s32 geometryFlags;

                    emitted = 0;
                    points = group->points;
                    if (entry->flags & 0x800) {
                        red = (entry->red * entry->intensity) >> 8;
                        green = (entry->green * entry->intensity) >> 8;
                        blue = (entry->blue * entry->intensity) >> 8;
                    } else {
                        red = entry->red;
                        green = entry->green;
                        blue = entry->blue;
                    }
                    vertexCount += pointCount;
                    geometryFlags = entry->geometryFlags;
                    vertexAlpha = entry->alphaWord >> 8;
                    if (geometryFlags & 1) {
                        sine1 = func_8002A8BC(entry->rotation2);
                        cosine1 = func_8002A8C0(entry->rotation2);
                        sine0 = func_8002A8BC(entry->rotation0);
                        cosine0 = func_8002A8C0(entry->rotation0);
                        if (pointCount > 0) {
                            do {
                                xScaled = (f32)points[0] * entry->scale;
                                yScaled = (f32)points[1] * entry->scale;
                                points += 2;
                                rotatedZ = -xScaled * cosine0;
                                xScaled *= sine0;
                                vertices->x = (s16)(s32)((xScaled * sine1) - (yScaled * cosine1) + entry->x);
                                vertices->y = (s16)(s32)((yScaled * sine1) + (xScaled * cosine1) + entry->y);
                                vertices->z = (s16)(s32)(entry->z + rotatedZ);
                                vertices->red = red;
                                vertices->green = green;
                                vertices->blue = blue;
                                vertices->alpha = vertexAlpha;
                                vertices++;
                                emitted++;
                            } while (emitted != pointCount);
                        }
                    } else if (geometryFlags & 2) {
                        sine1 = func_8002A8BC(entry->rotation1);
                        cosine1 = func_8002A8C0(entry->rotation1);
                        sine0 = func_8002A8BC(entry->rotation0);
                        cosine0 = func_8002A8C0(entry->rotation0);
                        if (pointCount > 0) {
                            do {
                                xScaled = (f32)points[0] * entry->scale;
                                yScaled = (f32)points[1] * entry->scale;
                                points += 2;
                                rotatedZ = -xScaled * cosine0;
                                xScaled *= sine0;
                                vertices->x = (s16)(s32)(entry->x + xScaled);
                                vertices->y = (s16)(s32)((yScaled * sine1) - (rotatedZ * cosine1) + entry->y);
                                vertices->z = (s16)(s32)((rotatedZ * sine1) + (yScaled * cosine1) + entry->z);
                                vertices->red = red;
                                vertices->green = green;
                                vertices->blue = blue;
                                vertices->alpha = vertexAlpha;
                                vertices++;
                                emitted++;
                            } while (emitted != pointCount);
                        }
                    } else if (geometryFlags & 4) {
                        sine1 = func_8002A8BC(entry->rotation0);
                        cosine1 = func_8002A8C0(entry->rotation0);
                        if (pointCount > 0) {
                            do {
                                xScaled = (f32)points[0] * entry->scale;
                                points += 2;
                                vertices->x = (s16)(s32)((xScaled * sine1) + entry->x);
                                vertices->y = (s16)(s32)((f32)points[-1] * entry->scale + entry->y);
                                vertices->z = (s16)(s32)((-xScaled * cosine1) + entry->z);
                                vertices->red = red;
                                vertices->green = green;
                                vertices->blue = blue;
                                vertices->alpha = vertexAlpha;
                                vertices++;
                                emitted++;
                            } while (emitted != pointCount);
                        }
                    } else {
                        if (pointCount > 0) {
                            do {
                                point[0] = (f32)points[0] * entry->scale;
                                points += 2;
                                point[1] = (f32)points[-1] * entry->scale;
                                point[2] = 0.0f;
                                pointListRPY(1, (s16 *)entry, &point[0], &point[0]);
                                vertices->x = (s16)(s32)(point[0] + entry->x);
                                vertices->y = (s16)(s32)(point[1] + entry->y);
                                vertices->z = (s16)(s32)(point[2] + entry->z);
                                vertices->red = red;
                                vertices->green = green;
                                vertices->blue = blue;
                                vertices->alpha = vertexAlpha;
                                vertices++;
                                emitted++;
                            } while (emitted != pointCount);
                        }
                    }
                    entry++;
                    index++;
                    primitiveCount += updateRate;
                }
            }

            if (primitiveCount != 0) {
                vertexAddress = (s32)vertexStart + 0x80000000;
                if (colorAlpha != currentAlpha) {
                    gDPPipeSync(displayList++);
                    gDPSetPrimColor(displayList++, 0, 0, 255, 255, 255, colorAlpha);
                }
                if ((texture != previousTexture) || (scale != previousScale)) {
                    func_800349A4(&displayList, texture, 0x12,
                                  (s32)(scale * 65536.0f));
                }
                if (texture != NULL) {
                    textureType = *(u16 *)((u8 *)texture + 6);
                    if (textureType != 0x20) {
                        if (textureType != 0x40) {
                            material = group->materialTranslucent;
                        } else {
                            material = group->materialDefault;
                        }
                    } else {
                        material = group->materialOpaque;
                    }
                }
                gDma1p(displayList++, 4, vertexAddress,
                        (vertexCount << 3) + (vertexCount << 1) + 8,
                        (vertexCount << 3) | (vertexAddress & 6));
                gDma1p(displayList++, 5, material + 0x80000000,
                        primitiveCount << 4, ((primitiveCount - 1) << 4) | 1);
            }
            gDPPipeSync(displayList++);
        }
        *dListArg = displayList;
        *verticesArg = vertices;
    }
}
void partInitTrigger(ParticleTrigger *trigger, s32 type, s32 value) {
    ParticleConfig *config;

    if (type < D_8007C8B0) {
        config = D_8007C8B8[type];
        partInitTriggerPos(trigger, type, value, config->emitterPosition.x, config->emitterPosition.y,
                           config->emitterPosition.z);
    }
}
void partInitTriggerSPPos(ParticleTrigger *trigger, s32 type, s32 value, s32 index) {
    if (type < D_8007C8B0) {
        partInitTriggerPos(trigger, type, value, 0, 0, 0);
        trigger->index = index;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partInitTriggerPos.s; body reconstructed from Mickey evidence. */
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z) {
    s32 flags;
    ParticleConfig *config;

    config = D_8007C8B8[type];
    trigger->value = value;
    trigger->unk0C = 0;
    trigger->x = x;
    trigger->config = config;
    trigger->y = y;
    trigger->z = z;
    trigger->unk20 = 0;
    trigger->index = -1;
    flags = config->flags;
    if (flags & 0x4000) {
        trigger->flags = 0x4000;
        trigger->alpha = 0xFF;
        return;
    }
    if (flags & 0x400) {
        trigger->flags = 0x400;
        trigger->alpha = 0xFF;
        return;
    }
    trigger->flags = 0;
    trigger->value0E = config->sourceRotation.x;
    trigger->value10 = config->sourceRotation.y;
    trigger->value12 = config->sourceRotation.z;
    trigger->value14 = config->emissionDirection.x;
    trigger->value16 = config->emissionDirection.y;
    trigger->value18 = config->emissionDirection.z;
    trigger->alpha = 0;
}
ParticleTrigger *func_8003E730(s32 type, s32 value) {
    ParticleTrigger *trigger;
    s32 i;

    trigger = D_8007C8BC;
    i = 0;
    if (D_8007C8C0 > 0) {
        do {
            i++;
            if (trigger->active == 0) {
                trigger->active = 1;
                trigger->unk0C = 0;
                partInitTriggerPos(trigger, type, value, 0, 0, 0);
                return trigger;
            }
            trigger++;
        } while (i < D_8007C8C0);
    }
    return NULL;
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005F99C.s; body reconstructed from Mickey evidence. */
void func_8003E7B8(ParticleObject *object, s32 index) {
    ParticleConfig *config;
    ParticleTriggerSlot *trigger;
    ParticleTypeDescriptor *descriptor;
    s32 flags;

    trigger = &object->triggers[index];
    descriptor = D_8007C8AC[trigger->type];
    trigger->unk0C = 0;
    config = trigger->config;
    if (trigger->flags & 0x4000) {
        trigger->result = func_8003EB08(descriptor, config);
    } else {
        if (config->flags & 1) {
            trigger->unk08 = 0;
            trigger->value0E = config->sourceRotation.x;
            trigger->value10 = config->sourceRotation.y;
            trigger->value12 = config->sourceRotation.z;
        }
        if (config->flags & 4) {
            trigger->unk09 = 0;
            trigger->value14 = config->emissionDirection.x;
            trigger->value16 = config->emissionDirection.y;
            trigger->value18 = config->emissionDirection.z;
        }
    }
    flags = trigger->flags;
    if (flags & 0x400) {
        trigger->result = func_8003E8D8(descriptor, config, trigger);
        flags = trigger->flags;
    }
    trigger->flags = flags | 0x8000;
    object->activeTriggerCount++;
}
/* The `else { entry = NULL; }` arm below emits nothing and must not be removed.
 * uopt sinks the pool-base reset out of the join block when it can prove the
 * reset redundant on the guard's skip edge, and the guard branch then lands one
 * word past it. A store of a *different* value on that edge destroys the proof,
 * so the reset stays at the head of the join block; the store itself is
 * dead-store-eliminated after the placement decision and costs no instruction.
 * Writing the same value there keeps the copy redundant and does not work. Its
 * companion is the index initialiser: with the reset held in place, guard 2
 * needs its own preheader instruction for the delay slot, which is why `i = 0`
 * sits inside guard 1 and before guard 2 rather than the other way round. Each
 * edit alone is a regression; the pair is exact. */
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005FAE8.s; body reconstructed from Mickey evidence. */
s32 func_8003E8D8(ParticleTypeDescriptor *descriptor, ParticleConfig *config, ParticleTriggerSlot *trigger) {
    s32 result;
    f32 *pointData;
    f32 *point;
    s32 pointIndex;
    ParticleModelEntry *entry;
    s32 i;
    s32 frameCount;

    if (D_8007C898 == NULL) {
        return 0xFF;
    }

    entry = D_8007C898;
    result = 0xFF;
    if (D_8007C890 > 0) {
        i = 0;
        do {
            if (trigger == entry->trigger && entry->active == 1) {
                entry->active = 2;
                return i;
            }
            i++;
            entry++;
        } while (i < D_8007C890);
    } else {
        entry = NULL;
    }

    entry = D_8007C898;
    i = 0;
    if (D_8007C890 > 0) {
        do {
            if (entry->active == 0) {
                result = i;
                i = D_8007C890;
            } else {
                entry++;
            }
            i++;
        } while (i < D_8007C890);
    }

    if (result != 0xFF) {
        entry->active = 2;
        point = &((ParticlePointStreamEntry *)entry)->points[0][0];
        pointIndex = 0;
        ((ParticlePointStreamEntry *)entry)->pointCount = (u32)descriptor->pointCount >> 4;
        pointData = D_8007CA90[(u32)descriptor->pointCount >> 4];
        if (((ParticlePointStreamEntry *)entry)->pointCount > 0) {
            do {
                point[0] = pointData[0];
                point[1] = pointData[1];
                point[2] = 0.0f;
                pointIndex++;
                pointData += 2;
                point += 3;
            } while (pointIndex < ((ParticlePointStreamEntry *)entry)->pointCount);
        }
        entry->animationState = descriptor->descriptorWord;
        entry->configFlags = config->flags;
        if (descriptor->resourceId == -1) {
            entry->resource = NULL;
            entry->animationSpeed = 0;
            entry->textureFrame = 0.0f;
        } else {
            entry->resource = func_80034448(descriptor->resourceId);
            entry->animationSpeed = descriptor->animationSpeed;
            frameCount = ((ParticleTexture *)entry->resource)->frameCount >> 8;
            if (config->flags & 0x800) {
                entry->textureFrame = mathRnd(0, frameCount - 1);
            } else if ((entry->animationState & 1) == 2) {
                entry->textureFrame = (f32)frameCount - D_80082A48;
            } else {
                entry->textureFrame = 0.0f;
            }
        }
        entry->trigger = trigger;
    }
    return result;
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005FD34.s; body reconstructed from Mickey evidence. */
s32 func_8003EB08(ParticleTypeDescriptor *descriptor, ParticleConfig *config) {
    s32 result;
    s32 i;
    ParticleTexture *texture;
    ParticleLineEntry *line;
    s32 frameCount;

    if (D_8007C894 == NULL) {
        return 0xFF;
    }
    line = D_8007C894;
    result = 0xFF;
    i = 0;
    if (D_8007C88C > 0) {
        do {
            if (line->active == 0) {
                line->active = 2;
                line->config = config;
                line->unk130 = 0;
                result = i;
                i = D_8007C88C;
            } else {
                line++;
            }
            i++;
        } while (i < D_8007C88C);
    }
    if (result != 0xFF) {
        line->descriptorWord = *(s32 *)descriptor;
        line->configFlags = config->flags;
        if (*(s16 *)((u8 *)descriptor + 6) == -1) {
            line->texture = NULL;
            line->value144 = 0;
            line->textureFrame = 0.0f;
        } else {
            texture = func_80034448(*(s16 *)((u8 *)descriptor + 6));
            line->texture = texture;
            line->value144 = *(s16 *)((u8 *)descriptor + 8);
            frameCount = texture->frameCount >> 8;
            if (config->flags & 0x800) {
                line->textureFrame = mathRnd(0, frameCount - 1);
            } else if ((line->descriptorWord & 1) == 2) {
                line->textureFrame = frameCount - D_80082A4C;
            } else {
                line->textureFrame = 0.0f;
            }
        }
    }
    return result;
}
/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_8005FED8.s; body reconstructed from Mickey
 * evidence. */
void func_8003EC8C(ParticleObject *object, s32 index) {
    ParticleTriggerSlot *trigger;
    s32 flags;

    trigger = &object->triggers[index];
    trigger->flags &= ~0x8000;
    object->activeTriggerCount--;
    flags = object->triggers[index].flags;
    if (flags & 0x4000) {
        ParticleLineEntry *line;

        line = &D_8007C894[trigger->result];
        trigger->result = -1;
        line->active = 1;
        return;
    }
    if (flags & 0x400) {
        ParticleModelEntry *model;

        model = &D_8007C898[trigger->result];
        trigger->result = -1;
        model->active = 1;
    }
}
/* PROVENANCE: body adapted from JFG src/particles.c:partObjFreeTriggers. */
void partObjFreeTriggers(ParticleObject *object) {
    s32 i;
    s32 offset;
    s8 count;

    offset = 0;
    count = object->header->triggerCount;
    i = 0;
    if (count > 0) {
        do {
            if (((ParticleTriggerSlot *)((u8 *)object->triggers + offset))->flags & 0x8000) {
                func_8003EC8C(object, i);
            }
            offset += sizeof(ParticleTriggerSlot);
            /* Inert allocation aid retained by exact C; tracked in
             * docs/cleanup-queue.md. */
            if (offset) {}
        } while (++i != count);
    }
}
/* PROVENANCE: body adapted from JFG src/particles.c:partAdjustScaling. */
void partAdjustScaling(f32 scale) {
    D_8007C8F8 = scale;
}
void func_8003EDD4(f32 value) {
    D_8007C8F0 = value;
}
void func_8003EDE0(f32 value) {
    D_8007C8F4 = value;
}
/* PROVENANCE: body adapted from JFG src/particles.c:partUpdateTriggers. */
void partUpdateTriggers(ParticleObject *object, s32 updateRate) {
    ParticleTriggerSlot *base;
    s32 offset;
    s32 i;
    s32 flags;
    s8 count;
    u32 triggerBits;
    ParticleTriggerSlot *trigger;
    ParticleTriggerSlot *callBase;

    count = object->header->triggerCount;
    i = 0;
    triggerBits = object->triggerBits;
    offset = 0;
    if (count > 0) {
        do {
            trigger = (ParticleTriggerSlot *)((u8 *)object->triggers + offset);
            if (triggerBits & 1) {
                flags = trigger->flags;
                if (!(flags & 0x8000)) {
                    func_8003E7B8(object, i);
                    trigger = (ParticleTriggerSlot *)((u8 *)object->triggers + offset);
                    flags = trigger->flags;
                }
                if (flags & 0x4000) {
                    trigger->unk0C += updateRate;
                    func_8003EF80(object, (ParticleTriggerSlot *)(offset + (u8 *)object->triggers));
                } else if (flags & 0x400) {
                    trigger->unk0C += updateRate;
                    func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)object->triggers + offset));
                } else {
                    trigger->unk0C += updateRate;
                    base = object->triggers;
                    callBase = object->triggers;
                    trigger = (ParticleTriggerSlot *)((u8 *)base + offset);
                    if (trigger->unk0C >= trigger->config->value40) {
                        func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)callBase + offset));
                    }
                }
            } else if (trigger->flags & 0x8000) {
                func_8003EC8C(object, i);
            }
            i++;
            offset += sizeof(ParticleTriggerSlot);
            triggerBits >>= 1;
        } while ((0, i) != count);
    }
    D_8007C8F0 = 0.0f;
    D_8007C8F4 = 255.0f;
    D_8007C8F8 = 1.0f;
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8006020C.s; body reconstructed from Mickey evidence. */
void func_8003EF80(ParticleObject *object, ParticleTriggerSlot *trigger) {
    ParticleConfig *config;
    EmittedParticle *particle;
    s32 i;

    config = trigger->config;
    if (trigger->flags & 0x4000) {
        if (trigger->unk0C >= config->value40) {
            trigger->unk0C -= config->value40;
            func_80040B88((ParticleEmitterObject *)object, trigger);
        }
    } else if (trigger->flags & 0x400) {
        if (trigger->unk0C >= config->value40) {
            trigger->unk0C -= config->value40;
            func_80041F48((s32)object, (ParticleTrigger *)trigger);
        }
    } else if (trigger->unk0C >= config->value40) {
        do {
            trigger->unk0C -= config->value40;
            i = 0;
            if (config->value42 > 0) {
                do {
                    particle = (EmittedParticle *)func_8003FB98(
                        (ParticleEmitterObject *)object, (ParticleTrigger *)trigger, -1);
                    if (particle != NULL) {
                        if (trigger->unk0C != 0) {
                            particle->x -= D_800D4134;
                            particle->y -= D_800D4138;
                            particle->z -= D_800D413C;
                            func_80040878((CircularParticle *)particle, trigger->unk0C);
                            particle->x += D_800D4134;
                            particle->y += D_800D4138;
                            particle->z += D_800D413C;
                        }
                        particle->flags |= 0x20000;
                    }
                    i++;
                } while (i < config->value42);
            }
        } while (trigger->unk0C >= config->value40);
    }
}
/* Matched 2026-09-16 (lane w1-a), 11 -> 0 masked words at delta 0 in one
 * measured batch of eight cells, no colour force. The residual was the
 * comparison zero after the sqrt call: the ROM materialises it into scale's
 * pool colour, ours drew a ring temp, and the tail's ring phase followed.
 * The `0.0f` constant is one uopt web wherever it is spelled `0.0f`; with
 * the velocity head spelled `0.0f` the web reaches from the entry to the
 * comparison, is split, and the comparison's piece has one reference and
 * nothing to save. Spelling the velocity-head zeros as the integer literal
 * `0` (a different IR constant, L151, like the emission-direction zeros)
 * shrinks the `0.0f` web to the disableTransform arm and the comparison
 * (records: two blocks, totalsave 2, cost 0, coloured whole rather than
 * split); with the sum of squares inlined x-first `scale` no longer
 * occupies the sqrt argument block, so that web takes c25 and the
 * comparison zero lands on $f2 as the ROM has it. The direct Z accumulate
 * is the tail's ring phase.
 * The velocity head spelled `0.0` (double) is byte-identical; the named
 * sum with the same head is 18, the inlined sum with a `0.0f` head is 17.
 * PROVENANCE: structure cross-checked against JFG's assembly-only asm/nonmatchings/particles/func_80060400.s sibling; body reconstructed from Mickey evidence. */
void func_8003F154(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config) {
    s32 flags;
    f32 offset[3];
    ParticleEmitterResource *resource;
    s16 rotation[2];
    f32 magnitude;
    f32 scale;
    s32 randomRange;
    f32 speed;
    ParticleEmitterHeader *header;
    s8 pointIndex;

    if (config->flags & 0x70) {
        particle->velocityX = config->velocityModifier.x;
        particle->velocityY = config->velocityModifier.y;
        particle->velocityZ = config->velocityModifier.z;
    } else {
        particle->velocityX = 0;
        particle->velocityY = 0;
        particle->velocityZ = 0;
    }

    flags = config->flags5C & 0x700;
    if (flags != 0) {
        if (flags & 0x100) {
            randomRange = config->velocityModifierRange.x;
            particle->velocityX += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 0x200) {
            randomRange = config->velocityModifierRange.y;
            particle->velocityY += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 0x400) {
            randomRange = config->velocityModifierRange.z;
            particle->velocityZ += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
    }

    switch (config->flags & 0x70) {
        case 0x10:
            particle->velocityX += object->velocityX;
            particle->velocityY += object->velocityY;
            particle->velocityZ += object->velocityZ;
            break;
        case 0x40:
            particle->velocityX *= object->velocityX;
            particle->velocityY *= object->velocityY;
            particle->velocityZ *= object->velocityZ;
            break;
    }

    D_800D4134 = particle->velocityX;
    D_800D4138 = particle->velocityY;
    D_800D413C = particle->velocityZ;

    if (config->flags & 4) {
        flags = config->flags5C;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = -config->value3C;
        if (flags & 0x10) {
            randomRange = config->value70;
            offset[2] += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        speed = -offset[2];
        if (flags & 0x60) {
            rotation[0] = trigger->value14;
            if (flags & 0x20) {
                rotation[0] += mathRnd(-config->emissionDirectionRange.x, config->emissionDirectionRange.x);
            }
            rotation[1] = trigger->value16;
            if (flags & 0x40) {
                rotation[1] += mathRnd(-config->emissionDirectionRange.y, config->emissionDirectionRange.y);
            }
        } else {
            rotation[0] = trigger->value14;
            rotation[1] = trigger->value16;
        }
        mathOneFloatPY(rotation, offset);

        if (((ParticleEmitterPointSet *)(object->header + object->pointSetIndex))->transformedPoints != 0) {
            resource = object->resources[object->resourceIndex];
        } else {
            resource = NULL;
        }
        pointIndex = trigger->index;
        if (pointIndex != -1 && resource != NULL && resource->header->transformedPoints != 0) {
            header = resource->header;
            if (resource->disableTransform != 0) {
                offset[0] = 0.0f;
                offset[1] = 0.0f;
                offset[2] = 0.0f;
            } else {
                mtxf_transform_dir(
                    (u8 *)resource->matrices[resource->matrixTableIndex] +
                        ((header->transformIndices[pointIndex].matrixIndex << 5) << 1),
                    offset, offset, header);
                magnitude = sqrtf(((offset[0] * offset[0]) + (offset[1] * offset[1])) + (offset[2] * offset[2]));
                if (magnitude == 0.0f) {
                    scale = speed;
                } else {
                    scale = speed / magnitude;
                }
                offset[0] *= scale;
                offset[1] *= scale;
                offset[2] *= scale;
            }
        } else if (particle->parent != NULL) {
            pointListRPY(1, particle->parent, offset, offset);
        }

        particle->velocityX += offset[0];
        particle->velocityY += offset[1];
        particle->velocityZ += offset[2];
    }
}
/* PROVENANCE: structure cross-checked against JFG's assembly-only
 * asm/nonmatchings/particles/func_800608EC.s sibling; body reconstructed
 * from Mickey evidence.
 */
void func_8003F5F8(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config) {
    s32 flags;
    f32 offset[3];
    ParticleEmitterResource *resource;
    s16 rotation[2];
    s32 randomRange;
    s8 pointIndex;

    pointIndex = trigger->index;
    if (pointIndex == -1) {
        particle->localX = trigger->value1A;
        particle->localY = trigger->value1C;
        particle->localZ = trigger->value1E;
    } else {
        if (((ParticleEmitterPointSet *)(object->header + object->pointSetIndex))->transformedPoints == 0) {
            resource = object->resources[object->resourceIndex];
        } else {
            resource = NULL;
        }
        if (resource != NULL && resource->points != NULL) {
            particle->localX = resource->points[pointIndex][0];
            particle->localY = resource->points[trigger->index][1];
            particle->localZ = resource->points[trigger->index][2];
            if (resource->header->transformedPoints == 0) {
                particle->localX *= object->scale;
                particle->localY *= object->scale;
                particle->localZ *= object->scale;
                pointListRPY(1, (s16 *)object, &particle->localX, &particle->localX);
            } else {
                particle->localX -= object->x;
                particle->localY -= object->y;
                particle->localZ -= object->z;
            }
        } else {
            particle->localX = 0.0f;
            particle->localY = 0.0f;
            particle->localZ = 0.0f;
        }
    }

    particle->movementValue = config->value58;
    if (config->flags5C & 0x80000) {
        randomRange = config->value94;
        particle->movementValue += mathRnd(-randomRange, randomRange) * 0.000015258789f;
    }
    if (config->flags & 1) {
        offset[0] = 0.0f;
        offset[1] = 0.0f;
        offset[2] = -config->value10;
        flags = config->flags5C;
        if (flags & 1) {
            randomRange = config->value60;
            offset[2] += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 6) {
            rotation[0] = trigger->value0E;
            if (flags & 2) {
                rotation[0] += mathRnd(-config->sourceDirectionRange.x, config->sourceDirectionRange.x);
            }
            rotation[1] = trigger->value10;
            if (flags & 4) {
                rotation[1] += mathRnd(-config->sourceDirectionRange.y, config->sourceDirectionRange.y);
            }
            mathOneFloatPY(rotation, offset);
        } else {
            pointListRPY(1, &trigger->value0E, offset, offset);
        }
        particle->localX += offset[0];
        particle->localY += offset[1];
        particle->localZ += offset[2];
    }
    if (particle->type != 4 && trigger->index == -1) {
        pointListRPY(1, (s16 *)object, &particle->localX, &particle->localX);
    }
    particle->x = particle->localX;
    particle->y = particle->localY;
    particle->z = particle->localZ;
    if (particle->type == 4) {
        pointListRPY(1, (s16 *)object, &particle->x, &particle->x);
    }
    if (D_8007C8F8 != 1.0f) {
        particle->x *= D_8007C8F8;
        particle->y *= D_8007C8F8;
        particle->z *= D_8007C8F8;
    }
    particle->x += object->x;
    particle->y += object->y;
    particle->z += object->z;
}
/*
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/partModelObjEmitModelPart.s; body reconstructed
 * from Mickey evidence.
 */
void partModelObjEmitModelPart(ParticleModelObject *object, f32 velocityX, f32 velocityY, f32 velocityZ, s32 index) {
    s32 offset;
    ParticleTriggerSlot *trigger;
    ParticleConfig *config;
    BasicParticle *particle;

    if (index < object->triggerCount) {
        offset = index * sizeof(ParticleModelPartConfig);
        trigger = &object->triggers[index];
        config = D_8007C8B8[((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->type];
        trigger->config = config;
        trigger->type =
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->triggerType;
        trigger->flags = 0;
        trigger->unk0C = 0;
        trigger->value1A = 0;
        trigger->value1C = 0;
        trigger->value1E = 0;
        trigger->value20 = 0;
        trigger->value0E = config->sourceRotation.x;
        trigger->value10 = config->sourceRotation.y;
        trigger->value12 = config->sourceRotation.z;
        trigger->value14 = config->emissionDirection.x;
        trigger->value16 = config->emissionDirection.y;
        trigger->value18 = config->emissionDirection.z;
        trigger->index = -1;
        particle = (BasicParticle *)func_8003FB98(
            (ParticleEmitterObject *)object, (ParticleTrigger *)trigger,
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->particleType);
        if (particle != NULL && (velocityX != 0.0f || velocityY != 0.0f || velocityZ != 0.0f)) {
            particle->velocityX = velocityX;
            particle->velocityY = velocityY;
            particle->velocityZ = velocityZ;
        }
    }
}
/*
 * PROVENANCE: body adapted from DKR src/particles.c:create_general_particle
 * and cross-checked against JFG
 * asm/nonmatchings/particles/func_80060ED4.s. Mickey-specific resource,
 * colour, trigger, and floating-point paths are reconstructed from Mickey
 * evidence.
 */
CircularParticle *func_8003FB98(ParticleEmitterObject *object, ParticleTrigger *trigger, s32 resourceId) {
    s32 randomizationFlags;
    ParticleTypeDescriptor *descriptor;
    CircularParticle *particle;
    void *resource;
    f32 scale;
    ParticleConfig *config;
    s32 kind;

    descriptor = D_8007C8AC[trigger->value];
    kind = (u32)descriptor->flags >> 28;
    if (kind == 5) {
        return NULL;
    }
    if (resourceId == -1) {
        resourceId = descriptor->resourceId;
    }
    config = trigger->config;
    particle = func_8004054C(kind, resourceId);
    if (particle == NULL) {
        return particle;
    }

    particle->kind = descriptor->pad04 & 0xF;
    particle->flags = descriptor->descriptorWord;
    particle->state = 0;
    particle->parent = object;
    particle->colorTable = (u8 *)descriptor->colorTable;
    particle->colorCount = descriptor->colorCount;
    if ((particle->flags & 0x800) && object->shading != NULL) {
        particle->alpha = (s32)(*object->shading * D_8007C8F4);
    } else {
        particle->alpha = 0xFF;
    }

    scale = config->value50;
    if (config->flags5C & 0x20000) {
        scale += mathRnd(-config->value8C, config->value8C) * 0.000015258789f;
    }
    particle->scale = descriptor->scale * scale;

    scale = config->value54;
    if (config->flags5C & 0x40000) {
        scale += mathRnd(-config->value90, config->value90) * 0.000015258789f;
    }
    if (config->flags & 0x1000) {
        particle->scaleVelocity =
            sqrtf((object->velocityX * object->velocityX) +
                  (object->velocityY * object->velocityY) +
                  (object->velocityZ * object->velocityZ)) *
            scale * D_80082A50;
    } else {
        particle->scaleVelocity = descriptor->scale * scale;
    }
    if (D_8007C8F0 != 0.0f) {
        particle->scale *= D_8007C8F0;
        particle->scaleVelocity *= D_8007C8F0;
    }

    particle->lifetime =
        mathRnd(-descriptor->lifetimeRange, descriptor->lifetimeRange) + descriptor->lifetime;
    particle->colorMode = descriptor->colorMode;
    if (particle->flags & 0x400) {
        particle->red = (*(u32 *)particle->colorTable & 0xFF000000) >> 24;
        particle->green = (*(u32 *)particle->colorTable & 0xFF0000) >> 16;
        particle->blue = (*(u32 *)particle->colorTable & 0xFF00) >> 8;
    } else {
        particle->red = descriptor->red;
        particle->green = descriptor->green;
        particle->blue = descriptor->blue;
    }
    particle->alternateRed = descriptor->pad1C[0];
    particle->alternateGreen = descriptor->pad1C[1];
    particle->alternateBlue = descriptor->pad1C[2];

    randomizationFlags = config->flags5C & 0xF00000;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x100000) {
            particle->red += mathRnd(-config->value98, config->value98);
        }
        if (randomizationFlags & 0x200000) {
            particle->green += mathRnd(-config->value99, config->value99);
        }
        if (randomizationFlags & 0x400000) {
            particle->blue += mathRnd(-config->value9A, config->value9A);
        }
    }

    particle->intensityTimer = descriptor->intensityTimer;
    particle->colorTimer = descriptor->colorTimer;
    particle->colorIndex = 0;
    particle->intensity = descriptor->intensity << 8;
    if (particle->intensityTimer < particle->lifetime) {
        particle->intensityVelocity =
            ((descriptor->targetIntensity - descriptor->intensity) << 8) /
            (particle->lifetime - particle->intensityTimer);
    } else {
        particle->intensityVelocity = 0;
    }
    particle->updateTexture = descriptor->animationSpeed;

    if (particle->type == 2) {
        particle->resource = func_800355A0(resourceId, 0);
        if (particle->resource != NULL) {
            if ((*((ParticleSprite *)particle->resource)->textures)->flags & 4) {
                if (particle->flags & 0x1000) {
                    particle->transformFlags |= 8;
                } else {
                    particle->transformFlags |= 4;
                }
            }
            if (config->flags & 0x800) {
                particle->textureFrame =
                    mathRnd(0, ((ParticleSprite *)particle->resource)->frameCount - 1);
            } else if ((particle->flags & 1) == 2) {
                particle->textureFrame =
                    (f32)((ParticleSprite *)particle->resource)->frameCount - D_80082A54;
            } else {
                particle->textureFrame = 0.0f;
            }
        }
    } else if (particle->type == 1 || particle->type == 0) {
        if (resourceId < 0) {
            particle->resource = NULL;
        } else {
            resource = func_80034448(resourceId);
            if (((ParticleTexture *)resource)->flags & 4) {
                if (particle->flags & 0x1000) {
                    particle->transformFlags |= 8;
                } else {
                    particle->transformFlags |= 4;
                }
            }
            particle->textureFrame = 0.0f;
            particle->resource = resource;
        }
    } else if (particle->type == 3) {
        resource = func_8001F520(resourceId, 0);
        particle->resource = resource;
    }

    func_8003F5F8((BasicParticle *)particle, object, (ParticleTriggerSlot *)trigger, config);
    if (config->flags & 0x80) {
        particle->rotationY = config->rotation.x;
        particle->rotationX = config->rotation.y;
        particle->rotationZ = config->rotation.z;
    } else {
        particle->rotationY = object->rotationY + config->rotation.x;
        particle->rotationX = object->rotationX + config->rotation.y;
        particle->rotationZ = object->rotationZ + config->rotation.z;
    }

    randomizationFlags = config->flags5C & 0x3800;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x800) {
            particle->rotationY += mathRnd(-config->rotationRange.x, config->rotationRange.x);
        }
        if (randomizationFlags & 0x1000) {
            particle->rotationX += mathRnd(-config->rotationRange.y, config->rotationRange.y);
        }
        if (randomizationFlags & 0x2000) {
            particle->rotationZ += mathRnd(-config->rotationRange.z, config->rotationRange.z);
        }
    }

    particle->angularVelocityY = config->angularVelocity.x;
    particle->angularVelocityX = config->angularVelocity.y;
    particle->angularVelocityZ = config->angularVelocity.z;
    randomizationFlags = config->flags5C & 0x1C000;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x4000) {
            particle->angularVelocityY += mathRnd(-config->angularVelocityRange.x, config->angularVelocityRange.x);
        }
        if (randomizationFlags & 0x8000) {
            particle->angularVelocityX += mathRnd(-config->angularVelocityRange.y, config->angularVelocityRange.y);
        }
        if (randomizationFlags & 0x10000) {
            particle->angularVelocityZ += mathRnd(-config->angularVelocityRange.z, config->angularVelocityRange.z);
        }
    }

    func_8003F154((BasicParticle *)particle, object, (ParticleTriggerSlot *)trigger, config);
    particle->gravity = D_8007C8C8[(particle->flags >> 4) & 7];
    if (particle->flags & 0x80) {
        particle->gravity = -particle->gravity;
    }
    if (particle->kind == 5) {
        particle->movementValue =
            sqrtf((particle->velocityX * particle->velocityX) +
                  (particle->velocityY * particle->velocityY) +
                  (particle->velocityZ * particle->velocityZ));
    }
    if (descriptor->descriptorWord & 0x10000) {
        particle->trigger = func_8003E730(descriptor->triggerType, descriptor->triggerValue);
    } else {
        particle->trigger = NULL;
    }
    if (config->flags & 2) {
        trigger->sourceRotationCounter++;
        if (trigger->sourceRotationCounter >= config->value1A) {
            trigger->value0E += config->sourceAngularVelocity.x;
            trigger->value10 += config->sourceAngularVelocity.y;
            trigger->value12 += config->sourceRotation.z;
            trigger->sourceRotationCounter -= config->value1A;
        }
    }
    if (config->flags & 8) {
        trigger->directionRotationCounter++;
        if (trigger->directionRotationCounter >= config->value28) {
            trigger->value14 += config->emissionDirectionAngularVelocity.x;
            trigger->value16 += config->emissionDirectionAngularVelocity.y;
            trigger->value18 += config->emissionDirectionAngularVelocity.z;
            trigger->directionRotationCounter -= config->value28;
        }
    }
    if (particle->resource == NULL && particle->type != 4) {
        func_80040740(particle);
        return NULL;
    }
    particle->value54 = 8;
    if (config->flags & 0x10000) {
        particle->flags |= 0x80000;
    }
    if (particle->kind == 3) {
        object->hasMovingParticle = 1;
    }
    return particle;
}
/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80061948.s; body reconstructed from Mickey evidence. */
CircularParticle *func_8004054C(s32 type, s32 direction) {
    CircularParticlePool *pool;
    CircularParticle *particle;
    s32 bits;
    s32 wordIndex;
    s32 bitIndex;

    particle = NULL;
    wordIndex = 0;
    if (type >= 5) {
        return NULL;
    }
    pool = D_800D4120[type];
    if (pool != NULL) {
        if (pool->activeCount >= pool->count) {
            if (pool->exhausted == 0) {
                pool->exhausted = 1;
            }
        } else {
            if (direction == -1) {
                if (pool->freeBits[wordIndex] == 0) {
                    bits = pool->lastBitWord;
                    if (bits >= wordIndex) {
                        do {
                            wordIndex++;
                        } while (pool->freeBits[wordIndex] == 0 && wordIndex <= bits);
                    }
                }
                if (pool->lastBitWord < wordIndex) {
                    return NULL;
                }
                bits = pool->freeBits[wordIndex];
                bitIndex = 0;
                if (!(bits & 1)) {
                    do {
                        bitIndex++;
                    } while (!(bits & (1U << bitIndex)));
                }
                pool->freeBits[wordIndex] = bits & ~(1U << bitIndex);
                wordIndex = (wordIndex << 5) + bitIndex;
            } else {
                wordIndex = pool->lastBitWord;
                if (wordIndex > 0) {
                    if (pool->freeBits[wordIndex] == 0) {
                        do {
                            wordIndex--;
                        } while (wordIndex > 0 && pool->freeBits[wordIndex] == 0);
                    }
                }
                bits = pool->freeBits[wordIndex];
                if (bits == 0) {
                    return NULL;
                }
                bitIndex = 0x1F;
                if (!(bits & 0x80000000)) {
                    do {
                        bitIndex--;
                    } while (!(bits & (1U << bitIndex)));
                }
                pool->freeBits[wordIndex] = bits & ~(1U << bitIndex);
                wordIndex = (wordIndex << 5) + bitIndex;
            }
            if (wordIndex >= pool->count) {
                return NULL;
            }
            particle = &pool->particles[wordIndex];
            particle->type = type;
            pool->activeCount++;
        }
    }
    return particle;
}
/* PROVENANCE: structure cross-checked against JFG assembly function
 * func_80061B50; body reconstructed from Mickey evidence. */
/* Workbench: mixed structural/register residual, 9/78 words, first +0x1C.
 * Canonical flags, value lifetimes, masks, pointer math, and AST forms did not improve it.
 * The branch-likely sense, temp-ring slot 5, and three relocation identities remain. */
void func_80040740(CircularParticle *particle) {
    s32 index;
    CircularParticlePool *pool;

    if (particle->type >= 5) {
        if (particle->type != 0x80) {
            return;
        }
    } else {
        switch (particle->type) {
            case 2:
                if (particle->resource != NULL) {
                    func_800359D4(particle->resource);
                    particle->resource = NULL;
                }
                break;
            case 3:
                if (particle->resource != NULL) {
                    modFreeModel(particle->resource);
                    particle->resource = NULL;
                }
                break;
            case 0:
            case 1:
            case 4:
                break;
            default:
                return;
        }
        if (particle->trigger != NULL) {
            particle->trigger->active = 0;
        }
        pool = D_800D4120[particle->type];
        if (pool->activeCount > 0) {
            if (particle->resource != NULL) {
                func_800347A0(particle->resource);
            }
            pool->activeCount--;
            index = (particle - pool->particles);
            particle->type = 0x80;
            pool->freeBits[index >> 5] |= 1 << (index & 0x1F);
        }
    }
}
/*
 * PROVENANCE: body adapted from DKR src/particles.c:particle_update and
 * cross-checked against JFG asm/nonmatchings/particles/func_80061228_61E28.s.
 * Mickey's resource-update and colour-table paths are reconstructed from its
 * own evidence.
 */
s32 func_80040878(CircularParticle *particle, s32 updateRate) {
    s32 flags;

    flags = particle->flags;
    if (flags & 0x80000) {
        particle->flags = flags & ~0x80000;
        return 0;
    }

    D_800D4140 = updateRate;
    flags = particle->flags;
    if (flags & 0x20000) {
        particle->flags = flags ^ 0x20000;
        if (particle->kind == 3) {
            D_800D4140 = 0;
            func_800420E0((BasicParticle *)particle);
            D_800D4140 = updateRate;
        }
        goto done;
    }

    if (particle->type == 5) {
        goto done;
    }

    particle->lifetime -= D_800D4140;
    if (particle->lifetime <= 0) {
        func_80040740(particle);
        return 1;
    }

    if (particle->type == 2 && (particle->flags & 1)) {
        func_80036544(particle->resource, &particle->flags, particle->updateTexture,
                      &particle->textureFrame, D_800D4140);
    } else if (particle->type == 1 || particle->type == 0) {
        if (particle->resource != NULL) {
            func_800367A4(particle->resource, &particle->flags, particle->updateTexture,
                          &particle->textureFrame, D_800D4140);
        }
    }

    if (particle->kind == 2) {
        func_800421F4((BasicParticle *)particle);
    } else if (particle->kind == 3) {
        func_800420E0((BasicParticle *)particle);
    } else if (particle->kind == 4) {
        func_80041FEC((BasicParticle *)particle);
    } else if (particle->kind == 5) {
        func_800423EC((BasicParticle *)particle);
    } else {
        func_8004233C((BasicParticle *)particle);
    }

    if (particle->flags & 0x400) {
        if (particle->colorTimer == 0) {
            particle->colorIndex += D_800D4140;
            while (particle->colorIndex >= particle->colorCount) {
                particle->colorIndex -= particle->colorCount;
            }
            particle->red = particle->colorTable[particle->colorIndex * 4];
            particle->green = particle->colorTable[(particle->colorIndex * 4) + 1];
            particle->blue = particle->colorTable[(particle->colorIndex * 4) + 2];
        } else {
            particle->colorTimer -= D_800D4140;
            if (particle->colorTimer < 0) {
                particle->colorIndex -= particle->colorTimer;
            }
        }
    }

    if (particle->intensityTimer == 0) {
        particle->intensity += D_800D4140 * particle->intensityVelocity;
    } else {
        particle->intensityTimer -= D_800D4140;
        if (particle->intensityTimer < 0) {
            particle->intensity -= particle->intensityTimer * particle->intensityVelocity;
            particle->intensityTimer = 0;
        }
    }

    if (particle->trigger != NULL) {
        particle->trigger->unk0C += updateRate;
        func_8003EF80((ParticleObject *)particle, (ParticleTriggerSlot *)particle->trigger);
    }
done:
    return 0;
}
/* Reopened m2c reconstruction: exact 302 instructions and target 0x68 frame.
 *
 * 2026-09-12, lane p11-mid: 145 -> 111 on the frame, and the two ladders are now
 * slot-for-slot identical at 0x68.  tools/frame_census.py read the fault as a
 * uniform four-byte shift from the fifth home down: the target carries ONE MORE
 * four-byte local between `scale` and `position`, register-resident (it has no
 * traffic on either side), which is the orientation swap's own temp that an
 * earlier lane deleted to keep the frame at 0x68.  Reintroducing it alone does
 * grow the frame to 0x70 as that lane measured -- there is no slack -- but the
 * frame is a COUNT of declared memory-class locals, so deleting any other one
 * pays for it: swap temp after `scale` plus `entryIndex` inlined lands 0x68 with
 * the target's ladder.  Measured on the same base: swap alone 167, entryIndex
 * alone 145, pointCount alone 145, swap + entryIndex 111, swap + pointCount 111,
 * swap + both 111 (so the two are interchangeable and neither is homed), swap
 * before `scale` 117, after `position` 124, after `offset` 142, last 145.
 * Buckets went byte-exact 163 -> 197, naming 82 -> 94, immediate-only 37 -> 5,
 * structural 23 -> 8, displacement tax 3 -> 4.
 *
 * Then two statement-order sweeps, re-climbed on the new shape per [L146],
 * took 111 -> 72.  Both are full exhaustions, not samples.  The four statements
 * guarded by `pointCount != 9` admit twelve orders once `point` is kept ahead of
 * its own dereference; the floor is pointCount-bump, point, lifetime, scale at
 * 75, against 111 for the order this file used to carry and 128 for the worst.
 * The four `point` initialisers admit all 24; the floor is intensityTimer,
 * colorTimer, colorIndex, intensity at 72, and every order that does not put
 * intensityTimer first costs four bytes of frame and lands at 254 or worse,
 * because the later `point->intensityTimer < point->lifetime` test reloads it.
 *
 * Buckets now: byte-exact 231, register naming 58, immediate-only 2, structural
 * 13, displacement tax -1, frame ladders identical.  The naming residual is one
 * coherent three-cycle t5->t7->t4->t5 at 98% over two windows ([L127]) plus a
 * float f12->f14 phase.  The one structural fact left is at +0x74: the target
 * reloads `trigger` from its incoming home 108(sp) in the block BEFORE the
 * `pointCount != 9` branch and dereferences it after, where we keep no copy and
 * reload two instructions later, inside the branch's own block -- which is why
 * the target's home shows four loads to our three.  Both [L144] forms that would
 * force the reload were measured and both grow the frame, because taking the
 * parameter's address gives it a declared slot of its own: `*(T **)&trigger` is
 * 264 words at delta +4 and `(T *)*(s32 *)&trigger` is 299 at delta +8. */
/* PROVENANCE: adapted from DKR src/particles.c:update_line_particle and
 * cross-checked against JFG's assembly-only sibling. */
#ifdef NON_MATCHING
void func_80040B88(ParticleEmitterObject *object, ParticleTriggerSlot *trigger) {
    ParticleTypeDescriptor *descriptor;
    ParticleLineEntry *entry;
    ParticleLinePoint *point;
    u32 *colorTable;
    f32 scale;
    f32 swap;
    ParticleVec3f position;
    ParticleVec3f offset;
    s32 orientation;
    s32 pointCount;

    descriptor = D_8007C8AC[trigger->type];
    if ((u32)descriptor->flags >> 28 == 5) {
        if (trigger->result != 0xFF) {
            entry = &D_8007C894[trigger->result];
            pointCount = entry->pointCount;
            if (pointCount != 9) {
                entry->pointCount = pointCount + 1;
                point = entry->points[pointCount];
                point->lifetime = descriptor->lifetime;
                scale = descriptor->scale * trigger->config->value50;
                if (descriptor->descriptorWord & 0x400) {
                    colorTable = entry->colorTable;
                    if (colorTable == NULL) {
                        entry->colorCount = descriptor->colorCount;
                        colorTable = descriptor->colorTable;
                        entry->colorTable = colorTable;
                    }
                    point->red = (*colorTable & 0xFF000000) >> 24;
                    point->green = (*entry->colorTable & 0xFF0000) >> 16;
                    point->blue = (*entry->colorTable & 0xFF00) >> 8;
                } else {
                    point->red = descriptor->red;
                    point->green = descriptor->green;
                    point->blue = descriptor->blue;
                }
                point->intensityTimer = descriptor->intensityTimer;
                point->colorTimer = descriptor->colorTimer;
                point->colorIndex = 0;
                point->intensity = descriptor->intensity << 8;
                if (point->intensityTimer < point->lifetime) {
                    point->intensityVelocity =
                        ((descriptor->targetIntensity - descriptor->intensity) << 8) /
                        (point->lifetime - point->intensityTimer);
                } else {
                    point->intensityVelocity = 0;
                }

                position.x = trigger->value1A;
                position.y = trigger->value1C;
                position.z = trigger->value1E;
                pointListRPY(1, (s16 *)object, &position.x, &position.x);
                position.x += object->x;
                position.y += object->y;
                position.z += object->z;
                if (trigger->config->flags & 0x1000) {
                    scale = scale * sqrtf((object->velocityX * object->velocityX) +
                                          (object->velocityY * object->velocityY) +
                                          (object->velocityZ * object->velocityZ)) *
                            D_80082A6C;
                }

                orientation = *(u16 *)&descriptor->flags & 0xF;
                if (!(descriptor->descriptorWord & 0x4000)) {
                    offset.x = 0.0f;
                    offset.y = 0.0f;
                    offset.z = 0.0f;
                    switch (orientation) {
                        default:
                            offset.x = scale;
                            break;
                        case 2:
                            offset.z = scale;
                            break;
                        case 1:
                            offset.y = scale;
                            break;
                    }
                    pointListRPY(1, (s16 *)object, &offset.x, &offset.x);
                } else {
                    offset.x = object->velocityX;
                    offset.y = object->velocityY;
                    offset.z = object->velocityZ;
                    if ((offset.z * offset.z) +
                            ((offset.x * offset.x) + (offset.y * offset.y)) <
                        D_80082A70) {
                        scale = 1.0f;
                    } else {
                        scale = scale / sqrtf((offset.z * offset.z) +
                                              ((offset.x * offset.x) + (offset.y * offset.y)));
                    }
                    offset.x *= scale;
                    offset.y *= scale;
                    offset.z *= scale;
                    switch (orientation) {
                        case 0:
                            swap = offset.x;
                            offset.x = -offset.z;
                            offset.z = swap;
                            break;
                        case 1:
                            swap = offset.y;
                            offset.y = -offset.z;
                            offset.z = swap;
                            break;
                    }
                }

                point->x0 = position.x + offset.x;
                point->y0 = position.y + offset.y;
                point->z0 = position.z + offset.z;
                point->x1 = position.x - offset.x;
                point->y1 = position.y - offset.y;
                point->z1 = position.z - offset.z;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
#endif
/*
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80061C30_62830.s and DKR
 * src/particles.c:update_line_particle; body reconstructed from Mickey
 * evidence.
 */
void func_80041040(ParticleLineEntry *entry, s32 updateRate) {
    ParticleLinePoint *point;
    s32 i;
    s32 j;
    ParticleLinePoint *removedPoints[9];

    i = 0;
    if (entry->pointCount > 0 && entry->points[0]->lifetime < updateRate) {
        do {
            i++;
        } while (i < entry->pointCount && entry->points[i]->lifetime < updateRate);
    }

    if (i > 0) {
        entry->pointCount -= i;
        for (j = 0; j < i; j++) {
            removedPoints[j] = entry->points[j];
        }
        j = 0;
        while (i < 9) {
            j++;
            i++;
            entry->points[j - 1] = entry->points[i - 1];
        }
        i = 0;
        while (j < 9) {
            j++;
            entry->points[j - 1] = removedPoints[i];
            i++;
        }
    }

    if (entry->active == 1 && entry->pointCount == 0) {
        entry->active = 0;
        if (entry->texture != NULL) {
            func_800347A0(entry->texture);
        }
    } else {
        for (i = 0; i < entry->pointCount; i++) {
            point = entry->points[i];
            point->lifetime -= updateRate;
            if (entry->colorTable != NULL) {
                if (point->colorTimer == 0) {
                    point->colorIndex += updateRate;
                    if (point->colorIndex >= entry->colorCount) {
                        point->colorIndex -= entry->colorCount;
                    }
                    point->red = (entry->colorTable[point->colorIndex] & 0xFF000000) >> 24;
                    point->green = (entry->colorTable[point->colorIndex] & 0xFF0000) >> 16;
                    point->blue = (entry->colorTable[point->colorIndex] & 0xFF00) >> 8;
                } else {
                    point->colorTimer -= updateRate;
                    if (point->colorTimer < 0) {
                        point->colorIndex -= point->colorTimer;
                    }
                }
            }
            if (point->intensityTimer == 0) {
                point->intensity += updateRate * point->intensityVelocity;
            } else {
                point->intensityTimer -= updateRate;
                if (point->intensityTimer < 0) {
                    point->intensity -= point->intensityTimer * point->intensityVelocity;
                    point->intensityTimer = 0;
                }
            }
        }

        if (entry->active != 0) {
            if (entry->texture != NULL) {
                if (entry->configFlags & 0x800) {
                    entry->textureFrame = mathRnd(0, (entry->texture->frameCount >> 8) - 1);
                } else {
                    func_800367A4(entry->texture, &entry->descriptorWord, entry->value144,
                                  &entry->textureFrame, updateRate);
                }
            }
        }
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_80062A4C.s; body reconstructed from Mickey evidence. */
void func_80041388(ParticleModelEntry *entry, s32 updateRate) {
    CircularParticle **particle;
    s32 changed;
    s32 found;
    s32 i;
    s32 j;
    ParticleTexture *texture;

    changed = 0;
    i = 0;
    if (entry->particleCount > 0) {
        particle = entry->particles;
        do {
            if (func_80040878((CircularParticle *)*particle, updateRate) != 0) {
                *particle = NULL;
                changed = 1;
            }
            i++;
            particle++;
        } while (i < entry->particleCount);
    }

    if (changed != 0) {
        entry->particleCount = 0;
        i = 0;
        do {
            found = 0;
            j = i;
            if (i < 15) {
                do {
                    particle = &entry->particles[j];
                    if (*particle != NULL) {
                        found = 1;
                        if (i != j) {
                            entry->particles[i] = *particle;
                            *particle = NULL;
                        }
                        j = 15;
                        entry->particleCount++;
                    }
                    j++;
                } while (j < 15);
            }
            if (found == 0) {
                entry->particles[i] = NULL;
            }
            i++;
        } while (i != 15);
    }

    if (entry->active == 1 && entry->particleCount == 0) {
        texture = entry->resource;
        entry->active = 0;
        if (texture != NULL) {
            func_800347A0(texture);
            entry->resource = NULL;
        }
    }
    if (entry->active != 0) {
        texture = entry->resource;
        if (entry->configFlags & 0x800) {
            entry->textureFrame = mathRnd(0, (texture->frameCount >> 8) - 1);
        } else {
            func_800367A4(texture, &entry->animationState, entry->animationSpeed, &entry->textureFrame, updateRate);
        }
    }
}
/* Matched 2026-09-11 (lane f10-mid). Four edits from the 36-word plateau, each
 * measured alone against the target and read from the objects:
 *  - the two display-list command scalars are not locals: uopt hoists
 *    `vertexCount << 3` and `(vertexCount << 3) + (vertexCount << 1) + 8` out
 *    of the particle loop as loop-invariant temps and spills them to the
 *    temp slots at 76 and 72, which is why no declaration order ever reached
 *    a 72 home for a declared length (36 -> 89, head exact through +0xFC);
 *  - the particle cursor is `entry->particles[particleIndex]`, not a pointer
 *    local: its strength-reduced induction temp spills to the temp slot at
 *    92 rather than to a declared home at 108 (frame exact, 89 -> 87);
 *  - the scale loop shares `i * 3` between the two arrays, so `points` is
 *    read through a flat f32 view and `input` is indexed `i * 3 + k`; with
 *    `entry->points[i].x` and `input[i][k]` the shared temp is `i * 12`,
 *    globalcolor gives it a1, and the ring is out of phase for the rest of
 *    the function (87 -> 4);
 *  - two as1 tie-breaks (L59): `i = 0` first at the loop top with
 *    `vertexStart = vertices` last, and `command = displayList++` folded
 *    onto the w0 line of the triangle command (4 -> 2 -> 0). */
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/
 * func_80062BFC.s; body reconstructed from Mickey evidence. */
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry) {
    Gfx *displayList;
    ParticleVertex *vertices;
    ParticleVertex *vertexStart;
    f32 output[24];
    f32 input[24];
    f32 *outputPtr;
    s32 particleIndex;
    s32 i;
    s32 triangleListIndex;

    if (entry->particleCount >= 2) {
        s32 triangleCount;
        Gfx *command;
        s32 vertexCount;
        void *triangleLists[2];
        s32 vertexIndex;
        s32 addressBase;
        u8 red;
        u8 green;
        u8 blue;
        u8 alpha;

        displayList = *(Gfx **)arg0;
        vertexCount = entry->vertexCount;
        vertices = *(ParticleVertex **)arg1;
        triangleLists[0] = ((void **)((u8 *)&D_8007CA60 + (vertexCount * 8)))[-4];
        triangleLists[1] = ((void **)((u8 *)&D_8007CA60 + (vertexCount * 8)))[-3];
        if (entry->vertexCount == 2) {
            triangleCount = 2;
        } else {
            triangleCount = entry->vertexCount * 2;
        }
        vertexIndex = 0;
        triangleListIndex = 0;
        func_800349A4(&displayList, entry->resource, 0x12,
                      (s32)(entry->textureFrame * 65536.0f));

        particleIndex = 0;
        if (entry->particleCount > 0) {
            CircularParticle *particle;

            do {
                particle = entry->particles[particleIndex];
                i = 0;
                outputPtr = &output[0];
                vertexStart = vertices;
                while (i < vertexCount) {
                    input[i * 3 + 0] = ((f32 *)entry->points)[i * 3 + 0] * particle->scale;
                    input[i * 3 + 1] = ((f32 *)entry->points)[i * 3 + 1] * particle->scale;
                    input[i * 3 + 2] = ((f32 *)entry->points)[i * 3 + 2] * particle->scale;
                    i++;
                }

                red = particle->red;
                green = particle->green;
                blue = particle->blue;
                alpha = (particle->intensity >> 8) & 0xFF;
                pointListRPY(vertexCount, (s16 *)particle, &input[0], &output[0]);
                addressBase = 0x80000000;
                i = 0;
                if (vertexCount > 0) {
                    do {
                        vertices->x = outputPtr[0] + particle->renderX;
                        vertices->y = outputPtr[1] + particle->renderY;
                        vertices->z = outputPtr[2] + particle->renderZ;
                        vertices->red = red;
                        vertices->green = green;
                        vertices->blue = blue;
                        vertices->alpha = alpha;
                        outputPtr += 3;
                        vertices++;
                        i++;
                    } while (i < vertexCount);
                }

                command = displayList++;
                command->words.w0 =
                    (((vertexCount << 3) | (((s32)vertexStart + addressBase) & 6)) & 0xFF) << 16 |
                    0x04000000 | ((((vertexCount << 3) + (vertexCount << 1) + 8) | (vertexIndex << 9)) & 0xFFFF);
                command->words.w1 = (s32)vertexStart + addressBase;
                if (particleIndex > 0) {
                    command = displayList++; command->words.w0 = ((((((triangleCount - 1) << 4) | 1) & 0xFF) << 16) |
                                         0x05000000 | ((triangleCount << 4) & 0xFFFF));
                    command->words.w1 = (s32)triangleLists[triangleListIndex] + addressBase;
                    triangleListIndex ^= 1;
                }
                if (vertexIndex == 0) {
                    vertexIndex = vertexCount;
                } else {
                    vertexIndex = 0;
                }
                particleIndex++;
            } while (particleIndex < entry->particleCount);
        }
        *(Gfx **)arg0 = displayList;
        *(ParticleVertex **)arg1 = vertices;
    }
}
void func_80041C50(s32 arg0, s32 arg1) {
    ParticleModelEntry *entry;
    s32 i;

    if (D_8007C898 != NULL) {
        entry = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (entry->active != 0) {
                    func_80041530(arg0, arg1, entry);
                }
                i++;
                entry++;
            } while (i < D_8007C890);
        }
    }
}
/* PROVENANCE: structure cross-checked against JFG's assembly-only
 * func_80063514 sibling; body reconstructed from Mickey evidence. */
void func_80041CE4(Gfx **dList, ParticleLineVertex **vertices) {
    Gfx *command;
    Gfx *command2;
    ParticleLineVertex *vertex;
    ParticleLineVertex *vertexStart;
    Gfx *displayList;
    ParticleLinePoint *point;
    ParticleLineEntry *line;
    s32 i;
    s32 j;
    s32 pointCount;
    if (D_8007C894 != NULL) {
        displayList = *dList;
        vertex = *vertices;
        line = D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active != 0) {
                    if (line->pointCount >= 2) {
                        vertexStart = vertex;
                        for (j = 0; j < line->pointCount; j++) {
                            point = line->points[j];
                            vertex->x0 = point->x0;
                            vertex->y0 = point->y0;
                            vertex->z0 = point->z0;
                            vertex->red0 = point->red;
                            vertex->green0 = point->green;
                            vertex->blue0 = point->blue;
                            vertex->alpha0 = ((u8 *)&point->intensity)[0];
                            vertex->x1 = point->x1;
                            vertex->y1 = point->y1;
                            vertex->z1 = point->z1;
                            vertex->red1 = point->red;
                            vertex->green1 = point->green;
                            vertex->blue1 = point->blue;
                            vertex->alpha1 = ((u8 *)&point->intensity)[0];
                            vertex++;
                        }
                        func_800349A4(&displayList, line->texture, 0x12,
                                      (s32)(line->textureFrame * 65536.0f));
                        pointCount = line->pointCount;
                        pointCount *= 2;
                        command = displayList++;
                        command->words.w0 = ((((pointCount << 3) | (((s32)vertexStart + 0x80000000) & 6)) & 0xFF) << 16) |
                                            0x04000000 |
                                            ((((pointCount << 3) + (pointCount << 1)) + 8) & 0xFFFF);
                        command->words.w1 = ((s32)vertexStart + 0x80000000);
                        command2 = displayList++;
                        command2->words.w0 = (((((pointCount - 3) << 4) | 1) & 0xFF) << 16) |
                                            0x05000000 | (((pointCount - 2) << 4) & 0xFFFF);
                        command2->words.w1 = (s32)D_7C900;
                    }
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
        gDPPipeSync(displayList++);
        *dList = displayList;
        *vertices = vertex;
    }
}
void func_80041F48(s32 arg0, ParticleTrigger *trigger) {
    void *particle;
    ParticleModelEntry *entry;
    ParticleTypeDescriptor *descriptor;
    u8 index;

    descriptor = D_8007C8AC[trigger->value];
    index = trigger->alpha;
    if (index != 0xFF && ((u32)descriptor->flags >> 28) == 4) {
        entry = &D_8007C898[index];
        if (entry->particleCount != 15) {
            particle = func_8003FB98((ParticleEmitterObject *)arg0, trigger, -1);
            if (particle != NULL) {
                entry->particles[entry->particleCount] = particle;
                entry->particleCount++;
            }
        }
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_basic_parent. */
void func_80041FEC(BasicParticle *particle) {
    s32 i;
    ParticleParent *parent;

    i = D_800D4140;
    while (i-- > 0) {
        particle->localX += particle->velocityX;
        particle->localY += particle->velocityY;
        particle->velocityY -= particle->gravity;
        particle->localZ += particle->velocityZ;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
        particle->scale += particle->scaleVelocity;
    }
    particle->x = particle->localX;
    particle->y = particle->localY;
    particle->z = particle->localZ;
    parent = particle->parent;
    if (parent != NULL) {
        particle->x += parent->x;
        particle->y += parent->y;
        particle->z += parent->z;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_attached_to_parent. */
void func_800420E0(BasicParticle *particle) {
    s32 i = D_800D4140;
    ParticleParent *parent;

    while (i-- > 0) {
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
        particle->scale += particle->scaleVelocity;
    }

    particle->x = 0.0f;
    particle->y = -particle->movementValue;
    particle->z = 0.0f;
    pointListRPY(1, (s16 *)particle, &particle->x, &particle->x);
    particle->x += particle->localX;
    particle->y += particle->localY;
    particle->z += particle->localZ;

    parent = particle->parent;
    if (parent != NULL) {
        particle->x += parent->x;
        particle->y += parent->y;
        particle->z += parent->z;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_with_acceleration. */
void func_800421F4(BasicParticle *particle) {
    s32 i = 0;
    f32 acceleration[3];

    while (i++ < D_800D4140) {
        particle->x += particle->velocityX;
        particle->y += particle->velocityY;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;

        acceleration[0] = 0.0f;
        acceleration[1] = -particle->movementValue;
        acceleration[2] = 0.0f;
        pointListRPY(1, (s16 *)particle, acceleration, acceleration);
        particle->velocityX += acceleration[0];
        particle->velocityY += acceleration[1];
        particle->velocityY -= particle->gravity;
        particle->velocityZ += acceleration[2];
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_basic. */
void func_8004233C(BasicParticle *particle) {
    s32 i = 0;

    while (i++ < D_800D4140) {
        particle->x += particle->velocityX;
        particle->y += particle->velocityY;
        particle->velocityY -= particle->gravity;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_forward. */
void func_800423EC(BasicParticle *particle) {
    s32 i = 0;

    while (i++ < D_800D4140) {
        particle->velocityX = 0.0f;
        particle->velocityY = 0.0f;
        particle->velocityZ = -particle->movementValue;
        mathOneFloatPY(particle, &particle->velocityX);
        particle->x += particle->velocityX;
        particle->y += particle->velocityY - particle->gravity;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partUpdateParticles.s; body reconstructed from Mickey evidence. */
void partUpdateParticles(s32 updateRate) {
    CircularParticlePool **poolPtr;
    CircularParticlePool **poolEnd;
    CircularParticlePool *pool;
    CircularParticle *particle;
    ParticleLineEntry *line;
    ParticleModelEntry *pointStream;
    s32 i;

    poolPtr = D_800D4120, poolEnd = D_800D4130;
    do {
        pool = *poolPtr;
        if (pool != NULL) {
            particle = pool->particles;
            i = 0;
            if (pool->count > 0) {
                do {
                    if (particle->type != 0x80) {
                        func_80040878(particle, updateRate);
                    }
                    i++;
                    particle++;
                } while (i < pool->count);
            }
        }
        poolPtr++;
    } while (poolPtr < poolEnd);

    if (D_8007C894 != NULL) {
        line = D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active != 0) {
                    func_80041040(line, updateRate);
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
    }

    if (D_8007C898 != NULL) {
        pointStream = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (pointStream->active != 0) {
                    func_80041388(pointStream, updateRate);
                }
                i++;
                pointStream++;
            } while (i < D_8007C890);
        }
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partDraw.s; body reconstructed from Mickey evidence. */
void partDraw(Gfx **dList, s32 arg1, s32 mode) {
    ParticleLineVertex *vertices;
    s32 pad;

    if (mode != 0) {
        D_8007C8EC = 0;
    }
    vertices = (u8 *)D_8007C89C[D_8007C8E8] + (D_8007C8EC * 10);
    gDPPipeSync((*dList)++);
    if (mode == 1) {
        D_8007C8EC = func_8003CE10(dList, arg1, (void **)&vertices,
                                   D_800D4128, 1);
        return;
    }
    camSetNo(0);
    func_800221E8((void **)dList, arg1);
    func_8003D4FC((void **)dList, (void **)&vertices, D_800D4120[0]);
    func_8003D4FC((void **)dList, (void **)&vertices, D_800D4124);
    func_80041CE4(dList, &vertices);
    func_80041C50((s32)dList, (s32)&vertices);
    func_8003CE10(dList, arg1, (void **)&vertices, D_800D4128, mode);
    func_8003D25C(dList, arg1, (void **)&vertices, D_800D412C);
    D_8007C8E8 ^= 1;
}
void partNullifyCircularParticleParents(ParticlePosition *position) {
    CircularParticle *particle;
    CircularParticlePool *pool;
    s32 i;
    CircularParticlePool **poolPtr;

    pool = NULL;
    do {
        poolPtr = D_800D4120; do {
            pool = *poolPtr++;
            particle = pool->particles;
            i = 0;
            if (pool->count > 0) {
                do {
                    i++;
                    if (particle->type != 0x80 && particle->kind == 3) {
                        particle->x += position->x;
                        particle->y += position->y;
                        particle->z += position->z;
                        particle->parent = NULL;
                    }
                    particle++;
                } while (i <= pool->count - 1);
            }
        } while (poolPtr != (CircularParticlePool **)&D_800D4134);
    } while (0);
}

/* PLATEAU-HANDOFF:func_8003D25C:start
 * symbol: func_8003D25C
 * score: 68/168 words
 * frame: 0xB8
 * relocations: 2
 * first-mismatch: +0x50
 * summary: Authenticated 202-force landscape: web 79 reaches 62, while neighbouring gains are rival footprints and their combinations decline; no force is exact.
 * PLATEAU-HANDOFF:func_8003D25C:end
 */

/* PLATEAU-HANDOFF:func_80040B88:start
 * symbol: func_80040B88
 * score: 72/302 words
 * frame: 0x68
 * relocations: 12
 * first-mismatch: +0x48
 * summary: pre-entry MOVE_END blocks the needed ring phase; source-safe extra draw remains unproved
 * PLATEAU-HANDOFF:func_80040B88:end
 */
