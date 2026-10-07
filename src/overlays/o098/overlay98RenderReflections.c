#include "PR/ultratypes.h"

typedef struct Gfx { u32 w0, w1; } Gfx;
typedef struct O98Mtx { u8 bytes[0x40]; } O98Mtx;

typedef struct O98ModelData {
    u8 pad00[0x11];
    u8 special;
    u8 pad12[0x3C];
    s8 mode4E;
    u8 pad4F[0x19];
    void *displayListA;
    void *displayListB;
} O98ModelData;

typedef struct O98Node {
    O98ModelData *data;
    void *vertexData;
    s16 useAlternate;
    s16 partIndex;
    void *partsA[17];
    void *partsB[1];
} O98Node;

typedef struct O98Object {
    s16 rotX, rotY, rotZ;
    s16 flags;
    f32 scale;
    f32 x, y, z;
    u8 pad18[0x21];
    u8 alpha;
    s8 nodeIndex;
    u8 pad3B[5];
    s8 *stateTable;
    u8 pad44[0x24];
    void **nodes;
    u8 pad6C[0x27];
    u8 stateIndex;
} O98Object;

typedef struct O98VisibleEntry { O98Object *object; f32 referenceY; } O98VisibleEntry;
typedef struct O98Context { u8 bytes[0x40]; } O98Context;
typedef struct O98Transform {
    s16 rot0, rot2, rot4;
    u16 pad06;
    f32 scale;
    f32 x, y, z;
} O98Transform;

extern void *o98AcquireRenderContextReloc(void);
extern void o98LoadMatrixReloc(void *, void *);
extern void o98BuildMatrixReloc(O98Transform *, O98Mtx *);
extern void o98CombineMatrixReloc(O98Mtx *, void *, O98Mtx *);
extern void o98BuildInverseMatrixReloc(O98Transform *, O98Mtx *);
extern void o98EmitObjectReloc(Gfx **, u8 **, s32, O98Object *);
extern void o98RestoreStateReloc(Gfx **);

extern s32 gO98Toggle;
extern O98Context gO98Contexts[2];
extern s32 gOverlay98AcceptedCount;
extern O98VisibleEntry gOverlay98AcceptedEntries[0x50];
extern u8 gO98SpecialVertices[];

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
 * vertex packet give the target's callee-saved order. */
#define O98_PACKET(word0, word1) gfx = *dl; *dl = gfx + 1; gfx->w0 = (word0); gfx->w1 = (word1)

