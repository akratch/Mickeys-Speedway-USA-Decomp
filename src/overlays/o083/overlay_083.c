#include "overlays/overlay_083.h"

/*
 * DKR v77/v80 and JFG have generic transformed-vertex builders, but their
 * object scans contain no exact donor for overlay 83.
 */
static void overlay83BuildLine(Overlay83Vertex **vertices,
                               Overlay83LineTransform *transform, s32 alpha) {
    Overlay83Vector points[2];

    points[0].x = -transform->halfLength;
    points[0].y = 0.0f;
    points[0].z = 0.0f;
    points[1].x = transform->halfLength;
    points[1].y = 0.0f;
    points[1].z = 0.0f;
    pointListRPY(2, transform, points, points);

    points[0].x += transform->x;
    points[0].y += transform->y;
    points[0].z += transform->z;
    points[1].x += transform->x;
    points[1].y += transform->y;
    points[1].z += transform->z;

    (*vertices)->x = points[0].x;
    (*vertices)->y = points[0].y;
    (*vertices)->z = points[0].z;
    (*vertices)->red = 0xFF;
    (*vertices)->green = 0xFF;
    (*vertices)->blue = 0xFF;
    (*vertices)->alpha = alpha;
    (*vertices)++;

    (*vertices)->x = points[1].x;
    (*vertices)->y = points[1].y;
    (*vertices)->z = points[1].z;
    (*vertices)->red = 0xFF;
    (*vertices)->green = 0xFF;
    (*vertices)->blue = 0xFF;
    (*vertices)->alpha = alpha;
    (*vertices)++;
}

/*
 * DKR v77/v80 and JFG contain generic transformed-line and debug-geometry
 * code, but their object scans contain no exact donor for this history-line
 * renderer.
 */
static void overlay83DrawLines(Overlay83Entry *object) {
    if (object->recordCount != 0) {
        s32 recordIndex;
        f32 savedHalfLength;
        Overlay83Vertex *vertices;

        object->bufferIndex ^= 1;
        vertices = (Overlay83Vertex *)
            ((u8 *)object + object->bufferIndex * 0xB4 + 0xF0);
        savedHalfLength = object->halfLength;
        object->halfLength = object->drawHalfLength;
        overlay83BuildLine(&vertices, (Overlay83LineTransform *)object,
                           object->alpha);
        object->halfLength = savedHalfLength;

        recordIndex = object->lastRecord;
        do {
            recordIndex--;
            if (recordIndex < 0) {
                recordIndex = 7;
            }
            overlay83BuildLine(&vertices,
                               (Overlay83LineTransform *)&object->records[recordIndex],
                               object->records[recordIndex].alpha);
        } while (recordIndex != object->firstRecord);
    }
}

static void overlay83Update(Overlay83Parent *parent,
                            Overlay83Entry *object,
                            s32 updateRate) {
    s32 activeCount;
    s32 recordIndex;
    Overlay83Record *record;
    Overlay83Batch *batch;
    UnkLight *light;

    activeCount = object->recordCount;
    batch = parent->batch;
    if (activeCount != 0) {
        recordIndex = object->firstRecord;
        do {
            record = &object->records[recordIndex];
            recordIndex++;
            if (recordIndex >= 8) {
                recordIndex = 0;
            }
            record->life--;
            if (record->life < 0) {
                object->firstRecord = recordIndex;
                object->recordCount--;
            } else {
                record->alpha = (record->life * 0x24 * object->alpha) >> 8;
            }
        } while (recordIndex != object->lastRecord);
        activeCount = object->recordCount;
    }

    object->alpha = batch->alpha;
    if (activeCount < 8 && object->alpha != 0) {
        record = &object->records[object->lastRecord];
        object->recordCount = activeCount + 1;
        object->lastRecord++;
        if (object->lastRecord >= 8) {
            object->lastRecord = 0;
        }
        record->x = object->x;
        record->y = object->y;
        record->z = object->z;
        record->life = 7;
        record->alpha = (record->life * 0x24 * object->alpha) >> 8;
        record->halfLength = object->drawHalfLength;
        record->worldX = object->worldX;
        record->worldY = object->worldY;
        record->worldZ = object->worldZ;
    }

    object->x += object->velocityX * updateRate;
    object->y += object->velocityY * updateRate;
    object->z += object->velocityZ * updateRate;
    object->worldY = object->height;
    object->worldX = 0.0f;
    object->worldZ = 0.0f;
    pointListRPY(1, object, &object->worldX, &object->worldX);
    light = object->light;
    object->worldX += parent->worldX;
    object->worldY += parent->worldY;
    object->worldZ += parent->worldZ;

    if (light != 0) {
        light->x = object->worldX;
        light->y = object->worldY;
        light->z = object->worldZ;
        changeLightIntensity(light, object->alpha);
    }
}

