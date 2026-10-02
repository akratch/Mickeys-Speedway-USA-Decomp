/*
 * Sprite/object animation controls -- ROM 0x1BEA0-0x1C790.
 *
 * PROVENANCE -- the TU and seven descriptive function names are borrowed
 * from Jet Force Gemini's public retail-derived src/spranim.c and its
 * nonmatching assembly names.  The attribution is supported at tier B by
 * object-control call roles and at tier D by function order and masked
 * instruction shape.  No JFG body is adapted by this scaffold.
 */

#include "PR/ultratypes.h"

typedef struct SpranimBAE4Target {
    u8 pad0[0x132];
    s16 state132;
} SpranimBAE4Target;

typedef struct SpranimBAE4Object {
    u8 pad0[0x58];
    SpranimBAE4Target *target58;
} SpranimBAE4Object;

typedef struct SpranimBB10Header {
    u8 pad0[0x22];
    s8 count22;
} SpranimBB10Header;

typedef struct SpranimBB10Object {
    u8 pad0[0x3A];
    s8 index3A;
    u8 pad3B[5];
    SpranimBB10Header *header40;
    u8 pad44[0x24];
    void **entries68;
    u8 pad6C[0x1C];
    u32 flags88;
} SpranimBB10Object;

typedef struct SprasjiInitState {
    u8 pad0[8];
    f32 scale;
    u8 padC[0x34];
    f32 *baseScale;
} SprasjiInitState;

typedef struct SprasjiInitEntry {
    u8 pad0[0xB];
    u8 scale;
} SprasjiInitEntry;

typedef struct SpranimInitState {
    u8 pad0[8];
    f32 scale;
    u8 padC[0x1C];
    f32 initialValue;
    u8 pad2C[0x14];
    f32 *baseScale;
    u8 pad44[0x40];
    s32 animationId;
} SpranimInitState;

typedef struct SpranimInitEntry {
    u8 pad0[0xA];
    u8 animationId;
    u8 scale;
    u8 initialValue;
} SpranimInitEntry;

typedef struct SpranimControlState {
    u8 pad0[0x28];
    u8 animationState[0x40];
    void **entries;
    u8 pad6C[0x18];
    s32 animationId;
} SpranimControlState;

typedef struct TexscrollEntry {
    s16 textureIndex;
    s16 pad2;
    s16 speedX;
    s16 speedY;
    s16 offsetX;
    s16 offsetY;
} TexscrollEntry;

typedef struct TexscrollState {
    u8 pad0[0x64];
    TexscrollEntry *entry;
} TexscrollState;

typedef struct SpranimOnceState {
    u8 pad0[0x28];
    f32 value;
    u8 pad2C[0x3C];
    void **entries;
    u8 pad6C[0x18];
    s32 animationId;
} SpranimOnceState;

typedef struct RangetriggerEntry {
    u8 pad0[0xA];
    u16 radius;
    u16 triggerId;
} RangetriggerEntry;

typedef struct RangetriggerState {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    RangetriggerEntry *entry;
    u8 pad40[0x40];
    s32 activeTrigger;
} RangetriggerState;

typedef struct SpranimPlane {
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
    f32 radius;
    f32 maxY;
    s16 mode;
    s16 parameter;
} SpranimPlane;

typedef struct SpranimB798Object {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state64;
} SpranimB798Object;

typedef struct SpranimB798Target {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state64;
} SpranimB798Target;

extern u8 D_8007BF2C;
extern u8 D_8007BF0C;
extern void func_80006EA0(void *object);
/* The exact caller passes owner/context in a3, which the target callee
 * overwrites without consuming. Keep this four-argument declaration local;
 * the guarded callee's three-argument definition preserves its frame. */
extern void func_80020D8C(void *arg0, s32 arg1, s32 arg2, void *arg3);
extern void func_8000D16C(s16 textureIndex, s32 x, s32 y, s32 updateRate);
extern void func_80036544(void *entry, s32 *mode, s32 animationId, void *state, s32 updateRate);
extern s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 useXZ, void *hits);
extern void partUpdateTriggers(void *state, s32 updateRate);
extern void **func_80005750(s32 *count);
extern void animseqPlay();
extern void animseqResetGroup();
extern s32 TrapDanglingJump();

