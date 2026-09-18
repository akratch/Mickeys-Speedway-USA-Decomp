#include "PR/ultratypes.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct Overlay43Node {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[9];
    u8 priority;
    u8 pad16[0xA];
} Overlay43Node;

typedef struct Overlay43NodeSet {
    u8 pad00[0xE];
    s16 count;
    Overlay43Node nodes[1];
} Overlay43NodeSet;

typedef struct Overlay43SortRecord {
    s32 active;
    u8 pad04[0x10];
    f32 value14;
    u8 pad18[0x28];
    Overlay43Node *node;
    s32 owner;
} Overlay43SortRecord;

typedef struct Overlay43VertexBound {
    u8 pad00[8];
    f32 radius;
} Overlay43VertexBound;

typedef struct Overlay43ModelDefinition {
    u8 pad00[0x2F];
    u8 vertexCount;
    u8 pad30[8];
    Overlay43VertexBound bounds[1];
} Overlay43ModelDefinition;

typedef struct Overlay43VertexPosition {
    f32 x;
    f32 y;
    f32 z;
} Overlay43VertexPosition;

typedef struct Overlay43ModelInstance {
    Overlay43ModelDefinition *definition;
    u8 pad04[4];
    s16 active;
    u8 pad0A[0x3E];
    Overlay43VertexPosition *positions;
} Overlay43ModelInstance;

typedef struct Overlay43State {
    f32 scaleX;
    f32 scaleY;
    u8 pad08[9];
    u8 maximumRecords;
    u8 pad12[0x1E];
    void *displayListEnd;
    u8 pad34[0x2C];
    Overlay43ModelInstance *model;
    u8 pad64[0x48];
    f32 fallbackScale;
    f32 diameter;
    s16 angle0;
    s16 angle1;
    u8 padB8;
    u8 pending;
    u8 boundsMode;
} Overlay43State;

typedef struct Overlay43Link {
    u8 pad00[0x1C];
    Overlay43State *state;
} Overlay43Link;

typedef struct Overlay43ScaleSource {
    u8 pad00[0x5C];
    f32 scale;
} Overlay43ScaleSource;

typedef struct Overlay43Input {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x28];
    Overlay43ScaleSource *scaleSource;
    u8 pad44[8];
    Overlay43Link *link;
    Overlay43NodeSet *nodeSet;
} Overlay43Input;

typedef struct Overlay43CloneState {
    u8 pad00[0xD8];
    s16 angle0;
    s16 angle1;
    u8 padDC[6];
    u8 priority;
} Overlay43CloneState;

typedef struct Overlay43Projection {
    f32 x;
    f32 unused;
    f32 z;
} Overlay43Projection;

extern s8 D_C8;
extern Overlay43State *D_120[];
extern s32 D_ACTIVE;
extern Overlay43SortRecord D_FALLBACK;
extern Overlay43SortRecord D_D8[];
extern f32 D_20;
extern void *levelGetLevel(void);
extern s16 overlay43Atan2(f32 x, f32 y);
extern f32 overlay43Sqrt(f32 value);
extern s16 overlay43Interpolate(s16 from, s16 to, f32 fraction);
extern void overlay43ProcessRecord(s16 *angles, u8 priority,
                                   Overlay43SortRecord *record);
extern void overlay43PrepareRecord(Overlay43SortRecord *record);
extern void overlay43MakeAngles(s16 *angles, s16 *stateAngles);
extern void overlay43MakeMatrix(s16 *angles, Overlay43SortRecord *record,
                                f32 *matrix);
extern void overlay43ProjectPoint(f32 *matrix, f32 x, f32 y, f32 z,
                                  f32 *outX, f32 *outY, f32 *outZ);
extern void overlay43NormalizeAngles(s16 *angles);
extern void overlay43ScaleMatrix(s16 *angles, f32 *matrix,
                                 s16 *stateAngles);
extern void func_overlay_043_F0000BE4_188ABB4(
    Overlay43Input *input, Overlay43SortRecord **records, s32 recordCount);

