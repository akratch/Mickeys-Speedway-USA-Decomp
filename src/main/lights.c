/*
 * Resident lights -- ROM 0x19310-0x1AE60 (VRAM 0x80018710).
 *
 * The boundary and function comparison map are documented in
 * docs/modules.md section 3.4. The preceding four-function shadows TU uses
 * odd single-precision FP registers and remains assembly by section 6.2;
 * none of the functions in this TU uses an odd FP register.
 *
 * PROVENANCE -- comparison names, declarations and future starting bodies
 * for this TU come from Jet Force Gemini's public decomp, src/lights.c and
 * src/lights.h. They are permitted published-decomp material under
 * docs/CLEANROOM.md. Mickey's own bytes decide every body and name adoption;
 * the unresolved functions below therefore retain their Mickey func_ names.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul (measured on the FP helpers).
 */

#include "PR/ultratypes.h"
#include "game/lights.h"

extern void initColourCycle();
extern f32 sqrtf(f32 value);
extern f32 func_8002A8BC(s32 angle);
extern void mmFree(void *ptr);
extern void *func_8002B280(s32 size, s32 tag);
extern void lightCreateLightTable(s32 red, s32 green, s32 blue, void *table);
extern void func_8000D728(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 func_8000D62C(f32 x, f32 y, f32 z, f32 radius, f32 radius2, s32 red, s32 green, s32 blue);
extern void func_800188CC(UnkLight *light);
extern void func_80018F08(UnkLight *light, s32 updateRate);
extern f32 func_80019934(f32 arg0, f32 arg1, f32 arg2, s32 arg3);
extern u8 *levelGetLevel(void);
extern s32 D_80079490;
extern s32 D_80079494;
extern void **D_80079498;
extern void *D_8007949C;
extern void *D_800794A0;
extern f32 D_800817B0;
extern s32 D_8007C854;
extern s32 D_8007C85C;
extern f32 D_800817C8;
extern void *D_800CB290;

/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
typedef struct LightingObject {
    u8 pad0[0x40];
    u8 *segmentData;
    u8 pad44[0xC];
    s32 objectLight;
    u8 pad54[0x3B];
    u8 unk8F;
    u8 pad90[3];
    u8 segmentIndex;
} LightingObject;

/* PROVENANCE: adapted from JFG's public decomp, include/structs.h, with Mickey offsets. */
typedef struct LightSourceHeader {
    u8 pad0[0x28];
    s8 lightCount;
    u8 pad29[0x23];
    ObjectLightEntry *lights;
} LightSourceHeader;

struct LightSourceObject {
    u8 pad0[0x40];
    LightSourceHeader *header;
    u8 pad44[0x2C];
    UnkLight **lights;
};

typedef struct FlareEntry {
    s8 type;
    u8 subtype;
    u8 pad2[6];
    f32 x;
    f32 y;
    f32 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    s16 kind;
    u16 size;
    s16 scaledSize;
    s8 index;
    s8 enabled;
} FlareEntry;

typedef struct GlowEntry {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u16 size;
    u8 scale;
    u8 pad11[2];
    u8 subtype;
} GlowEntry;

typedef struct GlowObject {
    u8 pad0[0x3C];
    GlowEntry *entry;
    u8 pad40[0x24];
    void *flare;
} GlowObject;

typedef struct ObjectLightState {
    f32 directionX;
    f32 directionY;
    f32 directionZ;
    s32 scaleStep;
    s32 colourStep;
    u8 shift;
    u8 endValue;
    u8 valueDelta;
    u8 startValue;
    void *table;
    s16 yaw;
    s16 pitch;
} ObjectLightState;

typedef struct LightUpdateHeader {
    u8 pad0[0x2D];
    u8 count2D;
} LightUpdateHeader;

/* PROVENANCE: Vec3f coordinate rows follow JFG camlight.h CamLight_inner1. */
typedef struct LightUpdateVec3f {
    f32 x;
    f32 y;
    f32 z;
} LightUpdateVec3f;

typedef struct LightUpdateSegment {
    LightUpdateHeader *header0;
    u8 pad4[0x3C];
    LightUpdateVec3f *coordinates40;
} LightUpdateSegment;

typedef struct LightUpdateOwner {
    u8 pad0[0xC];
    f32 offsetX0C;
    f32 offsetY10;
    f32 offsetZ14;
    u8 pad18[0x22];
    s8 segmentIndex3A;
    u8 pad3B[0x2D];
    LightUpdateSegment **segments68;
    u8 pad6C[0x25];
    u8 disabled91;
} LightUpdateOwner;

typedef struct LightUpdateState {
    u8 mode0;
    u8 pad1;
    u8 flags2;
    u8 flags3;
    u8 pad4[2];
    s16 index6;
    f32 inputX08;
    f32 inputY0C;
    f32 inputZ10;
    LightUpdateOwner *owner14;
    f32 x18;
    f32 y1C;
    f32 z20;
    u8 pad24[0x14];
    f32 lower38;
    f32 upper3C;
    u8 red40;
    u8 green41;
    u8 blue42;
    u8 intensity43;
    f32 intensity44;
    u8 pad48[0xC];
    s32 colourCycle54;
    s16 value58;
    s16 value5A;
    s16 step5C;
    s16 step5E;
    f32 directionX60;
    f32 directionY64;
    f32 directionZ68;
    s32 trackLight6C;
    void *table70;
} LightUpdateState;

typedef struct LightObjectState LightObjectState;

typedef struct LightObjectContext {
    LightObjectState *state0;
    s32 objectId4;
} LightObjectContext;

struct LightObjectState {
    u8 pad0[0x20];
    s32 environment20;
    u8 pad24[0x2B];
    s8 mode4F;
    u8 pad50[0xC];
    s32 value5C;
    s32 value60;
    s32 environment64;
};

typedef struct LightDescription {
    f32 scale0;
    u8 pad4[0xA];
    s16 jointCountE;
    u8 pad10[0x10];
    u32 packed20;
    u8 shift24;
    u8 red25;
    u8 green26;
    u8 blue27;
    u8 pad28[8];
    void *twoLightData30;
} LightDescription;

typedef struct LightData {
    u8 pad0[0x10];
    u32 packed10;
    u8 shift14;
    u8 red15;
    u8 blue16;
    u8 green17;
} LightData;

typedef struct LightPosition {
    u8 pad0[0xC];
    f32 x0C;
    f32 y10;
    f32 z14;
} LightPosition;

typedef struct LightCameraPosition {
    u8 pad0[0xC];
    f32 x0C;
    f32 y10;
    f32 z14;
} LightCameraPosition;

typedef struct LightInitHeader {
    u8 pad0[0x1E];
    s8 mode;
    u8 pad1F[0x71];
    f32 green;
    f32 red;
    u8 pad98[4];
    s16 yaw;
    s16 pitch;
    s16 shift;
} LightInitHeader;

typedef struct LightLevelData {
    u8 pad0[0xD8];
    s16 pitch;
    s16 yaw;
    u8 padDC[3];
    u8 green;
    u8 red;
    u8 padE1[2];
    u8 useLevelLight;
} LightLevelData;

typedef struct LightInitState {
    f32 intensity;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 padB[3];
    s16 enabled;
    ObjectLightState light;
} LightInitState;

typedef struct FlareHeader {
    u8 pad0[0x29];
    u8 flareCount;
    u8 pad2A[0x26];
    FlareEntry *flares;
} FlareHeader;

struct FlareObject {
    u8 pad0[0x40];
    FlareHeader *header;
    u8 pad44[0x30];
    void **flares;
};

extern LightingObject **func_8000572C(s32 *start, s32 *end);
extern void func_8001953C(LightingObject *object, s32 objectLight);
extern void func_80019DE8(ObjectLightState *state, s32 arg1, s32 arg2, s16 arg3, s16 arg4, s32 arg5);
extern void mathOneFloatRPY(s16 *rotation, f32 *output);
extern void *camlightAdd(void *object, FlareEntry *entry);
extern void camlightDelete(void);
extern ObjectLightState D_800CB298;
extern void func_8000D768(s32 light, s32 red, s32 green, s32 blue, s32 intensity);
extern void func_8000D7F8(s32 light, f32 x, f32 y, f32 z);
extern void func_80036AB0(void *cycle, s32 updateRate);
extern void mathOneFloatPY(s16 *rotation, f32 *vector);
extern void pointListRPY(s32 count, void *rotation, f32 *input, f32 *output);
extern void *camGetPtr(void);
extern void func_8002AA50(void *source, void *matrix);
extern void lightMakeJointPositions(s16 count, f32 *positions, s32 value,
                                    void *matrix, void *table);
extern void lightSingleLight(s32 objectId, s32 value, f32 *positions,
                             void *table, s32 value2);
extern void lightTwoLights(s32 objectId, s32 value, s32 table, s32 count,
                           f32 *positions, void *lightData, s32 value2);
extern void lightMultiLight(s32 objectId, s32 value, f32 *positions, s32 value2,
                            s32 table, s32 count, s32 jointCount);
extern void lightEnviroMap(s32 value0, s32 value1, s32 value2, s32 table);

/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void freeLights(void) {
    if (D_80079498 != 0) {
        mmFree(D_80079498);
        D_80079498 = 0;
        D_8007949C = 0;
    }
    if (D_800CB290 != 0) {
        mmFree(D_800CB290);
        D_800CB290 = 0;
    }
    if (D_800794A0 != 0) {
        mmFree(D_800794A0);
        D_800794A0 = 0;
    }
    D_80079494 = 0;
    D_80079490 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey's allocation sizes. */
void setupLights(s32 count, s32 arg1, s32 arg2) {
    s32 i;
    void **buffer;

    freeLights();
    D_80079490 = count;
    buffer = func_8002B280(D_80079490 * 0x78, 0x89);
    D_800CB290 = func_8002B280((D_80079490 << 9) + 0x200, 0x89);
    D_800794A0 = func_8002B280(0x240, 0x89);
    D_8007949C = (void **)((u8 *) buffer + (D_80079490 * sizeof(void *)));
    D_80079498 = buffer;
    for (i = 0; i < D_80079490; i++) {
        D_80079498[i] = (i * 0x74) + (u8 *) D_8007949C;
        *(void **) ((u8 *) D_8007949C + (i * 0x74) + 0x70) =
            (u8 *) D_800CB290 + 0x200 + (i * 0x200);
    }
    lightCreateLightTable(255, 255, 255, D_800CB290);
}
/* PROVENANCE: adapted from DKR's public decomp, src/lights.c, and Mickey's own assembly. */
void func_800188CC(UnkLight *light) {
    f32 radius;

    if (!(light->unk3 & 0x40)) {
        radius = light->radius;
        light->unk6C = func_8000D62C(
            light->x, light->y, light->z,
            radius * 1.25f, radius * D_800817B0,
            (light->red * light->unk43) >> 8,
            (light->green * light->unk43) >> 8,
            (light->blue * light->unk43) >> 8);
    } else {
        light->unk6C = 0;
    }
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *addRomdefLight(s32 arg0, RomdefLight *entry) {
    UnkLight *light;
    u8 *levelData;

    light = NULL;
    if (D_80079494 < D_80079490) {
        light = D_80079498[D_80079494];
        D_80079494++;
        light->unk0 = (entry->modeAndType >> 4) & 0xF;
        light->unk1 = entry->modeAndType & 0xF;
        if ((entry->flags & 0xE0) == 0x40) {
            light->unk0 = 3;
        }
        light->unk2 = 7;
        light->unk3 = ~entry->enabledFlags & 0x61;
        light->home = 0;
        light->index = -1;
        light->directionX = 0.0f;
        light->directionY = 0.0f;
        light->directionZ = 0.0f;
        light->owner = NULL;
        light->x = entry->x;
        light->y = entry->y;
        light->z = entry->z;
        light->radius = entry->radius;
        light->radius2 = light->radius;
        light->radius3 = light->radius;
        if (light->unk0 == 2) {
            light->radius2 = entry->radius2;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusInverse = 1.0f / light->radius;
        light->red = entry->red;
        light->green = entry->green;
        light->blue = entry->blue;
        light->unk43 = entry->intensity;
        light->unk44 = (u32) entry->intensity;
        light->unk54 = 0;
        if (entry->colourCycleIndex < 7) {
            levelData = levelGetLevel();
            if (*(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)) != -1) {
                initColourCycle(light->colourCycle, *(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)), entry);
            }
        }
        light->value58 = entry->value58;
        light->value5C = entry->value5C;
        light->value5A = entry->value5A;
        light->value5E = entry->value5E;
        func_800188CC(light);
        func_80018F08(light, 0);
    }
    return light;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *addObjectLight(s32 owner, ObjectLightEntry *entry) {
    UnkLight *light;

    light = NULL;
    if (D_80079494 < D_80079490) {
        light = D_80079498[D_80079494];
        D_80079494++;
        light->unk0 = entry->mode;
        light->unk1 = entry->type;
        light->unk2 = 7;
        light->unk3 = entry->flags;
        light->home = entry->home;
        light->index = entry->index;
        light->directionX = entry->x;
        light->directionY = entry->y;
        light->directionZ = entry->z;
        light->owner = (void *) owner;
        light->x = light->directionX;
        light->y = light->directionY;
        light->z = light->directionZ;
        light->radius = (u32) entry->radius;
        light->radius2 = light->radius;
        light->radius3 = light->radius;
        if (light->unk0 == 2) {
            light->radius2 = (u32) entry->radius2;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusInverse = 1.0f / light->radius;
        light->red = entry->red;
        light->green = entry->green;
        light->blue = entry->blue;
        light->unk43 = entry->intensity;
        light->unk44 = (u32) entry->intensity;
        if (entry->colourCycleIndex != -1) {
            initColourCycle(light->colourCycle, entry->colourCycleIndex, entry);
        } else {
            light->unk54 = 0;
        }
        light->value58 = entry->value58 << 8;
        light->value5C = 0;
        light->value5A = entry->value5A << 8;
        light->value5E = 0;
        func_800188CC(light);
        func_80018F08(light, 0);
    }
    return light;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOff(UnkLight *light) {
    light->unk3 &= ~1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOn(UnkLight *light) {
    light->unk3 |= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void toggleLight(UnkLight *light) {
    light->unk3 ^= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColour(UnkLight *light, u8 red, u8 green, u8 blue) {
    light->red = red;
    light->green = green;
    light->blue = blue;
    light->unk2 |= 2;
    light->unk54 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColourCycle(s32 arg0, s32 arg1) {
    initColourCycle(arg0 + 0x48, arg1);
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightIntensity(UnkLight *light, u8 intensity) {
    light->unk43 = intensity;
    light->unk44 = intensity;
    if (light->unk6C != 0) {
        light->unk2 |= 2;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void lightUpdateLights(s32 updateRate) {
    s32 i;

    for (i = 0; i < D_80079494; i++) {
        func_80018F08(D_80079498[i], updateRate);
    }
}
/*
 * Workbench: 205/205 words, masked 41, first naming +0x20, frame 0x48.
 * Remaining: one lbu/sll swap at +0x64 and a t-reg ring phase (t5 vs t4).
 * PROVENANCE: control flow, Vec3f coordinate rows, nested flags/mode test,
 * and PY rotation layout adapted from JFG's public retail-derived
 * src/camlight.c::camlightUpdate and src/lights.c::func_80021434_22034.
 * Mickey's owner/segment offsets, colour-cycle helper, and track-light
 * calls remain authoritative.
 */
#ifdef NON_MATCHING
void func_80018F08(UnkLight *light, s32 updateRate) {
    s16 rotation[3];
    LightUpdateState *state;
    LightUpdateOwner *owner;

    state = (LightUpdateState *) light;
    owner = state->owner14;
    if ((owner != NULL) && (owner->disabled91 == 0)) {
        if (state->index6 >= 0) {
            LightUpdateSegment *segment;

            segment = owner->segments68[owner->segmentIndex3A];
            if (segment != NULL) {
                if (state->index6 < segment->header0->count2D) {
                    state->x18 = segment->coordinates40[state->index6].x;
                    state->y1C = segment->coordinates40[state->index6].y;
                    state->z20 = segment->coordinates40[state->index6].z;
                    state->flags2 |= 1;
                }
            }
        } else {
            pointListRPY(1, owner, &state->inputX08, &state->x18);
            state->x18 += owner->offsetX0C;
            state->y1C += owner->offsetY10;
            state->z20 += owner->offsetZ14;
            state->flags2 |= 1;
        }
        state->flags2 |= 4;
        if (state->trackLight6C != 0) {
            func_8000D7F8(state->trackLight6C, state->x18, state->y1C, state->z20);
        }
    }
    if (state->colourCycle54 != 0) {
        func_80036AB0((u8 *) state + 0x48, updateRate);
        state->red40 = *(u8 *) ((u8 *) state + 0x50);
        state->green41 = *(u8 *) ((u8 *) state + 0x51);
        state->blue42 = *(u8 *) ((u8 *) state + 0x52);
        if (!(state->flags3 & 0x10)) {
            state->intensity43 = *(u8 *) ((u8 *) state + 0x53);
            state->intensity44 = (f32) (u32) (*(u8 *) ((u8 *) state + 0x53) & 0xFF);
        }
        state->flags2 |= 2;
    }
    if (state->step5C != 0) {
        state->value58 = (s16) (state->value58 + (state->step5C * updateRate));
        state->flags2 |= 4;
    }
    if (state->step5E != 0) {
        state->value5A = (s16) (state->value5A + (state->step5E * updateRate));
        state->flags2 |= 4;
    }
    if ((state->flags2 & 1) || ((owner != NULL) && (owner->disabled91 == 0))) {
        state->lower38 = state->y1C - *(f32 *) ((u8 *) state + 0x28);
        state->upper3C = state->y1C + *(f32 *) ((u8 *) state + 0x28);
    }
    if (state->flags2 & 2) {
        lightCreateLightTable(state->red40, state->green41, state->blue42, state->table70);
        if (state->trackLight6C != 0) {
            func_8000D768(state->trackLight6C, state->red40, state->green41,
                          state->blue42, state->intensity43);
        }
    }
    if (state->flags2 & 4) {
        if ((state->mode0 == 3) || (state->mode0 == 0)) {
            state->directionZ68 = -1.0f;
            rotation[0] = state->value58;
            rotation[1] = state->value5A;
            rotation[2] = 0;
            mathOneFloatPY(rotation, &state->directionX60);
            if ((owner != NULL) && (owner->disabled91 == 0)) {
                pointListRPY(1, owner, &state->directionX60, &state->directionX60);
            }
            state->directionX60 = -state->directionX60;
            state->directionY64 = -state->directionY64;
            state->directionZ68 = -state->directionZ68;
        }
    }
    state->flags2 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018F08.s")
#endif
/* PROVENANCE: adapted from DKR's public decomp, src/lights.c, and Mickey's own assembly. */
void killLight(UnkLight *light) {
    s32 i;
    UnkLight *entry;

    entry = NULL;
    for (i = 0; (i < D_80079494) && (entry == NULL); i++) {
        if (light == D_80079498[i]) {
            entry = D_80079498[i];
        }
    }
    if (entry != NULL) {
        if (light->unk6C != 0) {
            func_8000D728(light->unk6C, i, D_80079494, (s32) entry);
        }
        D_80079494--;
        for (i--; i < D_80079494; i++) {
            D_80079498[i] = D_80079498[i + 1];
        }
        D_80079498[D_80079494] = entry;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void **lightGetLights(s32 *count) {
    *count = D_80079494;
    return D_80079498;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *lightGetStrongestEffect(f32 x, f32 y, f32 z) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquare;
    f32 effect;
    f32 strongestEffect;
    s32 index;
    s32 offset;
    UnkLight *light;
    UnkLight *strongestLight;

    strongestEffect = 0.0f;
    strongestLight = NULL;
    index = D_80079494;
    if (index--) {
        offset = index * 4; do {
            light = *(UnkLight **)((u8 *) D_80079498 + offset);
            if (light != NULL) {
                dx = x - light->x;
                dy = y - light->y;
                dz = z - light->z;
                distanceSquare = (dx * dx) + (dy * dy) + (dz * dz);
                if (distanceSquare < light->radiusSquare) {
                    effect = (1.0f - (sqrtf(distanceSquare) * light->radiusInverse)) * light->unk44;
                    if (strongestEffect < effect) {
                        strongestEffect = effect;
                        strongestLight = light;
                    }
                }
            }
            offset -= 4;
        } while (index--);
    }
    return strongestLight;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightUpdateObjects(void) {
    s32 objectLight;
    LightingObject **objects;
    LightingObject *object;
    s32 start;
    s32 end;

    objects = func_8000572C(&start, &end);
    if (start < end) {
        do {
            object = objects[start];
            start++;
            objectLight = object->objectLight;
            if ((objectLight != 0) &&
                (*(s8 *) (object->segmentData + object->segmentIndex + 0x1E) == 0) &&
                (object->unk8F == 0)) {
                func_8001953C(object, objectLight);
            }
        } while (start < end);
    }
}
/*
 * PROVENANCE: JFG's public `lightObject` assembly-only entry supplies the
 * role and TU context; Mickey's target bytes and resident object/light
 * layouts supply this source reconstruction.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch; 225 differing words, first mismatch +0x0. */
/* Target 254 instructions/frame -160; candidate 262 instructions/frame -128. */
/* Remaining gap is FP expression association and a 32-byte local-frame deficit; not shape-exact. */
void func_8001953C(LightingObject *arg0, s32 arg1) {
    f32 sp74;
    u8 *level;
    f32 distanceXZ;
    f32 distance;
    f32 directionEffect;
    f32 distanceEffect;
    f32 xDifference;
    f32 yDifference;
    f32 zDifference;
    s16 lightCount;
    s32 totalLightCount;
    s32 lightOffset;
    s32 slotOffset;
    s32 slotEnd;
    s32 maximumIntensity;
    s32 candidateIntensity;
    s32 type;
    s32 index;
    s32 endOffset;
    s32 intensity;
    s32 minimumIntensity;
    u8 *object;
    u8 *state;
    u8 *slot;
    u8 *nextSlot;
    UnkLight *light;

    object = (u8 *) arg0;
    state = (u8 *) (u32) arg1;
    *(s16 *) (state + 0xE) = 1;
    maximumIntensity = 0;
    level = levelGetLevel();
    totalLightCount = 0;
    if (D_80079494 > 0) {
        lightOffset = 0;
        slotEnd = 0x80;
        do {
            light = *(UnkLight **) ((u8 *) D_80079498 + lightOffset);
            if (light->unk3 & 1) {
                intensity = light->unk43;
                if (intensity != 0) {
                    type = light->unk0;
                    candidateIntensity = intensity;
                    if (type != 0) {
                        xDifference = light->x - *(f32 *) (object + 0xC);
                        zDifference = light->z - *(f32 *) (object + 0x14);
                        candidateIntensity = 0;
                        distance = *(f32 *) (object + 0x10);
                        yDifference = light->y - distance;
                        distanceXZ = (xDifference * xDifference) +
                                     (zDifference * zDifference);
                        switch (type) {
                        case 2:
                            if ((distanceXZ < light->radiusSquare) &&
                                (*(f32 *) ((u8 *) light + 0x38) < distance) &&
                                (distance <= *(f32 *) ((u8 *) light + 0x3C))) {
                                candidateIntensity = (s32) func_80019934(
                                    light->unk44, sqrtf(distanceXZ),
                                    *(f32 *) ((u8 *) light + 0x34), light->unk1);
                            }
                            break;
                        case 3:
                            distance = distanceXZ +
                                       (yDifference * yDifference);
                            if (distance < light->radiusSquare) {
                                distanceEffect = sqrtf(distance);
                                directionEffect = lightDirectionCalc(
                                    *(f32 *) ((u8 *) light + 0x60),
                                    *(f32 *) ((u8 *) light + 0x64),
                                    *(f32 *) ((u8 *) light + 0x68),
                                    xDifference, yDifference, zDifference,
                                    distanceEffect);
                                if (directionEffect > 0.0f) {
                                    sp74 = directionEffect;
                                    candidateIntensity = (s32) (
                                        func_80019934(
                                            light->unk44, distanceEffect,
                                            *(f32 *) ((u8 *) light + 0x34),
                                            light->unk1) * directionEffect);
                                }
                            }
                            break;
                        default:
                            distance = distanceXZ +
                                       (yDifference * yDifference);
                            if (distance < light->radiusSquare) {
                                candidateIntensity = (s32) func_80019934(
                                    light->unk44, sqrtf(distance),
                                    *(f32 *) ((u8 *) light + 0x34), light->unk1);
                            }
                            break;
                        }
                    } else {
                        xDifference = *(f32 *) ((u8 *) light + 0x60);
                        yDifference = *(f32 *) ((u8 *) light + 0x64);
                        zDifference = *(f32 *) ((u8 *) light + 0x68);
                    }
                    if (candidateIntensity != 0) {
                        lightCount = *(s16 *) (state + 0xE);
                        slot = state + 0x30;
                        if ((light->unk3 & 0x20) &&
                            (maximumIntensity < candidateIntensity)) {
                            maximumIntensity = candidateIntensity;
                        }
                        slotOffset = 0x40;
                        if (lightCount < 4) {
                            slot = state + (lightCount << 5) + 0x10;
                            *(s16 *) (state + 0xE) = lightCount + 1;
                        } else {
                            minimumIntensity = *(u8 *) (state + 0x45);
                            nextSlot = state + 0x40;
                            do {
                                intensity = *(u8 *) (nextSlot + 0x25);
                                slotOffset += 0x20;
                                if (intensity < minimumIntensity) {
                                    slot = nextSlot + 0x10;
                                    minimumIntensity = intensity;
                                }
                                nextSlot += 0x20;
                            } while (slotOffset < slotEnd);
                            if (*(u8 *) (slot + 0x15) >= candidateIntensity) {
                                slot = NULL;
                            }
                        }
                        if (slot != NULL) {
                            *(f32 *) (slot + 0x0) = xDifference;
                            *(f32 *) (slot + 0x4) = yDifference;
                            *(f32 *) (slot + 0x8) = zDifference;
                            *(u8 *) (slot + 0x14) = 0;
                            *(u8 *) (slot + 0x15) = candidateIntensity;
                            *(s32 *) (slot + 0xC) =
                                (1 << (8 - *(u8 *) (state + 0x24))) << 6;
                            *(u8 *) (slot + 0x17) = 0;
                            *(u8 *) (slot + 0x16) = candidateIntensity;
                            *(s32 *) (slot + 0x10) =
                                candidateIntensity << *(u8 *) (state + 0x24);
                            *(s32 *) (slot + 0x18) =
                                *(s32 *) ((u8 *) light + 0x70);
                        }
                    }
                }
            }
            totalLightCount++;
            lightOffset += 4;
        } while (totalLightCount < D_80079494);
    }
    index = 0x13F - maximumIntensity;
    if (*(u8 *) (level + 0xE3) != 0) {
        intensity = *(u8 *) (level + 0xE0);
        endOffset = *(u8 *) (level + 0xDF) - intensity;
    } else {
        intensity = *(u8 *) (state + 0xD);
        endOffset = *(u8 *) (state + 0xC) - intensity;
    }
    if (maximumIntensity >= 0x40) {
        intensity = (index * intensity) >> 8;
        endOffset = (index * endOffset) >> 8;
    }
    *(s8 *) (state + 0x25) = intensity + endOffset;
    *(s8 *) (state + 0x26) = endOffset;
    *(s8 *) (state + 0x27) = intensity;
    *(s32 *) (state + 0x20) = endOffset << *(u8 *) (state + 0x24);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001953C.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey's trigonometry helper. */
f32 func_80019934(f32 arg0, f32 arg1, f32 arg2, s32 arg3) {
    f32 temp;

    temp = arg1 * arg2;
    switch (arg3) {
        case 1:
            temp = 1.0f - temp;
            break;
        case 2:
            temp = 1.0f - sqrtf(temp);
            break;
        case 3:
            temp = func_8002A8BC(temp * 16384.0f);
            break;
        case 4:
            temp = func_8002A8BC(temp * 16384.0f);
            temp *= temp;
            break;
        case 5:
            temp = 1.0f - temp;
            temp *= temp;
            break;
    }
    return arg0 * temp;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
f32 lightDirectionCalc(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6) {
    f32 temp_f0;
    f32 var_f2;

    if (arg6 > 0.0f) {
        temp_f0 = 1.0f / arg6;
        var_f2 = (arg3 * temp_f0 * arg0) + (arg4 * temp_f0 * arg1) + (arg5 * temp_f0 * arg2);
        if (var_f2 < 0.0f) {
            var_f2 = 0.0f;
        }
    } else {
        var_f2 = 1.0f;
    }
    return var_f2;
}
/* 63 masked words at size delta 0, frame 0xC8 with the target's slot ladder,
 * first mismatch +0x80 (was 155 at delta -4). Track B, 2026-09-23:
 *   - lightData is assigned once before the scale test and again after it.
 *     The first definition is dead on the unscaled path, so uopt sinks it
 *     into the scaled branch; the target computes description + 0x10 in both
 *     places and addresses the branch through it. One pointer declared once
 *     was hoisted above the test and folded into the offsets: the -4. 75.
 *   - declarations ordered so count, the four saved values and changed take
 *     the top homes, local sits at 0x70 and cameraDelta at 0x50: 63, and the
 *     immediate bucket is empty.
 * Left, 57 naming rows and one moved word: the three single-block CSE temps
 * of the scaling arithmetic (web numbers 78, 86 and 88 on this shape, save
 * 3 each) are coloured v0, v1 and a0 here where the target leaves them in
 * ring temps, and the ring phase differs downstream (t0-t4 against t5-t9); and
 * the changed = 1 constant is drawn later than the target draws it. */
/* PROVENANCE: JFG's corresponding object-light routine supplies the role and dispatch idiom; Mickey's fields, globals, and calls are authoritative below. */
#ifdef NON_MATCHING
void func_80019AB8(LightPosition *position, LightObjectContext *object,
                   LightDescription *description, f32 *matrix) {
    s32 count;
    s32 savedRed;
    s32 savedGreen;
    s32 savedBlue;
    s32 savedPacked;
    s32 changed;
    f32 local[4][4];
    LightData *lightData;
    f32 scale;
    f32 factor;
    s32 redValue;
    s32 greenValue;
    f32 cameraDelta[3];
    LightObjectState *state;
    s16 jointCount;

    state = object->state0;
    if ((description != NULL) || (state->environment64 != 0)) {
        count = state->mode4F;
        if (count <= 0) {
            if (matrix == NULL) {
                func_8002AA50(position, local);
                matrix = local;
            }
            count = 1;
        }
        if (description != NULL) {
            changed = 0;
            scale = description->scale0;
            lightData = (LightData *) ((u8 *) description + 0x10);
            if ((1.0f != scale) ||
                ((D_8007C854 != 0) && (D_8007C85C != 0xFF))) {
                savedRed = lightData->red15;
                savedGreen = lightData->green17;
                savedBlue = lightData->blue16;
                factor = scale * ((f32) D_8007C85C * D_800817C8);
                savedPacked = lightData->packed10;
                redValue = (s32) ((f32) savedRed * factor);
                lightData->red15 = (u8) redValue;
                greenValue = (s32) ((f32) savedGreen * factor);
                lightData->blue16 = (u8) (redValue - greenValue);
                lightData->packed10 =
                    ((redValue - greenValue) & 0xFF) << lightData->shift14;
                lightData->green17 = (u8) greenValue;
                changed = 1;
            }
            jointCount = description->jointCountE;
            lightData = (LightData *) ((u8 *) description + 0x10);
            if (jointCount > 0) {
                lightMakeJointPositions(jointCount, (f32 *) lightData,
                                        count, matrix, D_800794A0);
                if (description->jointCountE == 1) {
                    lightSingleLight(object->objectId4, state->value60,
                                     (f32 *) lightData, D_800794A0, state->value5C);
                } else if (description->jointCountE == 2) {
                    lightTwoLights(object->objectId4, state->value60,
                                   (s32) D_800794A0, count, (f32 *) lightData,
                                   (u8 *) description + 0x30, state->value5C);
                } else {
                    lightMultiLight(object->objectId4, state->value60,
                                    (f32 *) lightData, state->value5C,
                                    (s32) D_800794A0, count,
                                    description->jointCountE);
                }
            }
            if (changed != 0) {
                lightData->red15 = savedRed;
                lightData->green17 = savedGreen;
                lightData->blue16 = savedBlue;
                lightData->packed10 = savedPacked;
            }
        }
        if (state->environment64 != 0) {
            LightCameraPosition *camera;

            camera = camGetPtr();
            cameraDelta[0] = camera->x0C - position->x0C;
            cameraDelta[1] = camera->y10 - position->y10;
            cameraDelta[2] = camera->z14 - position->z14;
            lightMakeJointPositions(1, cameraDelta, count, matrix, D_800794A0);
            lightEnviroMap(state->environment20, state->environment64,
                           state->value5C, (s32) D_800794A0);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019AB8.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightDefaultObjectLight(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s32 arg4) {
    func_80019DE8(&D_800CB298, arg0, arg1, arg2, arg3, arg4);
}
/* PROVENANCE: JFG's public assembly-backed lightSetObjectLight authenticates
 * the structural role only; Mickey's body and globals remain authoritative. */
void func_80019DE8(ObjectLightState *state, s32 arg1, s32 arg2, s16 pitch, s16 yaw, s32 shift) {
    s16 rotation[3];
    f32 direction[3];

    arg1 &= 0xFF;
    arg2 &= 0xFF;
    shift &= 7;
    if (arg2 < arg1) {
        arg1 = arg2;
    }
    state->startValue = arg1;
    state->endValue = arg2;
    state->shift = shift;
    state->valueDelta = state->endValue - state->startValue;
    state->colourStep = state->valueDelta << state->shift;
    state->scaleStep = (1 << (8 - shift)) << 6;
    state->pitch = pitch;
    state->yaw = yaw;
    rotation[1] = pitch;
    rotation[2] = 0;
    rotation[0] = yaw;
    direction[0] = 0.0f;
    direction[1] = 0.0f;
    direction[2] = 1.0f;
    mathOneFloatRPY(rotation, direction);
    state->directionX = direction[0];
    state->directionY = direction[1];
    state->directionZ = direction[2];
    state->table = D_800CB290;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey offsets. */
void lightSetupLightSources(LightSourceObject *object) {
    s32 i;

    for (i = 0; i < object->header->lightCount; i++) {
        object->lights[i] = addObjectLight((s32) object, &object->header->lights[i]);
    }
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightSetupFlareSources(FlareObject *object) {
    s32 i;

    for (i = 0; i < object->header->flareCount; i++) {
        object->flares[i] = camlightAdd(object, &object->header->flares[i]);
    }
}
/* PROVENANCE: adapted from JFG's public asm/nonmatchings/lights/lightInitObjectLighting.s, with Mickey's layout. */
s32 func_8001A008(LightingObject *object, LightInitState *state) {
    s32 result;
    LightLevelData *level;
    s32 mode;
    LightInitHeader *header;

    level = (LightLevelData *) levelGetLevel();
    result = 0;
    header = (LightInitHeader *) object->segmentData;
    object->objectLight = (s32) state;
    mode = header->mode;
    if (mode == 0) {
        state->enabled = 1;
        result = 0x90;
        if (level->useLevelLight != 0) {
            func_80019DE8(&state->light, level->red, level->green, level->yaw,
                          level->pitch + 0x8000, 0);
        } else {
            func_80019DE8(&state->light, (s32) (header->red * 255.0f),
                          (s32) (header->green * 255.0f), header->yaw,
                          header->pitch, header->shift);
        }
        state->unk4 = 0;
        state->unk5 = 0xFF;
        state->unk6 = 0xFF;
        state->unk7 = 0xFF;
        state->unk8 = 0xFF;
        state->unk9 = 0xFF;
        state->unkA = 0xFF;
        state->intensity = 1.0f;
    } else if ((mode == 1) || (mode == 2)) {
        state->unk4 = 0;
        state->unk5 = 0xFF;
        state->unk6 = 0xFF;
        state->unk7 = 0xFF;
        result = 8;
        state->intensity = 1.0f;
    }
    return (result & ~3) + 4;
}
/* PROVENANCE: adapted from JFG's public asm/nonmatchings/lights/lightAdjustGlowingLight.s, with Mickey's constants and offsets. */
void func_8001A154(GlowObject *object) {
    FlareEntry flare;
    GlowEntry *entry;
    s32 scaledSize;

    entry = object->entry;
    flare.type = 0x41;
    flare.subtype = entry->subtype;
    flare.x = entry->x;
    flare.y = entry->y;
    flare.z = entry->z;
    flare.red = entry->red;
    flare.green = entry->green;
    flare.blue = entry->blue;
    flare.alpha = entry->alpha;
    flare.kind = 0x2B;
    flare.size = entry->size;
    scaledSize = flare.scaledSize = (entry->size * entry->scale) / 100;
    flare.index = -1;
    flare.enabled = 0;
    object->flare = camlightAdd(NULL, &flare);
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
s32 lightKillGlowingLight(void) {
    camlightDelete();
    return 1;
}

/* PLATEAU-HANDOFF:func_80018F08:start
 * symbol: func_80018F08
 * score: 41/205 words
 * frame: 0x48
 * relocations: 7
 * first-mismatch: +0x20
 * summary: Header as v0 symbol serializes the +0x64 pair. A second extra auto costs 8 frame. Sole-header is frame-exact plus 10 words. Colour cannot close it.
 * PLATEAU-HANDOFF:func_80018F08:end
 */

/* PLATEAU-HANDOFF:func_80019AB8:start
 * symbol: func_80019AB8
 * score: 63/184 words
 * frame: 0xC8
 * relocations: 28
 * first-mismatch: +0x80
 * summary: Sunk partially dead lightData def closes the -4; target homes give 63 at delta 0; three single-block CSE temps take v0/v1/a0 not ring temps
 * PLATEAU-HANDOFF:func_80019AB8:end
 */

/* PLATEAU-HANDOFF:func_8001953C:start
 * symbol: func_8001953C
 * score: 225 differing words
 * frame: 0x80
 * relocations: 14
 * first-mismatch: +0x0
 * summary: Target is 254 words/frame 0xA0 versus 262/0x80. Relocation identities stay ordered; next recover typed local layout, then FP expression association.
 * PLATEAU-HANDOFF:func_8001953C:end
 */