#ifdef NON_MATCHING
void overlay98RenderReflections(Gfx **dl, u8 **matrixHeap, s32 arg2) {
    O98Object *object;
    O98Node *node;
    O98Node *node2;
    O98ModelData *model;
    Gfx *gfx;
    void *modelDisplayList;
    void *savedDisplayList;
    O98Mtx matrixC;
    O98Mtx matrixB;
    O98Mtx matrixA;
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
    s8 state;
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
    savedDisplayList = o98AcquireRenderContextReloc();
    o98LoadMatrixReloc(savedDisplayList, &gO98Contexts[gO98Toggle]);

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
            state = object->stateTable[object->stateIndex + 0x1E];
            if (state == 0) {
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
                    modelDisplayList = pick->displayListA;
                } else {
                    modelDisplayList = pick->displayListB;
                }
                if (model->mode4E == 0) {
                    transform.y = referenceY - distance;
                    transform.x = object->x;
                    transform.z = object->z;
                    transform.scale = -object->scale;
                    transform.rot4 = object->rotZ;
                    transform.rot2 = object->rotY;
                    transform.rot0 = object->rotX + 0x8000;
                    o98BuildMatrixReloc(&transform, &matrixB);
                    o98CombineMatrixReloc(&matrixB, savedDisplayList, &matrixA);
                    o98LoadMatrixReloc(&matrixA, *matrixHeap);
                    O98_PACKET(0x01010040, (u32)*matrixHeap + 0x80000000);
                    *matrixHeap += 0x40;
                    drewObject = 1;
                } else if (node->useAlternate == 0) {
                    inverse.x = -object->x;
                    inverse.y = -object->y;
                    inverse.z = -object->z;
                    inverse.scale = 1.0f;
                    inverse.rot4 = -object->rotZ;
                    inverse.rot2 = -object->rotY;
                    inverse.rot0 = -object->rotX;
                    emittedReflection = 1;
                    o98BuildInverseMatrixReloc(&inverse, &matrixC);
                    transform.x = object->x;
                    transform.y = referenceY - distance;
                    transform.z = object->z;
                    transform.scale = -1.0f;
                    transform.rot4 = object->rotZ;
                    transform.rot2 = object->rotY;
                    transform.rot0 = object->rotX + 0x8000;
                    o98BuildMatrixReloc(&transform, &matrixB);
                    o98CombineMatrixReloc(&matrixC, &matrixB, &matrixA);
                    o98CombineMatrixReloc(&matrixA, savedDisplayList, &matrixA);
                    o98LoadMatrixReloc(&matrixA, *matrixHeap);
                    O98_PACKET(0x01000040, (u32)*matrixHeap + 0x80000000);
                    *matrixHeap += 0x40;
                    drewObject = 1;
                }
                if (drewObject) {
                    O98_PACKET(0xFA000000, object->alpha | ~0xFF);
                    O98_PACKET((((u32)node->partsA[node->partIndex] + 0x80000000) & 0xFFFFFF) | 0xBF000000, (u32)node2->vertexData + 0x80000000);
                    /* Two zero-cost blocks: they carry specialModel and
                     * modelDisplayList across the next save divisor (5.0 to
                     * 4.29), so &matrixA takes s7, specialModel s8 and the
                     * display-list pointer spills to its home as shipped.
                     * A stand-in for whatever block structure the original
                     * has here; see the shard. */
                    do { } while (0);
                    do { } while (0);
                    if (specialModel) {
                        if (stateIndex) {
                            O98_PACKET(0x02000050, (u32)gO98SpecialVertices + 0x80000000);
                        } else {
                            O98_PACKET(0x02000050, (u32)node->partsB[node->partIndex] + 0x80000000);
                        }
                    }
                    O98_PACKET(0x06000000, (u32)modelDisplayList + 0x80000000);
                    O98_PACKET(0xBF000000, 0);
                    O98_PACKET(0xBC00000A, 0);
                    o98RestoreStateReloc(dl);
                    O98_PACKET(0xFA000000, 0xFFFFFFFF);
                }
            } else if (!(object->flags & 0x400) && state == 2) {
            } else if (!(object->flags & 0x400) && state == 1) {
                if (emittedReflection) {
                    O98_PACKET(0x01000040, (u32)&gO98Contexts[gO98Toggle] + 0x80000000);
                    emittedReflection = 0;
                }
                oldY = object->y;
                object->y = referenceY - distance;
                object->scale = -object->scale;
                o98EmitObjectReloc(dl, matrixHeap, arg2, object);
                object->y = oldY;
            }
        } while (i < gOverlay98AcceptedCount);
    }
    O98_PACKET(0xB6000000, 0x1000);
    if (emittedReflection) {
        O98_PACKET(0x01000040, (u32)&gO98Contexts[gO98Toggle] + 0x80000000);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o098/overlay98RenderReflections/func_overlay_098_F0000234_18D8BF4.s")
#endif

/* PLATEAU-HANDOFF:overlay98RenderReflections:start
 * symbol: overlay98RenderReflections
 * score: 225/389 words
 * frame: 0x1C8
 * relocations: 36
 * first-mismatch: +0xB0
 * summary: Packets on one line, second node name and pick local, target callee-saved order: 267 to 225 at delta 0 (aligned 66); open: cursor's second home store at +0xFC.
 * PLATEAU-HANDOFF:overlay98RenderReflections:end
 */