/* DKR v77/v80 and JFG checks found no exact donor for this two-stage wrapper. */
static void overlay83Submit(Overlay83Parent *parent, Overlay83Entry *entry,
                            s32 updateRate) {
    overlay83Update(parent, entry, updateRate);
    overlay83DrawLines(entry);
}

/* Mickey-local reconstruction; pinned DKR/JFG scans found no exact donor. */
/* Tier A: stock IDO body and runtime relocations proved against overlay 83. */
void overlay83BuildBatch(Overlay83Parent *parent, Overlay83Source *source) {
    Overlay83Batch *batch;
    Overlay83Entry *output;
    ObjectLightEntry linkedInit;
    Overlay83SourceRecord *input;
    s32 remaining;
    f32 scaleFactor;

    batch = parent->batch;
    batch->entries = mmAlloc(source->count * 0x258, 0x87);
    if (batch->entries != 0) {
        batch->count = source->count;
        output = batch->entries;
        input = source->records;
        batch->alpha = source->alpha;
        remaining = source->count;
        if (remaining--) {
            scaleFactor = gOverlay83ScaleReloc;
            do {
                output->x = input->x * 256;
                output->y = input->y * 256;
                output->z = input->z * 256;
                output->height = input->alpha * 4;
                output->halfLength = input->scale * scaleFactor;
                output->worldY = output->height;
                output->worldX = 0.0f;
                output->worldZ = 0.0f;
                pointListRPY(1, output, &output->worldX, &output->worldX);
                output->worldX += parent->worldX;
                output->worldY += parent->worldY;
                output->worldZ += parent->worldZ;
                output->drawHalfLength = input->halfLength;
                output->velocityX = input->velocityX * 0x10;
                output->velocityY = input->velocityY * 0x10;
                output->velocityZ = input->velocityZ * 0x10;
                output->red = input->red;
                output->green = input->green;
                output->blue = input->blue;
                output->alpha = source->alpha; output->zero24 = 0.0f;
                if (input->flags & 1) {

                    linkedInit.mode = 1;
                    linkedInit.type = 3;
                    linkedInit.flags = input->flags;
                    linkedInit.index = -1;
                    linkedInit.home = 0;
                    linkedInit.x = 0;
                    linkedInit.y = 0;
                    linkedInit.z = 0;
                    linkedInit.radius = 0xC0;
                    linkedInit.radius2 = 0xC0;
                    linkedInit.red = output->red;
                    linkedInit.green = output->green;
                    linkedInit.blue = output->blue;
                    linkedInit.intensity = output->alpha;
                    linkedInit.colourCycleIndex = -1;
                    linkedInit.value58 = 0;
                    linkedInit.value5A = 0;
                    output->light = addObjectLight(0, &linkedInit);
                } else {
                    output->light = 0;
                }
                output->firstRecord = 0;
                output->lastRecord = 0;
                output->recordCount = 0;
                output->bufferIndex = 1;
                input++;
                output++;
            } while (remaining--);
        }
    } else {
        batch->count = 0;
    }
    parent->clear3C = 0;
}