/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimInit.s, with Mickey's offsets. */
void spranimInit(SpranimInitState *state, SpranimInitEntry *entry) {
    f32 scale;

    scale = (s32) (entry->scale & 0xFF);
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale /= 64;
    state->scale = *state->baseScale * scale;
    state->animationId = entry->animationId;
    state->initialValue = entry->initialValue;
}
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimControl.s, with Mickey's offsets. */
void spranimControl(SpranimControlState *state, s32 updateRate) {
    s32 mode;

    mode = 9;
    func_80036544(*state->entries, &mode, state->animationId, state->animationState, updateRate);
}
void sprasjiInit(SprasjiInitState *state, SprasjiInitEntry *entry) {
    f32 scale;

    scale = (s32) (entry->scale & 0xFF);
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale /= 64;
    state->scale = *state->baseScale * scale;
}
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimOnceControl.s, with Mickey's offsets. */
void spranimOnceControl(SpranimOnceState *state, s32 updateRate) {
    s32 mode[2];
    f32 initialValue;
    void *entry;

    mode[1] = 9;
    entry = *state->entries;
    initialValue = state->value;
    func_80036544(entry, &mode[1], state->animationId, &state->value, updateRate);
    if (state->value < initialValue) {
        func_80006EA0(state);
    }
}
#ifdef NON_MATCHING
/* 47 masked words at size delta 0 and the exact 0x80 frame (was 51),
 * 2026-10-02 lane x-res: rewritten as a plain counted loop over the hit list
 * (IDO unrolls it by four, as in the target) with the hit read into a local
 * and the state pointer named before the store; the hit list is nine
 * entries, and four scalars declared above `entry`, `hits` and `state` put
 * the two spill homes at the target's 0x6C and 0x44 (the order of those four
 * is inert). The `planeIndex < 1` bound is the target's bgtz.
 * Left, priced with forces on proc 4 (CDX_PROC=4): the cursor web taking a3
 * and the index web a2 (target roles) is 47 -> 34; the remaining rows are the
 * hit load: uopt keeps the loaded pointer and the `hit` variable as two
 * interfering webs (v1 and a0, joined by a move in the active-test delay
 * slot) in the remainder loop and the first unrolled copy, where the target
 * has one web and loads the state pointer into a0. Loop form (for, do-while,
 * while), block-scope `hit`, `continue` form of the test, and five store
 * spellings were measured flat or worse; any second `hits[i]` read stops the
 * unroller. */
/* PROVENANCE: JFG's public effectboxControl assembly establishes the trigger/hit-list idiom; all Mickey offsets and calls below are reconstructed locally. */
typedef struct SpranimEffectBox {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state64;
} SpranimEffectBox;

typedef struct SpranimEffectState {
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
    s32 radius;
    s16 planeIndex;
    s16 active;
} SpranimEffectState;

extern u8 D_800794B0[];
extern s32 func_8002905C(u8 type, void *state);

