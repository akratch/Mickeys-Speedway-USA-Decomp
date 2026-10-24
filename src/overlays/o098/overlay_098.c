#include "overlays/overlay_098.h"

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
/* Matched by indexing the unique list and bounding the scan by the count
 * global itself: the compiler reduces the subscript to a walking pointer and
 * an end pointer of its own, each with its own address load, and reads the
 * count once. No cursor, end pointer, count copy or next count is declared.
 * The point-reference index, the vertex base and the vertex pointer are named
 * locals; those three webs are what push the hoisted addresses and the stride
 * constant into saved registers.
 */
void overlay98CollectUniqueY(Overlay98Group *group) {
    Overlay98Block *block;
    Overlay98Span *span;
    s16 value;
    s32 blockIndex;
    s32 spanIndex;
    s32 k;
    s32 pointRefIndex;
    s32 vertexBase;
    s32 isNew;
    Overlay98Vertex *vertex;

    overlay98UniqueCountReloc = 0;
    for (blockIndex = 0; blockIndex < group->blockCount; blockIndex++) {
        block = &group->blocks[blockIndex];
        for (spanIndex = 0; spanIndex < block->spanCount; spanIndex++) {
            span = &block->spans[spanIndex];
            if (span->flags & 0x8000) {
                pointRefIndex = span->pointRefIndex;
                vertexBase = span->vertexBase;
                vertex = &block->vertices[
                    block->pointRefs[pointRefIndex].vertexIndex + vertexBase];
                value = vertex->y;
                isNew = 1;
                for (k = 0; k < overlay98UniqueCountReloc; k++) {
                    if (overlay98UniqueYReloc[k] == value) {
                        isNew = 0;
                    }
                }
                if (isNew) {
                    overlay98UniqueYReloc[overlay98UniqueCountReloc++] = value;
                    if (overlay98UniqueCountReloc >= 15) {
                        spanIndex = block->spanCount;
                        blockIndex = group->blockCount;
                    }
                }
            }
        }
    }
}
s32 overlay98CheckObject(O98Object *object, u8 *context, f32 *result);

/* PROVENANCE: reproduced from the retired public mirror, commit
 * f56d08c746f891f76c4b7bab8e3a2a4332894634, which no longer resolves. All
 * retained measurements and the ABI correction above were independently
 * derived from Mickey's own source, object, relocation tables, retail bytes. */
/* Matched 2026-08-31 by tracing IDO's automatic stack-home producers. Removing
 * two transient entry aliases and making the address-taken result the third
 * surviving automatic reproduces the 60-word body, 0x50 frame, and all six
 * relocation sites; the complete linked US ROM is byte-identical. */
void overlay98CollectAccepted(s32 count, O98Object **objects) {
    u8 *context;
    s32 index;
    f32 value;

    context = camGetPtr();
    gOverlay98AcceptedCount = 0;
    index = count - 1;
    if (index >= 0) {
        do {
            O98Object *object = objects[index];

            index--;
            if (overlay98CheckObject(object, context, &value) != 0) {
                object->accepted = 1;
                gOverlay98AcceptedEntries[gOverlay98AcceptedCount].object =
                    object;
                gOverlay98AcceptedEntries[gOverlay98AcceptedCount++].referenceY =
                    value;
                if (gOverlay98AcceptedCount >= 0x50) {
                    index = -1;
                }
            } else {
                object->accepted = 0;
            }
        } while (index >= 0);
    }
}

/* Rewritten from the listing (lane a-ovl4, 2026-10-07): transform and
 * inverse fields written to the offsets the target stores, the entry list
 * indexed so uopt creates the cursor temporary, one callee for the context
 * set-up and the matrix load (relocation records), two-argument matrix
 * builds, literal segment bases, and the target's home ladder: five
 * register locals, the display-list pointers, the matrices, the two float
 * homes, and fourteen unused cells after the state index.
 * Lane e-ovl3 (2026-10-07), 267 to 227 at delta 0: every packet is one
 * physical line (the macro), so as1 stores a constant w1 before w0 as
 * shipped and the packet pointer is one web in s0; a second node variable
 * read from the same subscript after the state-index store carries the
 * vertex word (the target's s5); the matrix word is (u32)*matrixHeap plus
 * the literal so it shares the hoisted 0x80000000 web; the emitted flag is
 * set just before the inverse build. Then 227 to 226 (aligned residual
 * 100 to 71): the display-list pick reads node2->data into its own local
 * before the alpha test (the target's second load of the node's first
 * word), and the reflected arm fills its transform x, y, z, scale. modelDisplayList is
 * a plain local again (226 to 225, aligned 66) once two blocks after the
 * vertex packet give the target's callee-saved order.
 * Lane h-5 (2026-10-07), 225 to 5 at delta 0, read from the aligned listing
 * and the relocation records: the special-vertex word is the bare address
 * (a SYMBOL record whose stored word already carries 0x80000000, built by a
 * lui/addiu pair with no add of the constant web); the alpha word masks the
 * u8 load (the extra ring draw as1 folds, L149); the first matrix arm fills
 * x before y; modelDisplayList is a u32, so its packet's 0x80000000 is the
 * same constant web as every other packet's (as a pointer cast it was a
 * second constant, materialised inline, and its extra interferer cost the
 * cursor's split piece the negation block and its second home store); the
 * emitted flag shares the inverse call's line (its store fills the delay
 * slot). Then 5 to 0: no `state` local; the three state tests read
 * object->stateTable[object->stateIndex + 0x1E] at each use (the local
 * held one more frame cell than the target declares, which put the
 * cursor temporary at +0x58 instead of +0x5C). Matched. */