/* DKR v77/v80 and JFG contain no exact donor or semantic source lead for this fixed-stride draw loop. */
void overlay83DrawEntries(Overlay83Parent *object, s32 updateRate) {
    Overlay83Batch *descriptor;
    u8 *entry;
    u8 *current;
    s32 remaining;

    descriptor = object->batch;
    remaining = descriptor->count;
    entry = (u8 *)descriptor->entries;
    while (remaining--) {
        current = entry;
        entry += 0x258;
        overlay83Submit(object, (Overlay83Entry *)current, updateRate);
    }
}

/*
 * PROVENANCE: the packet macros follow Jet Force Gemini's published
 * decompilation (include/f3ddkr.h gSPVertexJFG and gSPPolygon), adapted to
 * this file's command type. _SHIFTL, gDma1p, gDPSetPrimColor and
 * gDPSetEnvColor come from the SDK's gbi.h, whose shapes JFG's copies follow.
 */
#define gSPVertexJFG(pkt, v, n, v0) \
    gDma1p(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define gSPPolygon(dl, ptr, numTris, texEnabled) { \
    Overlay83Command *_g = (Overlay83Command *)(dl); \
    _g->w0 = _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(5, 24, 8) | \
             _SHIFTL(((numTris)*16), 0, 16); \
    _g->w1 = (unsigned int)(ptr); \
}

/*
 * Matched 2026-10-07 (lane a-ovl1). One packet macro per command, written
 * with the SDK/JFG shapes above. Two edits closed the old 58-word residual:
 * the colour words packed through _SHIFTL, and the vertex address written
 * as byte arithmetic on the strip base (index times record size first), which
 * gives the target's base-first add at both evaluations. The saved
 * display-list pointer is taken after the first packet; taken before it, the
 * parameter keeps a2 instead of the target's a3.
 */
static void overlay83DrawStrip(Overlay83Command **displayList, Overlay83Entry *strip) {
    Overlay83Command **savedDisplayList;
    s32 count;
    s32 doubledCount;
    s32 vertexCount;

    count = strip->recordCount;
    if (count != 0) {
        doubledCount = count * 2;
        vertexCount = doubledCount + 2;
        gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);
        savedDisplayList = displayList;
        gDPSetEnvColor((*displayList)++, strip->red, strip->green, strip->blue, 255);
        gSPVertexJFG((*displayList)++,
                     (u8 *)strip + strip->bufferIndex * sizeof(Overlay83VertexBuffer) + 0x800000F0,
                     vertexCount, 0);
        gSPPolygon((*savedDisplayList)++, D_80000000, doubledCount, 1);
    }
}

/* DKR v77/v80 and JFG have no exact donor for this submission wrapper. */
static void overlay83Dispatch(Overlay83Command **displayList, void *mtx,
                              void *vertices, Overlay83Entry *config,
                              void *resource) {
    if (config->alpha != 0) {
        sprSetIA2ColOverride(255, 255, 255, config->red,
                                config->green, config->blue);
        camDoSprite(displayList, mtx, vertices, config, resource, 14, config->alpha);
        sprClearIA2ColOverride();
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this two-pass renderer. */
void overlay83SubmitAll(Overlay83Command **displayList, void *mtx, void *vertices,
                        Overlay83Parent *context) {
    s32 count;
    u8 *record;
    Overlay83Batch *batch;

    batch = context->batch;
    overlay83SubmitSetupReloc(displayList, mtx, (s32)vertices, (s32)context);
    overlay83SubmitSetupReloc(displayList, *context->setup, 0x213, 0);

    count = batch->count;
    record = (u8 *)batch->entries;
    while (count--) {
        u8 *currentRecord = record;
        record += 0x258;
        overlay83DrawStrip(displayList, (Overlay83Entry *)currentRecord);
    }

    count = batch->count;
    record = (u8 *)batch->entries;
    while (count--) {
        u8 *currentRecord = record;
        record += 0x258;
        overlay83Dispatch(displayList, mtx, vertices,
                          (Overlay83Entry *)currentRecord,
                          *context->nested->resource);
    }

    gDPPipeSync((*displayList)++);
    gDPSetPrimColor((*displayList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor((*displayList)++, 0xFF, 0xFF, 0xFF, 0xFF);
}
