#include "PR/ultratypes.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef f32 MtxF[4][4];

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

typedef struct Overlay43Record {
    MtxF matrix;
    Overlay43Node *node;
    s32 alpha;
} Overlay43Record;

typedef struct Overlay43VertexBound {
    u8 pad00[8];
    f32 radius;
} Overlay43VertexBound;

typedef struct Overlay43ModelDefinition {
    u8 pad00[0x2F];
    u8 vertexCount;
    u8 pad30[8];
    Overlay43VertexBound *bounds;
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
    u8 pad64[8];
    MtxF transform;
    f32 fallbackScale;
    f32 diameter;
    s16 yaw;
    s16 pitch;
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

typedef struct Overlay43Level {
    u8 pad00[0xD8];
    s16 yaw;
    s16 pitch;
    u8 padDC[6];
    u8 priority;
} Overlay43Level;

typedef struct Overlay43Transform {
    s16 yaw;
    s16 pitch;
    s16 roll;
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay43Transform;

/*
 * Overlay 43 ("clone.c"): build the per-node projection records for one
 * clone and size its 64x64 image, then hand the records to the renderer.
 *
 * Matched from 494 masked words at -4 by rewriting the inherited m2c shape
 * from the listing and the relocation records:
 *  - the record array and the queue are their own array symbols (the queue
 *    store is `symbol(index)`, not a base register), and the fallback's
 *    alpha is a direct store to element 0 by its own name, which is the
 *    word the old candidate was short;
 *  - the fallback record is read back from records[0] into a local before
 *    the identity call, so the copy that survives the call is a register
 *    move; alpha is stored before the count;
 *  - the yaw/pitch pair handed to the motion helper is an s16 array, so the
 *    store to element 1 makes the state stores re-read element 0;
 *  - priority is a local at all three helper calls and the vertex pointer
 *    is a local taken before the radius, which fixes the argument and ring
 *    draw order;
 *  - two float locals carry several short roles each (node x, vertex
 *    radius and diameter; node z, the blend fraction, the minimum x and
 *    the tolerance test), which is what lands the five saved float
 *    registers and the frame;
 *  - the three transform angles are cleared roll first as separate
 *    statements (a chained clear costs a frame cell);
 *  - the TU builds with -Wab,-r4300_mul: the target keeps the radius
 *    multiply out of the call's delay slot.
 *
 * PROVENANCE: no code adapted. Callee names are the tree's adopted names
 * for the resident functions the relocation records select.
 */
extern s8 gO43QueueCountReloc;
extern s32 gO43RenderPendingReloc;
extern Overlay43Record gO43RecordsReloc[];
extern Overlay43State *gO43QueueReloc[];
extern Overlay43Record gO43FallbackReloc;
#define gO43Records gO43RecordsReloc
#define gO43Queue gO43QueueReloc
#define gO43QueueCount gO43QueueCountReloc
#define gO43RenderPending gO43RenderPendingReloc
#define gO43Fallback gO43FallbackReloc

extern Overlay43Level *levelGetLevel(void);
extern s16 Arctanf(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern s16 dAngle(s16 from, s16 to, f32 fraction);
extern void func_overlay_043_F00010A8_188B078(s16 *rotation, s32 owner,
                                              Overlay43Record *record);
extern void func_8002A82C(MtxF matrix);
extern void func_8002AE10(Overlay43Transform *transform, MtxF matrix);
extern void mtxf_mul(MtxF a, MtxF b, MtxF out);
extern void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                                 f32 *outX, f32 *outY, f32 *outZ);
extern void func_overlay_043_F0000BE4_188ABB4(
    Overlay43Input *input, Overlay43Record **records, s32 recordCount);

s32 func_overlay_043_F0000324_188A2F4(Overlay43Input *input, s32 useNodes,
                                      void *unused) {
    Overlay43ModelDefinition *definition;
    Overlay43State *state;
    Overlay43Level *level;
    Overlay43ModelInstance *model;
    Overlay43Record *records[8];
    Overlay43NodeSet *nodeSet;
    /* Node x, then a vertex's scaled radius, then the image diameter. */
    f32 scratchA;
    Overlay43Transform transform;
    MtxF matrix;
    /* Node z, the blend fraction, the minimum x and the tolerance test. */
    f32 scratchB;
    f32 nodeY;
    f32 minimumZ;
    f32 maximumX;
    f32 x;
    f32 y;
    f32 z;
    f32 maximumZ;
    Overlay43Node *node;
    s32 i;
    s32 j;
    f32 extent;
    Overlay43Record *swap;
    s32 priority;
    Overlay43VertexPosition *vertex;
    s32 recordCount;
    s16 yaw;
    s16 pitch;
    s16 rotation[2];
    Overlay43Record *record;

    state = input->link->state;
    model = state->model;
    if (model->active != 0) {
        if (state->displayListEnd == NULL) {
            return 0;
        }
        state->pending = 2;
        gO43RenderPending = 1;
        gO43Queue[gO43QueueCount] = state;
        gO43QueueCount++;
        return 1;
    }
    definition = model->definition;
    level = levelGetLevel();
    recordCount = 0;

    if (useNodes != 0) {
        nodeSet = input->nodeSet;
        if ((nodeSet != NULL) && (nodeSet->count >= 2)) {
            for (i = 1; i < nodeSet->count; i++) {
                gO43Records[recordCount].node = &nodeSet->nodes[i];
                recordCount++;
            }

            for (i = 0; i < recordCount; i++) {
                records[i] = &gO43Records[i];
            }

            for (i = recordCount - 1; i > 0; i--) {
                for (j = 0; j < i; j++) {
                    if (records[j]->node->priority <
                        records[j + 1]->node->priority) {
                        swap = records[j];
                        records[j] = records[j + 1];
                        records[j + 1] = swap;
                    }
                }
            }

            if (state->maximumRecords < recordCount) {
                recordCount = state->maximumRecords;
            }

            if (recordCount != 0) {
                node = records[0]->node;
                scratchA = node->x;
                scratchB = node->z;
                nodeY = node->y;
                yaw = Arctanf(-scratchA, -scratchB);
                pitch = Arctanf(
                    nodeY, sqrtf((scratchA * scratchA) + (scratchB * scratchB)));

                if (node->priority < level->priority) {
                    scratchB = (1.0f / (f32)level->priority) *
                               (f32)node->priority;
                    rotation[0] = dAngle(level->yaw, yaw, scratchB);
                    rotation[1] = dAngle(level->pitch, pitch, scratchB);
                    rotation[0] = dAngle(rotation[0], state->yaw, scratchB);
                    rotation[1] = dAngle(rotation[1], state->pitch, scratchB);
                    state->yaw = rotation[0];
                    state->pitch = rotation[1];
                    priority = level->priority;
                } else {
                    rotation[0] = yaw;
                    rotation[1] = pitch;
                    state->yaw = rotation[0];
                    state->pitch = rotation[1];
                    priority = node->priority;
                }

                func_overlay_043_F00010A8_188B078(rotation, priority, records[0]);
                for (i = 1; i < recordCount; i++) {
                    node = records[i]->node;
                    scratchA = node->x;
                    scratchB = node->z;
                    nodeY = node->y;
                    rotation[0] = Arctanf(-scratchA, -scratchB);
                    rotation[1] = Arctanf(
                        nodeY, sqrtf((scratchA * scratchA) + (scratchB * scratchB)));
                    priority = node->priority;
                    func_overlay_043_F00010A8_188B078(rotation, priority, records[i]);
                }
            }
        } else if (level->priority > 0) {
            rotation[0] = level->yaw;
            rotation[1] = level->pitch;
            state->yaw = rotation[0];
            state->pitch = rotation[1];
            priority = level->priority;
            records[0] = gO43Records;
            recordCount = 1;
            func_overlay_043_F00010A8_188B078(rotation, priority, gO43Records);
        }
    } else if (level->priority > 0) {
        records[0] = gO43Records;
        gO43Fallback.alpha = level->priority;
        recordCount = 1;
        record = records[0];
        func_8002A82C(record->matrix);
        record->matrix[1][1] = 0.0f;
    } else {
        return 0;
    }

    transform.x = -input->x;
    transform.y = -input->y;
    transform.z = -input->z;
    transform.roll = 0;
    transform.pitch = 0;
    transform.yaw = 0;
    func_8002AE10(&transform, state->transform);

    if (state->boundsMode == 0) {
        extent = input->scaleSource->scale * input->scale;
    } else if (state->boundsMode == 1) {
        scratchB = 32000.0f;
        minimumZ = 32000.0f;
        maximumX = -32000.0f;
        maximumZ = -32000.0f;
        for (j = 0; j < recordCount; j++) {
            mtxf_mul(state->transform, records[j]->matrix, matrix);
            for (i = 0; i < definition->vertexCount; i++) {
                vertex = &model->positions[i];
                scratchA = definition->bounds[i].radius * input->scale;
                mtxf_transform_point(matrix, vertex->x, vertex->y,
                                                vertex->z, &x, &y, &z);
                if (maximumX < x + scratchA) {
                    maximumX = x + scratchA;
                }
                if (x - scratchA < scratchB) {
                    scratchB = x - scratchA;
                }
                if (maximumZ < z + scratchA) {
                    maximumZ = z + scratchA;
                }
                if (z - scratchA < minimumZ) {
                    minimumZ = z - scratchA;
                }
            }
        }
        if (scratchB < 0.0f) {
            scratchB = -scratchB;
        }
        if (minimumZ < 0.0f) {
            minimumZ = -minimumZ;
        }
        if (maximumX < scratchB) {
            maximumX = scratchB;
        }
        if (maximumZ < minimumZ) {
            maximumZ = minimumZ;
        }
        if (maximumZ < maximumX) {
            scratchA = maximumX + 10.0f;
        } else {
            scratchA = maximumZ + 10.0f;
        }
        scratchA *= 2.0f;
        scratchB = (100.0f / state->diameter) * scratchA;
        scratchB -= 100.0f;
        if ((scratchB > -5.0f) && (scratchB < 5.0f)) {
            scratchA = state->diameter;
        } else {
            state->diameter = scratchA;
        }
        state->scaleX = 0.05f * scratchA;
        state->scaleY = 0.05f * scratchA;
        extent = 64.0f / scratchA;
    } else if (state->boundsMode == 2) {
        extent = state->fallbackScale;
    }

    func_8002A82C(matrix);
    matrix[0][0] = extent;
    matrix[1][1] = extent;
    matrix[2][2] = extent;
    mtxf_mul(state->transform, matrix, state->transform);
    func_overlay_043_F0000BE4_188ABB4(input, records, recordCount);
    return 1;
}