/* NON_MATCHING: L144 homes a0, 0-arg levelGetLevel, boundsMode==2, same-temp
 * scaleX/Y. 559/560 words, exact 0x178 frame, 41 slots, masked 494, first
 * structural +0xB8. One-word size deficit remains. */
#ifdef NON_MATCHING
s32 func_overlay_043_F0000324_188A2F4(
    Overlay43Input *input,
    s32 useNodes,
    void *unused) {
    Overlay43ModelDefinition *definition;
    Overlay43CloneState *clone;
    Overlay43State *state;
    Overlay43ModelInstance *model;
    Overlay43NodeSet *nodeSet;
    Overlay43SortRecord *record;
    Overlay43SortRecord *swap;
    Overlay43Node *node;
    Overlay43SortRecord *records[8];
    s16 angles[3];
    s16 angle0;
    s16 angle1;
    f32 matrix[15];
    f32 position[3];
    f32 extent;
    f32 fraction;
    f32 value;
    s32 recordCount;
    s32 pass;
    s32 index;
    s32 limit;
    s16 *stateAngles;

    (void)unused;
    /* L144: address-form homes incoming a0 at +0x178; volatile overshoots. */
    input = *(Overlay43Input **)&input;
    state = input->link->state;
    model = state->model;
    if (model->active != 0) {
        if (state->displayListEnd == 0) {
            return 0;
        }
        state->pending = 2;
        D_ACTIVE = 1;
        D_120[D_C8] = state;
        D_C8++;
        return 1;
    }
    definition = model->definition;
    clone = (Overlay43CloneState *)levelGetLevel();
    recordCount = 0;

    if (useNodes != 0) {
        nodeSet = input->nodeSet;
        if ((nodeSet != 0) && (nodeSet->count >= 2)) {
            for (index = 1; index < nodeSet->count; index++) {
                D_D8[recordCount].node = &nodeSet->nodes[index];
                recordCount++;
            }

            for (index = 0; index < recordCount; index++) {
                records[index] = &D_D8[index];
            }

            for (pass = recordCount - 1; pass > 0; pass--) {
                for (index = 0; index < pass; index++) {
                    if (records[index]->node->priority <
                        records[index + 1]->node->priority) {
                        swap = records[index];
                        records[index] = records[index + 1];
                        records[index + 1] = swap;
                    }
                }
            }

            limit = state->maximumRecords;
            if (limit < recordCount) {
                recordCount = limit;
            }

            if (recordCount != 0) {
                node = records[0]->node;
                angle0 = overlay43Atan2(-node->x, -node->z);
                value = overlay43Sqrt(
                    (node->x * node->x) + (node->z * node->z));
                angle1 = overlay43Atan2(node->y, value);

                if (node->priority < clone->priority) {
                    fraction = (1.0f / (f32)clone->priority) *
                               (f32)node->priority;
                    angle0 = overlay43Interpolate(
                        clone->angle0, angle0, fraction);
                    angle1 = overlay43Interpolate(
                        clone->angle1, angle1, fraction);
                    angle0 = overlay43Interpolate(
                        angle0, state->angle0, fraction);
                    angle1 = overlay43Interpolate(
                        angle1, state->angle1, fraction);
                    state->angle0 = angle0;
                    state->angle1 = angle1;
                    limit = clone->priority;
                } else {
                    state->angle0 = angle0;
                    state->angle1 = angle1;
                    limit = node->priority;
                }

                overlay43ProcessRecord(&angle0, limit, records[0]);
                for (index = 1; index < recordCount; index++) {
                    node = records[index]->node;
                    angle0 = overlay43Atan2(-node->x, -node->z);
                    value = overlay43Sqrt(
                        (node->x * node->x) + (node->z * node->z));
                    angle1 = overlay43Atan2(node->y, value);
                    overlay43ProcessRecord(
                        &angle0, node->priority, records[index]);
                }
            }
        }
        if ((recordCount == 0) && (clone->priority > 0)) {
            angle0 = clone->angle0;
            angle1 = clone->angle1;
            state->angle0 = angle0;
            state->angle1 = angle1;
            records[0] = NULL;
            recordCount = 1;
            overlay43ProcessRecord(
                &angle0, clone->priority, NULL);
        }
    } else {
        if (clone->priority == 0) {
            return 0;
        }
        records[0] = &D_FALLBACK;
        recordCount = 1;
        D_FALLBACK.owner = clone->priority;
        overlay43PrepareRecord(&D_FALLBACK);
        D_FALLBACK.value14 = 0.0f;
    }

    angles[0] = 0;
    angles[1] = 0;
    angles[2] = 0;
    position[0] = -input->x;
    position[1] = -input->y;
    position[2] = -input->z;
    stateAngles = (s16 *)((u8 *)state + 0x6C);
    overlay43MakeAngles(angles, stateAngles);

    if (state->boundsMode == 0) {
        extent = input->scaleSource->scale * input->scale;
    } else if (state->boundsMode == 1) {
        Overlay43Projection projected;
        f32 maximumX;
        f32 minimumX;
        f32 maximumZ;
        f32 minimumZ;
        f32 diameter;
        f32 radius;
        s32 vertexIndex;

        maximumX = -32000.0f;
        minimumX = 32000.0f;
        maximumZ = -32000.0f;
        minimumZ = 32000.0f;
        for (index = 0; index < recordCount; index++) {
            overlay43MakeMatrix(stateAngles, records[index], matrix);
            for (vertexIndex = 0;
                 vertexIndex < definition->vertexCount;
                 vertexIndex++) {
                radius = definition->bounds[vertexIndex].radius * input->scale;
                overlay43ProjectPoint(
                    matrix,
                    model->positions[vertexIndex].x,
                    model->positions[vertexIndex].y,
                    model->positions[vertexIndex].z,
                    &projected.x,
                    &projected.unused,
                    &projected.z);
                value = projected.x + radius;
                if (maximumX < value) {
                    maximumX = value;
                }
                value = projected.x - radius;
                if (value < minimumX) {
                    minimumX = value;
                }
                value = projected.z + radius;
                if (maximumZ < value) {
                    maximumZ = value;
                }
                value = projected.z - radius;
                if (value < minimumZ) {
                    minimumZ = value;
                }
            }
        }
        if (minimumX < 0.0f) {
            minimumX = -minimumX;
        }
        if (minimumZ < 0.0f) {
            minimumZ = -minimumZ;
        }
        if (maximumX < minimumX) {
            maximumX = minimumX;
        }
        if (maximumZ < minimumZ) {
            maximumZ = minimumZ;
        }
        extent = ((maximumZ < maximumX) ? maximumX : maximumZ) + 10.0f;
        diameter = extent * 2.0f;
        value = ((100.0f / state->diameter) * diameter) - 100.0f;
        if ((value > -5.0f) && (value < 5.0f)) {
            diameter = state->diameter;
        } else {
            state->diameter = diameter;
        }
        value = D_20 * diameter;
        extent = 64.0f / diameter;
        state->scaleX = value;
        state->scaleY = value;
    } else if (state->boundsMode == 2) {
        extent = state->fallbackScale;
    }

    overlay43NormalizeAngles(angles);
    matrix[0] = extent;
    matrix[5] = extent;
    matrix[10] = extent;
    overlay43ScaleMatrix(stateAngles, matrix, stateAngles);
    func_overlay_043_F0000BE4_188ABB4(input, records, recordCount);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/func_overlay_043_F0000324_188A2F4/func_overlay_043_F0000324_188A2F4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_043_F0000324_188A2F4:start
 * symbol: func_overlay_043_F0000324_188A2F4
 * score: 529/560 words
 * frame: 0x178
 * relocations: 41
 * first-mismatch: +0x40
 * summary: V0 is 3 words short (557/560) with broad structural/register drift; relocations are 41 vs 43 with 5 aligned sites and no stable identities.
 * PLATEAU-HANDOFF:func_overlay_043_F0000324_188A2F4:end
 */