#define O98_PACKET(word0, word1) gfx = *dl; *dl = gfx + 1; gfx->words.w0 = (word0); gfx->words.w1 = (word1)

void overlay98RenderReflections(Gfx **dl, Mtx **matrixHeap,
                                struct TrackVertex **vertices) {
    O98Object *object;
    O98Node *node;
    O98Node *node2;
    O98ModelData *model;
    Gfx *gfx;
    u32 modelDisplayList;
    MtxF *projection;
    MtxF matrixC;
    MtxF matrixB;
    MtxF matrixA;
    f32 referenceY;
    f32 distance;
    f32 oldY;
    O98Transform inverse;
    O98Transform transform;
    s32 emittedReflection;
    s32 i;
    s32 specialModel;
    s32 stateIndex;
    s32 drewObject;
    s32 pad0;
    s32 pad1;
    s32 pad2;
    s32 pad3;
    s32 pad4;
    s32 pad5;
    s32 pad6;
    s32 pad7;
    s32 pad8;
    s32 pad9;
    s32 pad10;
    s32 pad11;
    s32 pad12;
    O98ModelData *pick;

    gO98Toggle ^= 1;
    projection = camGetProjectionMtx();
    mtxf_to_mtx(*projection, &gO98Contexts[gO98Toggle]);

    O98_PACKET(0xB7000000, 0x1000);
    emittedReflection = 0;
    i = 0;
    if (gOverlay98AcceptedCount > 0) {
        do {
            object = gOverlay98AcceptedEntries[i].object;
            referenceY = gOverlay98AcceptedEntries[i].referenceY;
            i++;
            distance = object->y - referenceY;
            if (distance < 0.0f) {
                distance = -distance;
            }
            if ((object->flags & 0x400) || (object->flags & 0x40)) {
                continue;
            }
            if (object->stateTable[object->stateIndex + 0x1E] == 0) {
                node = (O98Node *)object->nodes[object->nodeIndex];
                model = node->data;
                specialModel = 0;
                drewObject = 0;
                if (model->special != 0) {
                    specialModel = 1;
                }
                stateIndex = object->stateIndex;
                node2 = (O98Node *)object->nodes[object->nodeIndex];
                pick = node2->data;
                if (object->alpha == 0xFF) {
                    modelDisplayList = (u32)pick->displayListA;
                } else {
                    modelDisplayList = (u32)pick->displayListB;
                }
                if (model->mode4E == 0) {
                    transform.x = object->x;
                    transform.y = referenceY - distance;
                    transform.z = object->z;
                    transform.scale = -object->scale;
                    transform.rot4 = object->rotZ;
                    transform.rot2 = object->rotY;
                    transform.rot0 = object->rotX + 0x8000;
                    func_8002AA50(&transform, matrixB);
                    mtxf_mul(matrixB, *projection, matrixA);
                    mtxf_to_mtx(matrixA, *matrixHeap);
                    O98_PACKET(0x01010040, (u32)*matrixHeap + 0x80000000);
                    (*matrixHeap)++;
                    drewObject = 1;
                } else if (node->useAlternate == 0) {
                    inverse.x = -object->x;
                    inverse.y = -object->y;
                    inverse.z = -object->z;
                    inverse.scale = 1.0f;
                    inverse.rot4 = -object->rotZ;
                    inverse.rot2 = -object->rotY;
                    inverse.rot0 = -object->rotX;
                    emittedReflection = 1; func_8002AC84(&inverse, matrixC);
                    transform.x = object->x;
                    transform.y = referenceY - distance;
                    transform.z = object->z;
                    transform.scale = -1.0f;
                    transform.rot4 = object->rotZ;
                    transform.rot2 = object->rotY;
                    transform.rot0 = object->rotX + 0x8000;
                    func_8002AA50(&transform, matrixB);
                    mtxf_mul(matrixC, matrixB, matrixA);
                    mtxf_mul(matrixA, *projection, matrixA);
                    mtxf_to_mtx(matrixA, *matrixHeap);
                    O98_PACKET(0x01000040, (u32)*matrixHeap + 0x80000000);
                    (*matrixHeap)++;
                    drewObject = 1;
                }
                if (drewObject) {
                    O98_PACKET(0xFA000000, (object->alpha & 0xFF) | ~0xFF);
                    O98_PACKET((((u32)node->partsA[node->partIndex] + 0x80000000) & 0xFFFFFF) | 0xBF000000, (u32)node2->vertexData + 0x80000000);
                    /* Two zero-cost blocks: they carry specialModel and
                     * modelDisplayList across the next save divisor (5.0 to
                     * 4.29), so matrixA takes s7, specialModel s8 and the
                     * display-list pointer spills to its home as shipped.
                     * A stand-in for whatever block structure the original
                     * has here; see the shard. */
                    do { } while (0);
                    do { } while (0);
                    if (specialModel) {
                        if (stateIndex) {
                            O98_PACKET(0x02000050, (u32)gO98SpecialVertices);
                        } else {
                            O98_PACKET(0x02000050, (u32)node->partsB[node->partIndex] + 0x80000000);
                        }
                    }
                    O98_PACKET(0x06000000, modelDisplayList + 0x80000000);
                    O98_PACKET(0xBF000000, 0);
                    O98_PACKET(0xBC00000A, 0);
                    texDPInit(dl);
                    O98_PACKET(0xFA000000, 0xFFFFFFFF);
                }
            } else if (!(object->flags & 0x400) && object->stateTable[object->stateIndex + 0x1E] == 2) {
            } else if (!(object->flags & 0x400) && object->stateTable[object->stateIndex + 0x1E] == 1) {
                if (emittedReflection) {
                    O98_PACKET(0x01000040, (u32)&gO98Contexts[gO98Toggle] + 0x80000000);
                    emittedReflection = 0;
                }
                oldY = object->y;
                object->y = referenceY - distance;
                object->scale = -object->scale;
                func_80009E78(dl, matrixHeap, vertices, object);
                object->y = oldY;
            }
        } while (i < gOverlay98AcceptedCount);
    }
    O98_PACKET(0xB6000000, 0x1000);
    if (emittedReflection) {
        O98_PACKET(0x01000040, (u32)&gO98Contexts[gO98Toggle] + 0x80000000);
    }
}

