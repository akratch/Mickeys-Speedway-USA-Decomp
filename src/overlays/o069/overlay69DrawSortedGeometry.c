#include "PR/ultratypes.h"

/*
 * Overlay 88 ships this renderer byte for byte: its TU renames this file's
 * function and callee symbols to its own and includes it, so one body serves
 * both modules.
 */

#define SHARED_SET_ENV_WHITE_ZERO_ALPHA(packet)             \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0xFB000000U;                      \
        macroCommand->w1 = 0xFFFFFF00U;                      \
    }

#define SHARED_LOAD_FIXED_GEOMETRY(packet, geometry)         \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0x01810040U;                      \
        macroCommand->w1 = (u32)(geometry);                  \
    }

#define SHARED_APPEND_TRAILING_STATE(packet)                 \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0xBC00000AU;                     \
        macroCommand->w1 = 0;                               \
    }

typedef struct SharedCommand {
    u32 w0;
    u32 w1;
} SharedCommand;

typedef struct SharedVec3 {
    f32 x;
    f32 y;
    f32 z;
} SharedVec3;

typedef struct SharedDynamicEntry {
    void *payload;
    s8 vectorIndex;
    u8 reserved05[3];
    f32 scale;
    u8 reserved0C[8];
} SharedDynamicEntry;

typedef struct SharedResourceSet {
    u8 reserved00[0x0A];
    s16 geometryGroup;
    void *geometryBases[13];
    SharedVec3 *vectors;
} SharedResourceSet;

typedef struct SharedDrawState {
    s16 dynamicHalf0[4];
    s16 dynamicHalf8[4];
    void *fixedResources[4];
    void *fixedRefs[4];
    u8 reserved30[4];
    u8 fixedActive[4];
    s8 fixedVectorIndex[4];
    s8 fixedGeometryIndex[4];
    u8 reserved40[4];
    SharedVec3 position;
    s16 angle;
} SharedDrawState;

typedef struct SharedGateConfig {
    f32 scale;
    u8 reserved04[0x1A];
    s8 suppressBySelector[1];
} SharedGateConfig;

typedef struct SharedRenderObject {
    u8 reserved00[2];
    s16 angle2;
    s16 angle4;
    s16 flags6;
    f32 scale8;
    u8 reserved0C[0x2D];
    u8 submitFlags39;
    u8 reserved3A[6];
    SharedGateConfig *gate;
    u8 reserved44[0x0C];
    void *submitResource50;
    u8 reserved54[0x0C];
    SharedDynamicEntry *dynamicEntries;
    SharedDrawState *drawState;
    SharedResourceSet **resourcesBySelector;
    u8 reserved6C[0x20];
    u8 dynamicCount;
    u8 reserved8D[6];
    u8 selector;
} SharedRenderObject;

typedef struct SharedTransform {
    s16 stateAngle;
    s16 objectAngle2;
    s16 objectAngle4;
    s16 reserved06;
    f32 objectScale;
    SharedVec3 position;
} SharedTransform;

typedef struct SharedDynamicSubmit {
    s16 half0;
    s16 half2;
    s16 untouched4;
    s16 mode6;
    f32 scale8;
    f32 oneC;
    SharedVec3 vector;
    s32 constant1C;
    void *payload20;
} SharedDynamicSubmit;

extern void overlay69DrawFixedResourceReloc(SharedCommand **commands,
                                        void *resource);
extern f32 overlay69MetricReloc(f32 x, f32 y, f32 z);
extern void overlay69PrepareTransformReloc(SharedTransform *transform);
extern void overlay69SubmitDynamicReloc(
    SharedCommand **commands, void *renderArg1, void *renderArg2,
    SharedTransform *transform, void *objectResource,
    SharedDynamicSubmit *submit, s32 mode, u8 flags);
extern void overlay69DrawConeReloc(SharedCommand **commands,
                                      void *reference, s32 mode, u8 key);

/* DKR v77/v80 and JFG have no exact donor for this renderer. */
/*
 * Rewritten 2026-10-02 from the listing (lane x-sort): 140 -> 57 masked words
 * at size delta 0 and the target's 0x148 frame.  What moved it: two scalar
 * homes above order[] instead of four (the frame), the sort's swap through
 * one temporary instead of left/right carriers (one ring draw per inner
 * iteration, 130 -> 63), count++ before i++ via a for-loop header on the
 * dynamic collect loop, and the entry pointer formed before its index slot.
 * The remaining residual starts in the fixed collect block: the target
 * recomputes sp+count*4 for the geometry store in a fresh ring register,
 * which reads as one more ring draw between the reference store and the
 * geometry store than this source spends.  Ordering the stores refs, keys,
 * geometry keeps size delta 0; refs, geometry, keys (the target's emission
 * order) lets as1 delete the repeated address and loses four bytes.
 */
