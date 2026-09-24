/*
 * Model animation loading and matrix generation -- ROM 0x5B300-0x5C310
 * (VRAM 0x8005A700-0x8005B710).
 *
 * The TU name is Tier B/D: its callers and data flow establish animation-table
 * loading, reference-counted animation allocation, frame selection and model
 * matrix construction. JFG models.c supplies the nearest non-exact skeletons
 * for the loader/free pair. camConvertMatrixList alone is Tier A against JFG
 * camera.c; that isolated helper does not turn the full range into camera.c.
 *
 * PROVENANCE: JFG's permitted src/models.c, models.h and camera.c were read
 * for names, layouts and comparison. The initial split adapted no body;
 * point-of-use notes identify the later JFG adaptations. Canonical
 * func_8005A948 flags include `-Wo,-loopunroll,0`; its earlier ignored local
 * isolated evidence omitted that override, and current HEAD is uncompiled.
 */

#include "PR/ultratypes.h"
#include "game/pi.h"

typedef f32 Matrix[4][4];

typedef struct ConvListEntry {
    Matrix *mtx;
    s16 count;
} ConvListEntry;

typedef struct AnimationCacheEntry {
    s32 id;
    u8 *animation;
} AnimationCacheEntry;

typedef struct LoadedAnimation {
    u8 references;
    u8 pad1[3];
    s16 id;
} LoadedAnimation;

typedef struct ModelMatrixNode {
    s16 parent;
    u8 pad2[2];
    f32 x;
    f32 y;
    f32 z;
} ModelMatrixNode;

typedef struct ModelAnimationTable {
    u8 pad0[0x4E];
    s8 animationCount;
    u8 pad4F;
    u8 **animations;
} ModelAnimationTable;

typedef struct ModelAnimationFrame {
    u8 pad0;
    u8 flags;
    s16 offset;
    u8 pad4[2];
    u8 loop;
    u8 pad7;
    u8 count;
} ModelAnimationFrame;

typedef struct ModelAnimationInfo {
    u8 pad0[0x4E];
    s8 frameCount;
    u8 pad4F;
    ModelAnimationFrame **frames;
} ModelAnimationInfo;

typedef struct ModelAnimationState {
    ModelAnimationInfo *info;
    u8 pad4[0x18];
    ModelAnimationFrame *frame;
    void *frameData;
    u8 pad24[4];
    f32 frameValue;
    f32 pad2C;
    f32 blendStart;
    f32 blendEnd;
    f32 blendValue;
    s16 frameIndex;
    s8 transition;
    u8 hasNext;
} ModelAnimationState;

typedef struct ModelAnimationInstance {
    u8 pad0[0x28];
    f32 frameValue;
    u8 pad2C[0xE];
    s8 animationIndex;
    s8 frame;
    u8 pad3C[0x2C];
    ModelAnimationState **states;
} ModelAnimationInstance;

typedef struct ModelRenderSlot {
    u8 pad0[0xC];
    Matrix *matrices;
    u8 pad10[4];
    s32 count;
} ModelRenderSlot;

typedef struct ModelRenderInstance {
    u8 pad0[4];
    s32 count;
    u8 pad8[2];
    s16 activeSlot;
    u8 padC[0x10];
    s32 animated;
    u8 pad20[8];
    f32 scale;
    f32 offset;
    u8 pad30[0xF];
    u8 mode;
    f32 *vertices[3];
} ModelRenderInstance;

typedef struct ModelRenderPointA {
    u16 vertex;
    u16 node;
} ModelRenderPointA;

typedef struct ModelRenderPointB {
    u16 vertex;
    s8 node;
    u8 pad3[9];
} ModelRenderPointB;

typedef struct ModelRenderContext {
    u8 pad0[0x1C];
    u8 *vertexData;
    u8 pad20[0xD];
    u8 count0;
    u8 count1;
    u8 count2;
    ModelRenderPointA *points0;
    ModelRenderPointB *points1;
    ModelRenderPointB *points2;
    u8 pad3C[0x13];
    s8 matrixCount;
    u8 pad50[4];
    ModelMatrixNode *nodes;
} ModelRenderContext;

typedef struct ModelRenderAsset {
    s8 cameraIndex;
    u8 pad1[0x4F];
    f32 scale;
    u8 pad54[0x3E8];
    s16 angle;
} ModelRenderAsset;