/* PROVENANCE: reproduced from the retired public mirror, commit
 * f56d08c746f891f76c4b7bab8e3a2a4332894634, which no longer resolves. All
 * retained measurements were independently derived from Mickey's own source,
 * object, relocation tables, and retail bytes. */
/* Mickey byte identity: named data and float-difference carriers reproduce
 * allocation; initialize accepted after the height branch and preserve the
 * target homes with the unused scratch area preceding the vectors. */
s32 overlay98CheckObject(O98Object *object, u8 *context, f32 *result) {
    O98NodeData *data;
    f32 difference;
    s32 scratch[8];
    Vec3f output;
    Vec3f input;
    f32 adjustment;
    s32 index;
    s32 accepted;
    (void)&scratch;

    if (object->stateTable[object->stateIndex + 0x1E] == 0) {
        O98HeightNode *node = (O98HeightNode *)object->nodes[object->nodeIndex];
        data = node->data;
        adjustment = (f32)data->height * object->scale;
    } else {
        adjustment = 0.0f;
    }

    accepted = 0;
    if ((func_8001357C(object->x, object->z, result, 0x8000, 0) & 0x8000) &&
        ((*result - 5.0f) < object->y)) {
        accepted = 1;
    } else {
        input.x = object->x;
        input.z = object->z;
        index = 0;
        if (overlay98UniqueCountReloc > 0) {
            do {
                f32 height = (f32)overlay98UniqueYReloc[index];

                difference = object->y - height;
                input.y = (height - difference) - adjustment;
                if (func_8001291C(context + 0xC, &input, &output, 0, 0x8000) != 0) {
                    *result = output.z;
                    accepted = 1;
                    index = overlay98UniqueCountReloc;
                }
                index++;
            } while (index < overlay98UniqueCountReloc);
        }
    }
    return accepted;
}
