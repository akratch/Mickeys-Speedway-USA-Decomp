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
/* 51 masked words at size delta 0 and the exact 0x80 frame (was 57),
 * 2026-10-01 lane d-res2: every local declared once at the top of the
 * function (no nested scopes), the state pointer declared after the hit list
 * and the pointers before it, which moves the state spill from above the hit
 * list to below it as in the target (a 150 s declaration-order climb, 57 ->
 * 51). Left: the hit-index spill sits 0x10 above the target's 0x6C, and the
 * loop cursor/index pair takes a2/a3 in the opposite roles. */
/*
 * 2026-09-12 (lane p7-res2): the `st` local in the store body is load-bearing.
 * Naming the hit's state pointer before storing through it is worth three words
 * (60 -> 57): it draws the allocator slot that puts the second unrolled loop
 * body back on the target's temporaries (L76). Five other store spellings --
 * a typed destination cast, word-indexing at 0x32, and a typed hit list --
 * are byte-flat. The hit list stays at ten entries: eleven lengths were
 * measured and every other one is worse.
 */
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
    u8 *entry;
    SpranimB798Target *hit;
    s32 hitCount;
    s32 processed;
    void *hits[10];
    SpranimEffectState *state;
    void *st;

    state = arg0->state64;
    if ((state->planeIndex >= 0) && (state->planeIndex <= 0)) {
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
        processed = 0;
        if (hitCount > 0) {
            do {
                hit = ((SpranimB798Target **) hits)[processed];

                if ((state->active == 0) ||
                    ((state->normalX * hit->x) + (state->normalY * hit->y) +
                     (state->normalZ * hit->z) + state->distance < 0.0f)) {
                    st = hit->state64;

                    *(void **)((u8 *) st + 0xC8) = arg0;
                }
                processed++;
            } while (processed < hitCount);
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
/* Workbench verdict: structure-mismatch, 131 differing words, first mismatch +0x0. */
/* Candidate: 171/175 instructions, a 0xD0 frame versus target 0xE0, and 3/9 exact relocation identities. */
/* Shape status: signed plane tests, intersection arithmetic, and action dispatch are preserved; FP lifetimes and stack homes remain non-exact. */
/* PROVENANCE: JFG's public character-plane control role supplies the idiom; Mickey's fields, globals, and action calls are authoritative below. */
void func_8001B798(SpranimB798Object *arg0, s32 arg1) {
    SpranimPlane *plane;
    SpranimB798Target **objectPtr;
    void *targetState;
    s32 count;
    s32 i;
    f32 firstDistance;
    f32 secondDistance;
    f32 fraction;
    f32 hitX;
    f32 hitY;
    f32 hitZ;
    f32 deltaX;
    f32 deltaZ;
    f32 radius;

    plane = arg0->state64;
    objectPtr = (SpranimB798Target **) func_80005750(&count);
    for (i = 0; i < count; i++, objectPtr++) {
        SpranimB798Target *object = *objectPtr;

        targetState = object->state64;
        if ((*(u16 *)((u8 *) targetState + 0x1A8) & 1) &&
            (*(s8 *) targetState != 0)) {
            continue;
        }
        firstDistance = plane->distance +
            ((plane->normalX * object->x) + (plane->normalY * object->y) +
             (plane->normalZ * object->z));
        if (firstDistance < 0.0f) {
            secondDistance = plane->distance +
                ((plane->normalX * *(f32 *)((u8 *) targetState + 0x38)) +
             (plane->normalY * *(f32 *)((u8 *) targetState + 0x3C)) +
             (plane->normalZ * *(f32 *)((u8 *) targetState + 0x40)));
            if (secondDistance >= 0.0f) {
                fraction = secondDistance / (secondDistance - firstDistance);
                hitX = *(f32 *)((u8 *) targetState + 0x38) + fraction *
                    (object->x - *(f32 *)((u8 *) targetState + 0x38));
                hitY = *(f32 *)((u8 *) targetState + 0x3C) + fraction *
                    (object->y - *(f32 *)((u8 *) targetState + 0x3C));
                hitZ = *(f32 *)((u8 *) targetState + 0x40) + fraction *
                    (object->z - *(f32 *)((u8 *) targetState + 0x40));
                deltaX = hitX - arg0->x;
                deltaZ = hitZ - arg0->z;
                radius = plane->radius;
                if (((deltaX * deltaX) + (deltaZ * deltaZ) <= radius) &&
                    (arg0->y <= hitY) && (hitY <= plane->maxY)) {
                    switch (plane->mode) {
                    case 0:
                        if (D_8007BF0C == 0) {
                            if (*(s16 *)((u8 *) plane + 0x1A) == 0) {
                                animseqResetGroup();
                                animseqPlay();
                            } else if (*(s16 *)((u8 *) plane + 0x1A) == 1) {
                                animseqPlay();
                            }
                        }
                        break;
                    case 1:
                        TrapDanglingJump(*(s16 *)((u8 *) plane + 0x1A));
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
 * score: 51/193 words
 * frame: 0x80
 * relocations: 5
 * first-mismatch: +0x48
 * summary: 57 to 51 by declaration placement (state spill now below the hit list). Left: hit-index spill 0x7C vs 0x6C; cursor/index a2/a3 roles swapped
 * PLATEAU-HANDOFF:effectboxControl:end
 */

/* PLATEAU-HANDOFF:func_8001B798:start
 * symbol: func_8001B798
 * score: 44/175 words
 * frame: 0xD0
 * relocations: 9
 * first-mismatch: +0x0
 * summary: Configured O2/MIPS-II is best; target frame is 0xE0, only 3/9 relocation identities align, and the FP lifetime/stack-home web remains.
 * PLATEAU-HANDOFF:func_8001B798:end
 */