typedef struct ModelRenderModel {
    u8 pad0[6];
    s16 flags;
    f32 transformScale;
    u8 padC[0x1C];
    f32 scale;
    u8 pad2C[0x18];
    s16 type;
    u8 pad46[0x1E];
    ModelRenderAsset *asset;
} ModelRenderModel;

typedef struct ModelRenderCamera {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x3C];
} ModelRenderCamera;

typedef struct ModelRenderNodeData {
    u8 pad0[0x94];
    f32 x;
    f32 y;
    f32 z;
} ModelRenderNodeData;

typedef struct ModelRenderMatrixNode {
    u8 pad0[0x30];
    f32 x;
    f32 y;
    f32 z;
} ModelRenderMatrixNode;

typedef struct ModelRenderVertex {
    s16 x;
    s16 y;
    s16 z;
} ModelRenderVertex;

typedef struct ModelRenderTransform {
    s16 rotation0;
    s16 rotation1;
    s16 rotation2;
    u8 pad6[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} ModelRenderTransform;

extern s32 D_800D7CF0;
extern s32 D_800D7CF4;
extern s32 D_800D7CF8;
extern s32 D_800D7CFC;
extern s32 D_800D7D00;
extern s32 D_800D7D04;
extern ConvListEntry D_800D78F0[];

s32 func_8002B280(s32 size, s32 tag);
void *func_8002B314(s32 size, u32 colourTag);
void func_80058FF0(ConvListEntry *entries, s32 count);
void func_8002A82C(void *mtx);
void mtxf_mul(void *lhs, void *rhs, void *dest);
void func_8002AA50(void *transform, void *matrix);
void func_80029AB8(void *matrix, f32 scale);
void mtxf_transform_point(Matrix matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
ModelRenderCamera *camGetListPtr(void);
s32 camGetMode(void);
s32 func_800290A0(void);
s32 Arctanf(f32 y, f32 x);
f32 func_8002A8BC(s16 angle);
f32 func_8002A8C0(s16 angle);
void func_8002B040(void *matrix, s32 x, s32 y, s32 z,
                   f32 *outX, f32 *outY, f32 *outZ);
void func_800591B0(Matrix *matrices, Matrix root,
                    ModelRenderInstance *instance, ModelMatrixNode *nodes,
                    void *asset);
void func_8005B644(Matrix *matrices, Matrix *root, ModelMatrixNode *node,
                   s32 count);
void mmFree(void *ptr);
u8 *func_8005A948(s16 animationId);
void func_8005AAC0(u8 *animation);

/* PROVENANCE: adapted from the modelsInit tail in JFG src/models.c. */
void func_8005A700(void) {
    s32 allocation;

    allocation = func_8002B280(0xA0, 0x80);
    D_800D7CFC = allocation;
    D_800D7D00 = allocation + 0x80;
    D_800D7CF8 = allocation + 0x90;
    D_800D7CF4 = func_8002B280(0x800, 0x80);
    D_800D7D04 = 0;
    D_800D7CF0 = 0;
}
void func_8005A764(void) {
    D_800D7CF0 = 0;
}
void func_8005A770(void) {
    func_80058FF0(D_800D78F0, D_800D7CF0);
    D_800D7CF0 = 0;
}
/*
 * PROVENANCE: Mickey-derived. JFG src/models.c::modLoadModel remains assembly
 * and supplies role/TU context only; no donor body was imported.
 *
 * Matched 2026-09-16 (lane w1-a), 10 -> 0 masked words at delta 0 in four
 * measured batches, no colour force. The ten words were the frame (0x50
 * against 0x38), the colour of `firstAnimation & 3` (ours v1, the ROM s0)
 * and the spill placed around the wrong call. Read off the ROM: the value
 * lives in s0 through the second piRomLoadSection call, is stored to the
 * frame's one home right before func_8002B314, and only the doubling in the
 * loop setup reads it back, into a temporary. That is two webs, not one:
 * `alignment` stops at that store, so it no longer shares the loop-setup
 * block with the loop's byte offset and takes s0 at cost 0; a second symbol
 * carries the value across the allocation call. Which symbol decides the
 * reload register and the frame. `loadSize` is coloured a3 whole and reloads
 * into a3 (5 words); `inputOffset` takes s1 and no spill at all (24);
 * `lastAnimation` a2 (6); a fresh local +4. `firstAnimation` is the one whose
 * web already spans the second call's argument block, so a0-a3 are denied
 * and it takes c7 = t0, the ROM's reload register, at cost 4 -- but written
 * as a plain copy `firstAnimation = alignment` the two chains of `alignment`
 * stop being renamed apart and the callee-saved order rotates (27). The
 * self-defining spelling `firstAnimation = firstAnimation & 3` keeps the
 * chains apart and is 2, and those two are the carrier's home one slot below
 * the ROM's: homes descend from the frame top in declaration order (L99), so
 * `firstAnimation` is declared first. Frame 0x38, seven slots, ten relocation
 * identities, 106 of 106 words.
 */
s32 func_8005A7A0(ModelAnimationTable *model, s32 modelId) {
    s32 firstAnimation;
    s32 alignment;
    s32 lastAnimation;
    s32 loadSize;
    s32 loaded;
    s32 inputOffset;

    piRomLoadSection(0x28, (void *)D_800D7D00, (modelId & ~3) * 2, 0x10);
    firstAnimation = ((u16 *)D_800D7D00 + (modelId & 3))[0] >> 1;
    lastAnimation = ((u16 *)D_800D7D00 + (modelId & 3))[1] >> 1;
    model->animationCount = lastAnimation - firstAnimation;
    if (firstAnimation == lastAnimation) {
        return TRUE;
    }

    alignment = firstAnimation & 3;
    loadSize = ((model->animationCount & ~3) + 4) << 1;
    if (alignment != 0) {
        loadSize += 8;
    }
    piRomLoadSection(0x29, (void *)D_800D7CFC, (firstAnimation & ~3) * 2, loadSize);
    firstAnimation = firstAnimation & 3;
    model->animations = (u8 **)func_8002B314(model->animationCount * 4, 0x80);
    if (model->animations == NULL) {
        return FALSE;
    }

    loaded = 0;
    inputOffset = firstAnimation * 2;
    alignment = 0;
    do {
        *(u8 **)((u8 *)model->animations + alignment) =
            func_8005A948(*(s16 *)(D_800D7CFC + inputOffset));
        if (*(u8 **)((u8 *)model->animations + alignment) == NULL) {
            alignment = 0;
            if (loaded > 0) {
                inputOffset = 0;
                do {
                    func_8005AAC0(*(u8 **)((u8 *)model->animations + inputOffset));
                    alignment++;
                    inputOffset += 4;
                } while (alignment != loaded);
            }
            mmFree(model->animations);
            model->animations = NULL;
            return FALSE;
        }
        loaded++;
        inputOffset += 2;
        alignment += 4;
    } while (loaded < model->animationCount);
    return TRUE;
}
/* The two boolean spellings in this function are one edit and neither of them
 * works alone. ugen materialises a `(relational) == 0` test as a `seq`/`beq`
 * pair, which as1 fuses back into a single branch, so the temporary is
 * consumed but never emitted: it is a free +1 step on ugen's temp ring, with
 * no instruction and no size cost. `!(x)`, `(x) != 0` and `(x) != 0U` all fold
 * back to a bare branch and burn nothing, while `== 1`, `!= 1` and `^ 1` emit
 * a real instruction and cost two words. That gives a ring-phase dial with
 * three settings, usable at any branch site in either direction.
 *
 * The target burns its ring temp at the loop guard rather than at the inner
 * compare, so here the guard carries the normalisation and the compare is
 * plain. Alone, the guard edit is 23 differing words and dropping the inner
 * `!= 0U` is 26; together they are exact. Do not "simplify" either one.
 *
 * The owned 0x8005A948..0x8005AAC0 / ROM 0x5B548..0x5B6C0 range has no
 * padding. func_8005A7A0+0x104 is the sole caller, passing an lh animation ID;
 * there is no export, runtime, overlay or pointer inbound. */
u8 *func_8005A948(s16 animationId) {
    s32 i;
    s32 emptyIndex;
    s32 offset;
    s32 size;
    LoadedAnimation *animation;
    s32 tableOffset;

    emptyIndex = -1;
    i = 0;
    if ((D_800D7D04 <= 0) == 0) {
        do {
            AnimationCacheEntry *entry = &((AnimationCacheEntry *)D_800D7CF4)[i];

            if (animationId == entry->id) {
                u8 *existing = entry->animation;

                existing[0]++;
                return existing;
            }
            if (entry->id == -1) {
                emptyIndex = i;
            }
            i++;
        } while (i < D_800D7D04);
    }

    if (emptyIndex == -1) {
        emptyIndex = D_800D7D04;
        if (D_800D7D04 >= 0x100) {
            return NULL;
        }
        D_800D7D04++;
    }

    tableOffset = (animationId & 1) * 4;
    piRomLoadSection(0x2A, (u8 *)D_800D7CF8, (animationId & ~1) * 4, 0x10);
    offset = *(s32 *)(D_800D7CF8 + tableOffset);
    size = *(s32 *)(D_800D7CF8 + tableOffset + 4) - offset;
    animation = (LoadedAnimation *)func_8002B314(size, 0x80);
    if (animation == NULL) {
        return NULL;
    }

    piRomLoadSection(0x2B, animation, offset, size);
    animation->references = 1;
    animation->id = animationId;
    ((s32 *)D_800D7CF4)[emptyIndex * 2] = animationId;
    ((u8 **)D_800D7CF4)[(emptyIndex * 2) + 1] = (u8 *)animation;
    return (u8 *)animation;
}

/* PROVENANCE: Mickey-only reconstruction informed by JFG's corresponding
 * modFreeAnim identity and structure; the public JFG peer remained assembly,
 * and no external C body is copied. */
/* Retained configured C is instruction-identical to an independently rebuilt
 * historical target across all 46 words, with frame 0x20 and exact target
 * relocations: D_800D7D04 HI/LO at +0x14/+0x28, D_800D7CF4 HI/LO at
 * +0x38/+0x3C and +0x7C/+0x84, and mmFree R_MIPS_26 at +0x74. Linked
 * function/TU/resident bytes are exact; a fresh current-source compile through
 * full-ROM comparison remains as a contemporaneous reproof. */
void func_8005AAC0(u8 *animation) {
    s32 i;
    s32 index;

    if (animation != NULL) {
        animation[0]--;
        if (animation[0] > 0) {
            return;
        }
        index = -1;
        if (D_800D7D04 > 0) {
            i = 0;
            do {
                if (animation == ((u8 **)D_800D7CF4)[(i << 1) + 1]) {
                    index = i;
                }
                i++;
            } while (i < D_800D7D04);
        }
        if (index != -1) {
            mmFree(animation);
            ((s32 *)D_800D7CF4)[index * 2] = -1;
            ((s32 *)D_800D7CF4)[index * 2 + 1] = -1;
        }
    }
}
/* PROVENANCE: adapted from JFG src/camera.c (camConvertMatrixList). */
void camConvertMatrixList(Matrix *mtx, s32 count) {
    s32 index = D_800D7CF0;
    ConvListEntry *entry = &D_800D78F0[index];

    entry->mtx = mtx;
    D_800D7CF0 = index + 1;
    entry->count = count;
}

/* Keep the original TU order: func_8005ABA8 precedes func_8005AD64. */
/* Workbench: allocation-mismatch, 2 differing words, first mismatch +0x3C.
 * 111/111 words, frameless, zero relocations on both sides.
 *
 * Four source artefacts closed 45 of the 47, none of which was an allocator
 * question (2026-09-09):
 *  - `temp_f0_2` cached `instance->frameValue` for two tests that no store
 *    separates. The cache costs a `mov.s` where the target reads the field
 *    twice and lets uopt common the load; the target's own `nop` at that
 *    hazard slot is what the copy was filling.
 *  - the two blend stores were emitted sub-then-div; the target allocates the
 *    div's FP temp first, so the source computes `blendStart` before
 *    `blendEnd`. Both read only locals, so the order is free.
 *  - the null test spelled through `temp_a1` gave the loaded pointer a copy
 *    and exchanged a1/a2 on both frame carriers plus their two later uses.
 *    Testing `temp_v0->frame == NULL` directly lets the load keep a1 and the
 *    surviving carrier take the copy into a2.
 *  - `var_v1 = 1` written before the inner `if` of each arm, rather than once
 *    after it, changed nothing in the schedule but made as1 duplicate the
 *    join's `move v0,v1` into two annulled delay slots the target leaves as
 *    `nop`. That is the same class as the one word still open.
 * `temp_f2_2` was an m2c-only second name: one carrier serves both blendEnd
 * reads. Removing it and the dead `temp_a1` is byte-inert.
 *
 * What is left: at +0x3C the target branches `beqz` with a `nop` delay slot
 * to a block whose first scheduled instruction is `mul.s $f18,$f14,$f12`;
 * as1 turns the same branch into `beqzl` and duplicates that multiply into
 * the annulled slot, retargeting past it (the copy at +0xD8 then becomes
 * unreachable, so both sides are 111 words). Every other word, every branch
 * target and the whole register assignment agree. This is an as1 delay-slot
 * decision, not a codegen one, and it is reachable from source: the
 * `var_v1 = 1` move above flipped the same decision at two other sites
 * without moving a single instruction. Twenty-eight further shapes of the
 * else-block head, the transition test, the declaration list and the
 * comparison spellings are all flat at 2.
 *
 * Tooling note: the permuter's isolated scratch for this TU compiles the
 * function at 112 words against the real object's 111, so its base score of
 * 400 is a false reading and no score from it transfers.
 *
 * Second pass, 2026-09-09: the phase input is now proved correct. Replaying
 * the compiler's own listing through its preprocessor, first and second
 * assembler passes reproduces this object exactly, and inserting a single
 * location-counter directive at the else arm's label there suppresses the
 * duplication and yields a byte-exact 111 words. So every other word of this
 * C is already the target's C. The suppressing set is exactly the three
 * location-counter directives, which ugen emits only at function starts, so
 * it is not reachable from source. Retired for this residual: all debug-line
 * edits (hence physical line grouping), every other in-body directive, any
 * single-line move of the phase input, any neighbouring function, and 384 C
 * spellings of the tests, loops, blend order and carrier placement. See
 * docs/matching-triage-handoffs/func_8005ABA8.md. */
/* PROVENANCE: Mickey-only reconstruction from func_8005ABA8.s and the
 * existing models TU layouts; no external function body is copied. */
#ifdef NON_MATCHING
s32 func_8005ABA8(ModelAnimationInstance *instance, f32 arg1, f32 arg2) {
    s32 var_v1;
    f32 temp_f0;
    f32 temp_f2;
    ModelAnimationFrame *frame;
    ModelAnimationState *temp_v0;

    temp_v0 = instance->states[(s32)instance->animationIndex];
    var_v1 = 0;
    if (temp_v0->frame == NULL) {
        return 0;
    }
    frame = (ModelAnimationFrame *)temp_v0->frame;
    if (temp_v0->transition != 0) {
        if (temp_v0->hasNext != 0) {
            temp_f0 = temp_v0->blendValue + arg2;
            temp_f2 = temp_v0->blendEnd;
            temp_v0->blendValue = 0.0f;
            temp_v0->blendStart = temp_f0 / temp_f2;
            temp_v0->blendEnd = temp_f2 - temp_f0;
        } else {
            temp_v0->blendValue = temp_v0->blendValue + arg2;
        }
        temp_f2 = temp_v0->blendEnd;
        if ((temp_f2 <= 0.0f) || (temp_f2 <= temp_v0->blendValue)) {
            temp_v0->transition = 0;
            temp_v0->blendStart = 0.0f;
            temp_v0->blendValue = 0.0f;
            instance->frameValue = (f32)temp_v0->frameIndex /
                                   temp_v0->frameValue;
        }
    } else {
        instance->frameValue += arg1 * arg2;
        if (instance->frameValue >= 1.0f) {
            if (frame->loop != 0) {
                if (instance->frameValue >= 1.0f) {
                    do {
                        instance->frameValue -= 1.0f;
                    } while (instance->frameValue >= 1.0f);
                }
            } else {
                instance->frameValue = 1.0f;
            }
            var_v1 = 1;
        } else if (instance->frameValue < 0.0f) {
            if (frame->loop != 0) {
                if (instance->frameValue < 0.0f) {
                    do {
                        instance->frameValue += 1.0f;
                    } while (instance->frameValue < 0.0f);
                }
            } else {
                instance->frameValue = 0.0f;
            }
            var_v1 = 1;
        }
    }
    return var_v1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005ABA8.s")
#endif
/* PROVENANCE: Mickey-only reconstruction from func_8005AD64.s and the
 * existing models TU layouts; no external function body is copied. */
/* Exact configured C: 108 words, no stack frame or relocations. The canonical
 * linked resident range and full ROM are byte-identical. */
void func_8005AD64(ModelAnimationInstance *instance, s32 frame, s32 arg2,
                   f32 value) {
    f32 temp_f0;
    s32 temp_f6;
    s32 var_a1;
    s32 temp_a1;
    s32 var_v1;
    ModelAnimationFrame *temp_a0;
    ModelAnimationState *temp_v0;
    ModelAnimationInfo *temp_v1;

    temp_v0 = instance->states[(s32)instance->animationIndex];
    temp_v1 = temp_v0->info;
    if (temp_v1->frameCount != 0) {
        if (value > 1.0f) {
            value = 1.0f;
        } else if (value < 0.0f) {
            value = 0.0f;
        }
        instance->frameValue = value;
        temp_a1 = temp_v1->frameCount;
        if (frame >= temp_a1) {
            frame = temp_a1 - 1;
        } else if (frame < 0) {
            frame = 0;
        }
        instance->frame = frame;
        var_a1 = 0;
        if ((temp_v0->frame != NULL) && (temp_v0->hasNext != 0)) {
            var_a1 = 1;
        }
        temp_a0 = temp_v1->frames[frame];
        temp_v0->frame = temp_a0;
        temp_v0->frameData = (u8 *)temp_a0 + temp_a0->offset + 0x14;
        temp_v0->frameValue = (f32)temp_a0->count;
        if (temp_a0->loop == 0) {
            temp_v0->frameValue = temp_v0->frameValue - 1.0f;
        }
        if (arg2 != -1) {
            var_v1 = arg2;
        } else {
            var_v1 = temp_a0->flags;
        }
        if ((var_a1 != 0) && (var_v1 != 0)) {
            temp_v0->blendEnd = (f32)var_v1;
            temp_f0 = temp_v0->frameValue * value;
            temp_v0->transition = 1;
            temp_v0->blendStart = 0.0f;
            temp_f6 = (s32)temp_f0;
            if ((temp_f0 - (f32)temp_f6) >= 0.5f) {
                temp_v0->frameIndex = temp_f6 + 1;
                return;
            }
            temp_v0->frameIndex = temp_f6;
        }
    }
}

/*
 * Plateau: the animation-frame update's closest reconstruction emits 110
 * instructions against 111 and follows the broad target CFG, but diverges at
 * +0x38 before cascading through the FP allocator. The 119-combination flag
 * lattice found no exact result; its closest alternate still differs in 59
 * words and would also perturb this TU's already-exact functions.
 */
/*
 * PROVENANCE: field roles were cross-checked against Jet Force Gemini's
 * permitted public decomp at pinned commit c82affff, specifically
 * include/structs.h (ObjectModel/ModelInstance), src/models.h, and
 * src/camera.h. JFG's peer body remains assembly; Mickey's offsets, node
 * selection, control flow, and call sequence are reconstructed from Mickey.
 */
/* Workbench: structure-mismatch, 377 differing words, first mismatch +0x0. */
/* Structural gap: target 460 instructions/frame -0xF8 versus candidate 463/-0x110; camera-angle stack layout remains unresolved. */
/* Next: constant-audit the earliest immediate, then repair structure before register allocation (workbench mixed-residual routing). */
/* Not shape-exact or permuter-ready; model matrix and attachment-point control flow are represented. */
#ifdef NON_MATCHING
void func_8005AF14(ModelRenderInstance *instance, ModelRenderContext *context,
                   ModelRenderModel *model) {
    s32 matrixList;
    ModelRenderSlot *slot;
    Matrix *activeMatrices;
    ModelRenderAsset *asset;
    ModelRenderCamera *camera;
    ModelRenderNodeData *nodeData;
    ModelRenderMatrixNode *matrixNode;
    ModelRenderPointA *pointA;
    ModelRenderPointB *pointB;
    ModelRenderVertex *vertex;
    void *assetPart;
    u8 *matrixBase;
    f32 *output;
    f32 scale;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 sine;
    f32 cosine;
    s16 yaw;
    s16 pitch;
    s16 angle;
    s16 rawAngle;
    s16 clampedAngle;
    s16 scaledAngle;
    s32 index;
    s32 pointOffset;
    s32 temp;

    {
        Matrix matrix;

        scale = 1.0f;
        if (model->type == 1) {
            asset = model->asset;
            func_8002AA50((u8 *) asset + 0x43C, matrix);
            if (asset->scale != 1.0f) {
                scale = asset->scale;
                func_80029AB8(matrix, asset->scale);
            }
        } else {
            func_8002AA50(model, matrix);
        }

        instance->activeSlot ^= 1;
    slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
    matrixList = (s32) slot->matrices;
    instance->count = slot->count;
    if (instance->animated == 0) {
        func_8005B644((Matrix *) matrixList, matrix, context->nodes, context->matrixCount);
    } else {
        instance->offset = model->scale * instance->scale;
        switch (model->type) {
        case 1:
            assetPart = (u8 *) model->asset + 0x1B8;
            break;
        case 0x36:
            assetPart = (u8 *) model->asset + 0x3E;
            break;
        case 0x37:
            assetPart = (u8 *) model->asset + 0x28;
            break;
        case 0x54:
            assetPart = (u8 *) model->asset + 0x1C;
            break;
        case 0x56:
            assetPart = (u8 *) model->asset + 0x10;
            break;
        default:
            assetPart = NULL;
            break;
        }
        func_800591B0((Matrix *) matrixList, matrix, instance, context->nodes, assetPart);
        instance->mode = 2;
        }
    }

    if ((model->flags & 0x1000) != 0) {
        ModelRenderTransform transform;

        camera = camGetListPtr();
        if ((model->type == 1) && (asset->cameraIndex >= 0)) {
            temp = asset->cameraIndex;
            if (camGetMode() >= temp) {
                camera += temp;
            }
        }
        slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
        activeMatrices = slot->matrices;
        matrixBase = (u8 *) activeMatrices;
        matrixNode = (ModelRenderMatrixNode *) (matrixBase + 0x240);
        deltaX = camera->x - matrixNode->x;
        deltaY = camera->y - matrixNode->y;
        deltaZ = camera->z - matrixNode->z;
        yaw = Arctanf(deltaX, deltaZ);
        if (deltaY < 0.0f) {
            deltaY *= deltaY;
        } else {
            deltaY = -(deltaY * deltaY);
        }
        pitch = Arctanf(deltaY, (deltaX * deltaX) + (deltaZ * deltaZ));
        func_8002B040(matrixBase + 0x200, 0, 0, 0x3F800000,
                      &deltaX, &deltaY, &deltaZ);
        angle = -yaw;
        sine = func_8002A8C0(angle);
        cosine = func_8002A8BC(angle);
        rawAngle = Arctanf(-((deltaX * cosine) + (deltaZ * sine)),
                           (deltaZ * cosine) - (deltaX * sine));
        clampedAngle = rawAngle;
        if (rawAngle >= 0x4001) {
            clampedAngle = 0x4000 - (rawAngle - 0x4000);
        } else if (rawAngle < -0x4000) {
            clampedAngle = -0x4000 - (rawAngle + 0x4000);
        }
        scaledAngle = (s16) ((s32) (((f32) clampedAngle / 16384.0f) * 8192.0f));
        func_8002B040(matrixBase + 0x200, 0, 0x3F800000, 0,
                      &deltaX, &deltaY, &deltaZ);
        sine = func_8002A8C0(angle);
        transform.rotation2 = scaledAngle +
                               Arctanf(-((deltaX * func_8002A8BC(angle)) +
                                          (deltaZ * sine)), deltaY);
        transform.rotation0 = yaw;
        transform.rotation1 = pitch;
        transform.scale = model->transformScale;
        nodeData = (ModelRenderNodeData *) context->nodes;
        mtxf_transform_point((Matrix *) (matrixBase + 0x200), nodeData->x,
                             nodeData->y, nodeData->z, &transform.x,
                             &transform.y, &transform.z);
        func_8002AA50(&transform, matrixNode);
        func_80029AB8(matrixNode, scale);
    }

    if ((model->type == 1) && (func_800290A0() == 0)) {
        slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
        func_8002B040((u8 *) slot->matrices, 0, 0, 0x3F800000,
                      &deltaX, &deltaY, &deltaZ);
        asset->angle = Arctanf(deltaX, deltaZ);
    }

    output = instance->vertices[0];
    index = 0;
    pointOffset = 0;
    if ((s32) context->count0 > 0) {
        do {
            pointA = (ModelRenderPointA *) ((u8 *) context->points0 + pointOffset);
            vertex = (ModelRenderVertex *) (context->vertexData + (pointA->vertex * 0xA));
            slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
            activeMatrices = slot->matrices;
            mtxf_transform_point((Matrix *) ((u8 *) activeMatrices + (pointA->node << 6)),
                                 (f32) vertex->x, (f32) vertex->y, (f32) vertex->z,
                                 output, output + 1, output + 2);
            index++;
            pointOffset += 4;
            output += 3;
        } while (index < (s32) context->count0);
    }

    output = instance->vertices[1];
    index = 0;
    pointOffset = 0;
    if ((s32) context->count1 > 0) {
        do {
            pointB = (ModelRenderPointB *) ((u8 *) context->points1 + pointOffset);
            vertex = (ModelRenderVertex *) (context->vertexData + (pointB->vertex * 0xA));
            slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
            activeMatrices = slot->matrices;
            mtxf_transform_point((Matrix *) ((u8 *) activeMatrices + ((s32) pointB->node << 6)),
                                 (f32) vertex->x, (f32) vertex->y, (f32) vertex->z,
                                 output, output + 1, output + 2);
            index++;
            pointOffset += 0xC;
            output += 3;
        } while (index < (s32) context->count1);
    }

    output = instance->vertices[2];
    index = 0;
    pointOffset = 0;
    if ((s32) context->count2 > 0) {
        do {
            pointB = (ModelRenderPointB *) ((u8 *) context->points2 + pointOffset);
            vertex = (ModelRenderVertex *) (context->vertexData + (pointB->vertex * 0xA));
            slot = (ModelRenderSlot *) ((u8 *) instance + (instance->activeSlot * 4));
            activeMatrices = slot->matrices;
            mtxf_transform_point((Matrix *) ((u8 *) activeMatrices + ((s32) pointB->node << 6)),
                                 (f32) vertex->x, (f32) vertex->y, (f32) vertex->z,
                                 output, output + 1, output + 2);
            index++;
            pointOffset += 0xC;
            output += 3;
        } while (index < (s32) context->count2);
    }
    camConvertMatrixList((Matrix *) matrixList, context->matrixCount);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AF14.s")
#endif

/* Mickey-derived parented matrix-list builder; JFG retains its peer as asm. */
void func_8005B644(Matrix *matrices, Matrix *root, ModelMatrixNode *node, s32 count) {
    Matrix temp;
    Matrix *output;
    Matrix *parent;
    s32 i;

    output = matrices;
    i = 0;
    if (count > 0) {
        do {
            func_8002A82C((u8 *)temp - 8);
            *(f32 *)((u8 *)temp + 0x28) = node->x;
            *(f32 *)((u8 *)temp + 0x2C) = node->y;
            *(f32 *)((u8 *)temp + 0x30) = node->z;
            if ((node->parent == -1) != FALSE) {
                parent = root;
            } else {
                parent = &matrices[node->parent];
            }
            mtxf_mul((u8 *)temp - 8, parent, output);
            i++;
            output++;
            node++;
        } while (i != count);
    }
}

/* PLATEAU-HANDOFF:func_8005ABA8:start
 * symbol: func_8005ABA8
 * score: 2/111 words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x3C
 * summary: as1 branch-delay decision remains; source-side C and phase replay are closed
 * PLATEAU-HANDOFF:func_8005ABA8:end
 */

/* PLATEAU-HANDOFF:func_8005AF14:start
 * symbol: func_8005AF14
 * score: 377 differing words
 * frame: 0x110
 * relocations: 27
 * first-mismatch: +0x0
 * summary: split-not-copy at rotation2 line 771. Frame-close reorder hits 0xF8 but aligned residual 376>327; the +0x24 callee save stays missing. Stall.
 * PLATEAU-HANDOFF:func_8005AF14:end
 */