#ifdef NON_MATCHING
void overlay69DrawSortedGeometry(SharedCommand **commands, void *renderArg1,
                          void *renderArg2, SharedRenderObject *object) {
    SharedDrawState *state;
    SharedResourceSet *resources;
    s16 order[8];
    s16 fixedKeys[8];
    f32 inverseScale;
    SharedTransform transform;
    SharedDynamicSubmit submit;
    void *fixedGeometry[8];
    f32 metrics[8];
    void *reference;
    void *fixedRefs[8];
    SharedDynamicEntry *entry;
    SharedVec3 *vector;
    s16 count;
    s16 i;
    s16 j;
    s16 left;
    s16 right;
    s16 slot;

    if (object->flags6 & 0x400) {
        return;
    }

    state = object->drawState;
    resources = object->resourcesBySelector[object->selector];

    SHARED_SET_ENV_WHITE_ZERO_ALPHA((*commands)++);

    for (i = 0; i < 4; i++) {
        if (state->fixedResources[i] != NULL) {
            overlay69DrawFixedResourceReloc(commands, state->fixedResources[i]);
        }
    }
    entry = object->dynamicEntries;
    if (entry != NULL) {
        if (!object->gate->suppressBySelector[object->selector]) {
            for (i = 0, count = 0; (i < object->dynamicCount) && (i < 8); i++) {
                vector = &resources->vectors[entry->vectorIndex];
                metrics[count] = overlay69MetricReloc(vector->x, vector->y, vector->z);
                order[count] = count;
                count++;
                entry++;
            }

            for (i = count - 1; i > 0; i--) {
                for (j = 0; j < i; j++) {
                    if (metrics[order[j + 1]] < metrics[order[j]]) {
                        left = order[j];
                        order[j] = order[j + 1];
                        order[j + 1] = left;
                    }
                }
            }

            transform.stateAngle = state->angle;
            transform.objectAngle2 = object->angle2;
            transform.objectAngle4 = object->angle4;
            transform.objectScale = object->scale8;
            transform.position.x = state->position.x;
            transform.position.y = state->position.y;
            transform.position.z = state->position.z;
            overlay69PrepareTransformReloc(&transform);

            inverseScale = 1.0f / object->gate->scale;
            submit.mode6 = 3;
            submit.constant1C = 0x3333;
            for (i = 0; i < count; i++) {
                entry = &object->dynamicEntries[order[i]];
                slot = order[i];
                vector = &resources->vectors[entry->vectorIndex];
                submit.half0 = state->dynamicHalf0[slot];
                submit.half2 = state->dynamicHalf8[slot];
                submit.scale8 = entry->scale * inverseScale;
                submit.vector.x = vector->x;
                submit.vector.y = vector->y;
                submit.vector.z = vector->z;
                submit.oneC = 1.0f;
                submit.payload20 = entry->payload;
                overlay69SubmitDynamicReloc(commands, renderArg1, renderArg2,
                                            &transform, object->submitResource50,
                                            &submit, 0xE, object->submitFlags39);
            }
        }
    }

    if (object->gate->suppressBySelector[object->selector]) {
        return;
    }

    i = 0;
    count = 0;
    while (i < 4) {
        if ((state->fixedActive[i] != 0) &&
            ((reference = state->fixedRefs[i]) != NULL)) {
            vector = &resources->vectors[state->fixedVectorIndex[i]];
            order[count] = count;
            metrics[count] = overlay69MetricReloc(vector->x, vector->y, vector->z);
            fixedRefs[count] = reference;
            fixedKeys[count] = state->fixedActive[i];
            fixedGeometry[count] =
                (u8 *)resources->geometryBases[resources->geometryGroup] +
                (state->fixedGeometryIndex[i] * 64);
            count++;
        }
        i++;
    }

    if (count > 0) {
        for (i = count - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                if (metrics[order[j + 1]] < metrics[order[j]]) {
                    left = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = left;
                }
            }
        }

        for (i = 0; i < count; i++) {
            SHARED_LOAD_FIXED_GEOMETRY((*commands)++,
                (void *)((u32)fixedGeometry[order[i]] + 0x80000000U));
            overlay69DrawConeReloc(commands, fixedRefs[order[i]], 6,
                                      (u8)fixedKeys[order[i]]);
        }

        SHARED_APPEND_TRAILING_STATE((*commands)++);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o069/overlay69DrawSortedGeometry/func_overlay_069_F0000170_18C8BD8.s")
#endif

/* PLATEAU-HANDOFF:overlay69DrawSortedGeometry:start
 * symbol: overlay69DrawSortedGeometry
 * score: 57/359 words
 * frame: 0x148
 * relocations: 6
 * first-mismatch: +0x3DC
 * summary: Listing rewrite at the target frame. Exact up to the fixed collect block, where the geometry store needs one more ring draw.
 * PLATEAU-HANDOFF:overlay69DrawSortedGeometry:end
 */
