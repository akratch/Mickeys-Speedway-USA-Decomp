#define Z1 0.0f
#define Z2 0.0f
#define Z3 0.0f

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
    Matrix *matrices[2];
    s32 counts[2];
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

typedef struct ModelRenderAsset {
    s8 cameraIndex;
    u8 pad1[0x4F];
    f32 scale;
    u8 pad54[0x3E8];
    ModelRenderTransform transform;
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
    u8 pad6[4];
} ModelRenderVertex;


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
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
void func_8002B040(void *matrix, f32 x, f32 y, f32 z,
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

/* Exact stock C output proved 2026-09-30: 111/111 words, frameless, zero relocations.
 * The empty `instance` guard the search left in the non-transition branch was
 * byte-inert and is gone (lane c-4, 2026-10-09). */
/* PROVENANCE: Mickey-only reconstruction from func_8005ABA8.s and the
 * existing models TU layouts; no external function body is copied. */
s32 func_8005ABA8(ModelAnimationInstance *instance, f32 speed, f32 timeStep) {
    s32 result;
    f32 elapsed;
    f32 duration;
    ModelAnimationFrame *frame;
    ModelAnimationState *state;

    state = instance->states[(s32)instance->animationIndex];
    result = 0;
    if (state->frame == NULL) {
        return 0;
    }
    frame = (ModelAnimationFrame *)state->frame;
    if (state->transition != 0) {
        if (state->hasNext != 0) {
            elapsed = state->blendValue + timeStep;
            duration = state->blendEnd;
            state->blendValue = 0.0f;
            state->blendStart = elapsed / duration;
            state->blendEnd = duration - elapsed;
        } else {
            state->blendValue = state->blendValue + timeStep;
        }
        duration = state->blendEnd;
        if ((duration <= 0.0f) || (duration <= state->blendValue)) {
            state->transition = 0;
            state->blendStart = 0.0f;
            state->blendValue = 0.0f;
            instance->frameValue = (f32)state->frameIndex /
                                   state->frameValue;
        }
    } else {
        instance->frameValue += speed * timeStep;
        if (instance->frameValue >= 1.0f) {
            if (frame->loop != 0) {
                while (instance->frameValue >= 1.0f) {
                    instance->frameValue -= 1.0f;
                }
            } else {
                instance->frameValue = 1.0f;
            }
            result = 1;
        } else if (instance->frameValue < 0.0f) {
            if (frame->loop != 0) {
                while (instance->frameValue < 0.0f) {
                    instance->frameValue += 1.0f;
                }
            } else {
                instance->frameValue = 0.0f;
            }
            result = 1;
        }
    }
    return result;
}
/* PROVENANCE: Mickey-only reconstruction from func_8005AD64.s and the
 * existing models TU layouts; no external function body is copied. */
/* Exact configured C: 108 words, no stack frame or relocations. The canonical
 * linked resident range and full ROM are byte-identical. */
void func_8005AD64(ModelAnimationInstance *instance, s32 frame, s32 blendTime,
                   f32 value) {
    f32 position;
    s32 frameIndex;
    s32 blending;
    s32 frameCount;
    s32 duration;
    ModelAnimationFrame *animFrame;
    ModelAnimationState *state;
    ModelAnimationInfo *info;

    state = instance->states[(s32)instance->animationIndex];
    info = state->info;
    if (info->frameCount != 0) {
        if (value > 1.0f) {
            value = 1.0f;
        } else if (value < 0.0f) {
            value = 0.0f;
        }
        instance->frameValue = value;
        frameCount = info->frameCount;
        if (frame >= frameCount) {
            frame = frameCount - 1;
        } else if (frame < 0) {
            frame = 0;
        }
        instance->frame = frame;
        blending = 0;
        if ((state->frame != NULL) && (state->hasNext != 0)) {
            blending = 1;
        }
        animFrame = info->frames[frame];
        state->frame = animFrame;
        state->frameData = (u8 *)animFrame + animFrame->offset + 0x14;
        state->frameValue = (f32)animFrame->count;
        if (animFrame->loop == 0) {
            state->frameValue = state->frameValue - 1.0f;
        }
        if (blendTime != -1) {
            duration = blendTime;
        } else {
            duration = animFrame->flags;
        }
        if ((blending != 0) && (duration != 0)) {
            state->blendEnd = (f32)duration;
            position = state->frameValue * value;
            state->transition = 1;
            state->blendStart = 0.0f;
            frameIndex = (s32)position;
            if ((position - (f32)frameIndex) >= 0.5f) {
                state->frameIndex = frameIndex + 1;
                return;
            }
            state->frameIndex = frameIndex;
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
/* The TU uses -Wab,-r4300_mul for the target multiply-hazard spacing.
 * Rewritten plainly (lane b-models, 2026-10-07): the slot arrays as
 * matrices[2]/counts[2], the head and neck as &matrices[slot][8] and [9],
 * node 9's position read through a pointer, the three point loops indexed,
 * both clamp arms updating clampedAngle in place (the narrowing frees the
 * intermediate before the constant, as shipped), the second Arctanf result
 * held in rawAngle so the add reads clampedAngle first, and a redundant
 * 0xFFFF mask on that sum (one deleted draw, L149). Each point loop takes a
 * vertex pointer first and indexes the slot's matrices as a flat f32 array,
 * [node * 16]: ugen shifts the node by 4 and the subscript multiplies by 4,
 * and as1 folds the two into one sll while the second draw stays spent
 * (L149), which is the ring draw the target spends before the node load in
 * every loop (lane h-4, 2026-10-07; 32 to 0). */
void func_8005AF14(ModelRenderInstance *instance, ModelRenderContext *context,
                   ModelRenderModel *model) {
    ModelRenderAsset *asset;
    ModelRenderCamera *camera;
    f32 *output;
    s32 i;
    s32 angle;
    Matrix *matrixList;
    Matrix *neck;
    Matrix *head;
    void *assetPart;
    ModelMatrixNode *node;
    ModelRenderTransform transform;
    Matrix matrix;
    f32 scale;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 sine;
    f32 cosine;
    ModelRenderVertex *vertex;
    s16 yaw;
    s16 pitch;
    s16 rawAngle;
    s16 clampedAngle;
    s32 unused;
    u16 excess;

    scale = 1.0f;
    if (model->type == 1) {
        asset = model->asset;
        func_8002AA50(&asset->transform, matrix);
        if (asset->scale != 1.0f) {
            scale = asset->scale;
            func_80029AB8(matrix, asset->scale);
        }
    } else {
        func_8002AA50(model, matrix);
    }

    /* A one-statement region: it adds the block that puts model one
     * division below the neck matrix on save, so the neck takes s2. */
    do { instance->activeSlot ^= 1; } while (0);
    matrixList = instance->matrices[instance->activeSlot];
    instance->count = instance->counts[instance->activeSlot];
    if (instance->animated == 0) {
        func_8005B644(matrixList, matrix, context->nodes, context->matrixCount);
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
        func_800591B0(matrixList, matrix, instance, context->nodes, assetPart);
        instance->mode = 2;
    }

    if (model->flags & 0x1000) {
        camera = camGetListPtr();
        if (model->type == 1 && asset->cameraIndex >= 0) {
            if (camGetMode() >= asset->cameraIndex) {
                camera = &camera[asset->cameraIndex];
            }
        }
        head = &instance->matrices[instance->activeSlot][8];
        neck = &instance->matrices[instance->activeSlot][9];
        dx = camera->x - (*neck)[3][0];
        dy = camera->y - (*neck)[3][1];
        dz = camera->z - (*neck)[3][2];
        yaw = Arctanf(dx, dz);
        if (dy < 0.0f) {
            dy = dy * dy;
        } else {
            dy = -(dy * dy);
        }
        pitch = Arctanf(dy, (dx * dx) + (dz * dz));
        func_8002B040(head, Z1, Z1, 1.0f, &dx, &dy, &dz);
        angle = -yaw;
        sine = func_8002A8C0(angle);
        cosine = func_8002A8BC(angle);
        rawAngle = Arctanf(-((dx * cosine) + (dz * sine)), (dz * cosine) - (dx * sine));
        clampedAngle = rawAngle;
        if (rawAngle > 0x4000) {
            clampedAngle = clampedAngle - 0x4000;
            clampedAngle = 0x4000 - clampedAngle;
        } else if (rawAngle < -0x4000) {
            clampedAngle = clampedAngle + 0x4000;
            clampedAngle = -0x4000 - clampedAngle;
        }
        clampedAngle = ((f32) clampedAngle / 16384.0f) * 8192.0f;
        func_8002B040(head, Z2, 1.0f, Z2, &dx, &dy, &dz);
        sine = func_8002A8C0(angle);
        cosine = func_8002A8BC(angle);
        rawAngle = Arctanf(-((dx * cosine) + (dz * sine)), dy);
        transform.rotation2 = (clampedAngle + rawAngle) & 0xFFFF;
        transform.rotation0 = yaw;
        transform.rotation1 = pitch;
        transform.scale = model->transformScale;
        node = &context->nodes[9];
        mtxf_transform_point(*head, node->x, node->y, node->z,
                             &transform.x, &transform.y, &transform.z);
        func_8002AA50(&transform, neck);
        func_80029AB8(neck, scale);
    }

    if (model->type == 1 && func_800290A0() == 0) {
        func_8002B040(instance->matrices[instance->activeSlot], Z3, Z3, 1.0f, &dx, &dy, &dz);
        asset->transform.rotation0 = Arctanf(dx, dz);
    }

    output = instance->vertices[0];
    for (i = 0; i < context->count0; i++) {
        vertex = (ModelRenderVertex *) &context->vertexData[context->points0[i].vertex * 10];
        mtxf_transform_point(*(Matrix *) &((f32 *) instance->matrices[instance->activeSlot])[context->points0[i].node * 16],
                             vertex->x, vertex->y, vertex->z,
                             output, output + 1, output + 2);
        output += 3;
    }
    output = instance->vertices[1];
    for (i = 0; i < context->count1; i++) {
        vertex = (ModelRenderVertex *) &context->vertexData[context->points1[i].vertex * 10];
        mtxf_transform_point(*(Matrix *) &((f32 *) instance->matrices[instance->activeSlot])[context->points1[i].node * 16],
                             vertex->x, vertex->y, vertex->z,
                             output, output + 1, output + 2);
        output += 3;
    }
    output = instance->vertices[2];
    for (i = 0; i < context->count2; i++) {
        vertex = (ModelRenderVertex *) &context->vertexData[context->points2[i].vertex * 10];
        mtxf_transform_point(*(Matrix *) &((f32 *) instance->matrices[instance->activeSlot])[context->points2[i].node * 16],
                             vertex->x, vertex->y, vertex->z,
                             output, output + 1, output + 2);
        output += 3;
    }
    camConvertMatrixList(matrixList, context->matrixCount);
}

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