void effectboxControl(SpranimEffectBox *arg0, s32 arg1) {
    s32 hitCount;
    s32 i;
    SpranimB798Target *hit;
    void *st;
    u8 *entry;
    SpranimB798Target *hits[9];
    SpranimEffectState *state;

    state = arg0->state64;
    if ((state->planeIndex >= 0) && (state->planeIndex < 1)) {
        entry = &D_800794B0[state->planeIndex * 4];
        if (entry[0] != 0xFF && func_8002905C(entry[0], state) != entry[1]) {
            return;
        }
        if (entry[2] != 0xFF && func_8002905C(entry[2], state) != entry[3]) {
            return;
        }
    }

    hitCount = func_8005776C(arg0->x, arg0->y, arg0->z, (f32) state->radius, 0, hits);
    if (hitCount != 0) {
        for (i = 0; i < hitCount; i++) {
            hit = hits[i];
            if ((state->active == 0) ||
                ((state->normalX * hit->x) + (state->normalY * hit->y) +
                 (state->normalZ * hit->z) + state->distance < 0.0f)) {
                st = hit->state64;
                *(void **) ((u8 *) st + 0xC8) = arg0;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/effectboxControl.s")
#endif
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/texscrollControl.s, with Mickey's object offset. */
void texscrollControl(TexscrollState *state, s32 updateRate) {
    s32 x;
    s32 y;
    TexscrollEntry *entry;

    entry = state->entry;
    x = updateRate;
    x = entry->speedX * x;
    x += entry->offsetX;
    entry->offsetX = x & 3;
    x >>= 2;
    y = entry->speedY * updateRate;
    y += entry->offsetY;
    entry->offsetY = y & 3;
    y >>= 2;
    func_8000D16C(entry->textureIndex, x, y, updateRate);
}
#ifdef NON_MATCHING
/* 2026-10-02 (lane x-res): 131 at size delta -16 -> 128 at delta 0. The
 * intersection point is a three-float array (the target keeps its y in a
 * stack home and reloads it for both height tests) and the plane radius is
 * read into a local; the plane fields are the typed SpranimPlane members.
 * Left: the frame (0xC0 here, 0xE0 in the target), the target's six
 * callee-saved FP webs (it rematerialises 0.0f at each compare where this
 * build hoists it), and the object's y/z spilled to homes at 0x8C/0x88. */
/* PROVENANCE: JFG's public character-plane control role supplies the idiom; Mickey's fields, globals, and action calls are authoritative below. */
void func_8001B798(SpranimB798Object *arg0, s32 arg1) {
    SpranimPlane *plane;
    SpranimB798Target **objects;
    SpranimB798Target *object;
    u8 *targetState;
    s32 count;
    s32 i;
    f32 firstDistance;
    f32 secondDistance;
    f32 fraction;
    f32 hit[3];
    f32 deltaX;
    f32 deltaZ;
    f32 radius;

    plane = arg0->state64;
    objects = (SpranimB798Target **) func_80005750(&count);
    for (i = 0; i < count; i++) {
        object = objects[i];
        targetState = object->state64;
        if ((*(u16 *)(targetState + 0x1A8) & 1) && (*(s8 *) targetState != 0)) {
            continue;
        }
        firstDistance = plane->distance +
            ((plane->normalX * object->x) + (plane->normalY * object->y) +
             (plane->normalZ * object->z));
        if (firstDistance < 0.0f) {
            secondDistance = plane->distance +
                ((plane->normalX * *(f32 *)(targetState + 0x38)) +
                 (plane->normalY * *(f32 *)(targetState + 0x3C)) +
                 (plane->normalZ * *(f32 *)(targetState + 0x40)));
            if (secondDistance >= 0.0f) {
                fraction = secondDistance / (secondDistance - firstDistance);
                hit[0] = *(f32 *)(targetState + 0x38) +
                         fraction * (object->x - *(f32 *)(targetState + 0x38));
                hit[1] = *(f32 *)(targetState + 0x3C) +
                         fraction * (object->y - *(f32 *)(targetState + 0x3C));
                hit[2] = *(f32 *)(targetState + 0x40) +
                         fraction * (object->z - *(f32 *)(targetState + 0x40));
                deltaX = hit[0] - arg0->x;
                deltaZ = hit[2] - arg0->z;
                radius = plane->radius;
                if (((deltaX * deltaX) + (deltaZ * deltaZ) <= radius) &&
                    (arg0->y <= hit[1]) && (hit[1] <= plane->maxY)) {
                    switch (plane->mode) {
                    case 0:
                        if (D_8007BF0C == 0) {
                            if (plane->parameter == 0) {
                                animseqResetGroup();
                                animseqPlay();
                            } else if (plane->parameter == 1) {
                                animseqPlay();
                            }
                        }
                        break;
                    case 1:
                        TrapDanglingJump(plane->parameter);
                        break;
                    case 2:
                        TrapDanglingJump();
                        break;
                    case 3:
                        TrapDanglingJump();
                        break;
                    }
                }
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/func_8001B798.s")
#endif
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/rangetriggerControl.s, with Mickey's offsets. */
void rangetriggerControl(RangetriggerState *state, s32 updateRate) {
    RangetriggerState *owner;
    RangetriggerEntry *entry;
    u64 hits[4];

    owner = state;
    entry = owner->entry;
    if (func_8005776C(owner->x, owner->y, owner->z, entry->radius, 1, hits) > 0) {
        owner->activeTrigger = entry->triggerId;
    } else {
        owner->activeTrigger = 0;
    }
    partUpdateTriggers(owner, updateRate);
}
void func_8001BAE4(SpranimBAE4Object *arg0, void *arg1) {
    arg0->target58->state132 = 1;
}
void func_8001BAF8(void *arg0, void *arg1) {
}
void func_8001BB04(void *arg0, void *arg1) {
}
void func_8001BB10(SpranimBB10Object *arg0, void *arg1) {
    s8 index;
    s32 frame;

    arg0->index3A = D_8007BF2C;
    index = arg0->index3A;
    if ((index < 0) || (index >= arg0->header40->count22)) {
        arg0->index3A = 0;
        index = arg0->index3A;
    }
    frame = (arg0->flags88 & 3) << 8;
    func_80020D8C(arg0->entries68[index], 0, frame, arg0);
}

/* PLATEAU-HANDOFF:effectboxControl:start
 * symbol: effectboxControl
 * score: 47/193 words
 * frame: 0x80
 * relocations: 5
 * first-mismatch: +0xDC
 * summary: 51 to 47: counted loop, hit local, nine-entry list, homes at 0x6C/0x44. Left: cursor/index a3/a2 roles (forced 34) and the hit load split in two webs
 * PLATEAU-HANDOFF:effectboxControl:end
 */

/* PLATEAU-HANDOFF:func_8001B798:start
 * symbol: func_8001B798
 * score: 128/175 words
 * frame: 0xC0
 * relocations: 9
 * first-mismatch: +0x0
 * summary: -16 to delta 0 (131 to 128): hit point as a three-float array, radius local. Left: frame 0xC0 vs target 0xE0 and the six callee-saved FP webs
 * PLATEAU-HANDOFF:func_8001B798:end
 */
