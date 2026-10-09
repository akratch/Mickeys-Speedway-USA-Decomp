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

typedef struct SpranimB798State {
    s8 flag0;
    u8 pad1[0x37];
    f32 previousX;
    f32 previousY;
    f32 previousZ;
    u8 pad44[0x164];
    u16 flags;
} SpranimB798State;

extern u8 D_8007BF2C;
extern u8 D_8007BF0C;
extern void func_80006EA0(void *object);
/* The exact caller passes owner/context in a3, which the target callee
 * overwrites without consuming. Keep this four-argument declaration local;
 * the guarded callee's three-argument definition preserves its frame. */
extern void func_80020D8C(void *arg0, s32 arg1, s32 arg2, void *arg3);
extern void trackAddTextureScroll(s16 textureIndex, s32 x, s32 y, s32 updateRate);
extern void texAnimateSprite(void *entry, s32 *mode, s32 animationId, void *state, s32 updateRate);
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
    texAnimateSprite(*state->entries, &mode, state->animationId, state->animationState, updateRate);
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
    texAnimateSprite(entry, &mode[1], state->animationId, &state->value, updateRate);
    if (state->value < initialValue) {
        func_80006EA0(state);
    }
}
/* Matched 2026-10-02 (lane z-res) from lane x-res's 47-word body. The plain
 * counted loop over the hit list is unrolled by four, so `hits[i]` occurs in
 * the remainder loop and in the first copy and uopt saves it; it then
 * replaces a variable assigned from it wherever the block does not alter
 * memory. Two things were needed and each alone is worse: the hit is read
 * into `object` and copied to `hit`, which keeps the load in the test block
 * (a copy of a variable survives where a dead copy of an expression is
 * dropped), and the attach store sits in a `do { } while (0)`, which starts
 * a block at the store so the state read ahead of it is replaced too. The
 * variable then lives only in copies two to four and takes v0, and every
 * other register follows. `object` is declared last so the two spill homes
 * stay at 0x6C and 0x44. */
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
    void *object;

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
            object = hits[i];
            hit = object;
            if ((state->active == 0) ||
                ((state->normalX * hit->x) + (state->normalY * hit->y) +
                 (state->normalZ * hit->z) + state->distance < 0.0f)) {
                st = hit->state64;
                do { *(void **) ((u8 *) st + 0xC8) = arg0; } while (0);
            }
        }
    }
}
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
    trackAddTextureScroll(entry->textureIndex, x, y, updateRate);
}
#ifdef NON_MATCHING
/* 2026-10-07 (lane d-fx): 128 -> 48 at delta 0, frame 0xE0 exact: the
 * per-file -Wab,-r4300_mul (no two mul.s back to back, non-likely bc1f on
 * the radius test); the hit interpolation through delta locals with
 * deltaX/deltaZ reused for the horizontal offsets (the pressure leaves the
 * radius and hit y in memory, as shipped).
 * 2026-10-07 (lane g-4): 44 -> 10. Object x/y/z read in place (no x local:
 * the first block is the target's), plus three ordering levers measured on
 * the allocator records (proc 6):
 *  - `fraction = object->x;` (a dead store uopt removes) numbers the x
 *    expression ahead of the normals, so the y/z split pieces are grown
 *    into the second-distance block (10 registers left, not 8) and take
 *    f24/f26 with their spill stores after the previous-position loads;
 *  - `firstDistance = 0.0f;` in the block above numbers firstDistance
 *    ahead of x, so firstDistance takes f28 and x f30;
 *  - `i = 0;` before the call numbers i ahead of the cursor (s2/s3), and
 *    the second distance summed z, y, x numbers the previous position
 *    px, py, pz (f16/f18/f20).
 * 2026-10-08 (lane q-f): 10 -> 8. The call result goes into `list` (the
 * frame cell the unused x held) and the for-init copies it to the cursor
 * after `i = 0`, so ugen emits the index clear before the cursor copy, as
 * shipped (stream surgery: that emission order alone is worth the 2 words).
 * 2026-10-09 (lane s-2): 8 -> 6. The target state is a struct and the
 * previous position is read through members, summed x, y, z like the first
 * distance: a byte-offset cast is a heavier operand (L92), which had put the
 * position left of the normal and the sum left of the distance. The z, y, x
 * spelling that compensated goes, and so does the dead `i = 0;` (inert).
 * 2026-10-09 (lane y-1): 6 -> 3, every frame offset now the target's. The
 * spill slots go to cross-block temporaries in web-number order, and a
 * later temporary reuses the first slot whose owner it does not meet. So:
 *  - x is a declared local (the cell pad1 held), assigned before the rest,
 *    so it wins the coordinate colour tie (f30, whole) without making a
 *    slot request;
 *  - STAND-IN: the dead `fraction = y + z + previous position` numbers
 *    those five ahead of the plane values, so y and z take slots 0x8C and
 *    0x88 and the plane distance takes 0x78;
 *  - STAND-IN: the dead `fraction = plane->radius;` puts the radius load in
 *    the block before the varref cut, so the compare's radius is a
 *    temporary, and it reuses the distance's slot at 0x78;
 *  - STAND-IN: `secondDistance = 0.0f;` numbers secondDistance ahead of the
 *    previous position (their colour tie).
 * Left (3): x's load is emitted before the plane normal's (its own
 * statement), so as1 puts it in the beql slot. Stream surgery moving that
 * one load after the normal's gives 0. */
/* PROVENANCE: JFG's public character-plane control role supplies the idiom; Mickey's fields, globals, and action calls are authoritative below. */
void func_8001B798(SpranimB798Object *arg0, s32 arg1) {
    SpranimPlane *plane;
    SpranimB798Target **objects;
    SpranimB798Target *object;
    SpranimB798State *targetState;
    s32 i;
    f32 firstDistance;
    f32 secondDistance;
    f32 fraction;
    f32 hitX;
    f32 hitY;
    f32 hitZ;
    SpranimB798Target **list;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    s32 count;
    f32 x;
    s32 pad1;
    s32 pad2;

    plane = arg0->state64;
    list = (SpranimB798Target **) func_80005750(&count);
    for (i = 0, objects = list; i < count; i++, objects++) {
        object = *objects;
        targetState = object->state64;
        firstDistance = 0.0f;
        secondDistance = 0.0f;
        if ((targetState->flags & 1) && (targetState->flag0 != 0)) {
            continue;
        }
        x = object->x;
        fraction = object->y + object->z + targetState->previousX + targetState->previousY + targetState->previousZ;
        firstDistance = plane->distance +
            ((plane->normalX * x) + (plane->normalY * object->y) +
             (plane->normalZ * object->z));
        if (firstDistance < 0.0f) {
            do {
                secondDistance = plane->distance +
                    ((plane->normalX * targetState->previousX) +
                     (plane->normalY * targetState->previousY) +
                     (plane->normalZ * targetState->previousZ));
            } while (0);
            if (secondDistance >= 0.0f) {
                deltaX = x - targetState->previousX;
                deltaY = object->y - targetState->previousY;
                deltaZ = object->z - targetState->previousZ;
                fraction = secondDistance / (secondDistance - firstDistance);
                hitX = targetState->previousX + fraction * deltaX;
                hitY = targetState->previousY + fraction * deltaY;
                hitZ = targetState->previousZ + fraction * deltaZ;
                fraction = plane->radius;
                deltaX = hitX - arg0->x;
                deltaZ = hitZ - arg0->z;
                if (((deltaX * deltaX) + (deltaZ * deltaZ) <= plane->radius) &&
                    (arg0->y <= hitY) && (hitY <= plane->maxY)) {
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

/* PLATEAU-HANDOFF:func_8001B798:start
 * symbol: func_8001B798
 * score: 3/175 words
 * frame: 0xE0
 * relocations: 9
 * first-mismatch: +0x8C
 * summary: 3 at 0: the residual is one priority order (firstDistance, x above 15); declared x reaches it but emits x's load first
 * PLATEAU-HANDOFF:func_8001B798:end
 */
