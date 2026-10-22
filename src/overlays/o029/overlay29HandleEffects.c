#include "PR/ultratypes.h"

typedef struct Vec3f { f32 x, y, z; } Vec3f;

typedef struct Overlay29Record {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[2];
    f32 scalar;
    Vec3f position;
    Vec3f vector;
    s16 random0;
    s16 random1;
    s16 random2;
    s16 value;
} Overlay29Record;

typedef struct Overlay29State {
    u8 pad00[0xB];
    u8 active;
    u8 pad0C[0x10];
    Vec3f direction;
    Overlay29Record records[4];
} Overlay29State;

typedef struct Overlay29Resource {
    u8 pad00[6];
    u16 flags;
} Overlay29Resource;

typedef struct Overlay29Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    s16 flags06;
    f32 scalar;
    Vec3f position;
    u8 pad18[0x30];
    Overlay29Resource *resource;
    u8 pad4C[0x18];
    Overlay29State *state;
    u8 pad68[0x18];
    u32 flags80;
} Overlay29Object;

extern void func_80006EA0(void);
extern s16 overlay29AngleReloc(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern void mathOneFloatPY(s16 *angles, Vec3f *vector);
extern s16 mathRnd(s32 low, s32 high);
extern void partUpdateTriggers(Overlay29Object *object, s32 mode);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z, s32 type, s32 arg);

#define INITIALIZE_RECORD(rec_) \
    do { \
        record->position.x = object->position.x; \
        record->position.y = object->position.y; \
        record->position.z = object->position.z; \
        record->angle0 = object->angle0; \
        record->angle1 = object->angle1; \
        record->angle2 = object->angle2; \
        record->scalar = object->scalar; \
        record->random0 = mathRnd(-0x500, 0x500);       \
        record->random1 = mathRnd(-0x500, 0x500);       \
        *(volatile s16 *) &record->random2 =                              \
            mathRnd(-0x500, 0x500);                     \
        *(volatile s16 *) &record->value = 0xFF;                           \
    } while (0)

/* The second block's angle setup belongs to the first block's initialiser
 * line, not to its own.  Emitting it there keeps `record` -- the first
 * block's pool web, colour v1 -- live across the shared
 * `verticalAngle + 0x2000` CSE, and the CSE moves off v0 onto a2, which is
 * what the target does.  The macro must spell the body out rather than
 * wrap INITIALIZE_RECORD: a nested expansion resynchronises acpp's line
 * counter and puts the two statements back on their own line (17 words). */
#define INITIALIZE_RECORD_THEN(rec_, next_)                                \
    do {                                                                   \
        record->position.x = object->position.x;                           \
        record->position.y = object->position.y;                           \
        record->position.z = object->position.z;                           \
        record->angle0 = object->angle0;                                   \
        record->angle1 = object->angle1;                                   \
        record->angle2 = object->angle2;                                   \
        record->scalar = object->scalar;                                   \
        record->random0 = mathRnd(-0x500, 0x500);        \
        record->random1 = mathRnd(-0x500, 0x500);        \
        *(volatile s16 *) &record->random2 =                               \
            mathRnd(-0x500, 0x500);                      \
        *(volatile s16 *) &record->value = 0xFF;                           \
        next_;                                                             \
    } while (0)

/* Exact size (1028 bytes / 257 words), exact 0x48 frame, all 22 relocation
 * offsets and types.  253 of 257 words agree and the four that do not are one
 * fact: the CSE web for `verticalAngle + 0x2000` -- defined in the second
 * block, spilled to the compiler temp at sp+0x2C and reloaded by the third and
 * fourth -- takes `v0` here and `a2` in the target.  The four words are
 * +0x13C, +0x148, +0x150 and +0x164, which is exactly that web's load, add,
 * store and spill; its temp slot, its schedule, its spill point and every
 * other register in the function agree.  ugen emits the register itself
 * (`addu $2, $2, 8192` in its listing), so this is ugen's allocation, not a
 * scheduling artefact of as1.
 *
 * The declaration list is closed on both sides.  A sixth local costs 22 words
 * at best in any of six positions and any of seven types -- the frame law is
 * frame = align8(below + S) with below = 0x38 and S = 0x10, so 0x48 admits
 * exactly these five.  The obvious composition -- drop `state` to make room,
 * then name the shared value -- does not work either: the named carrier is
 * free, but spelling `object->state` at its six uses is nine words SHORTER
 * (248/257), so `state` is load-bearing as a local and the list cannot be
 * traded.  That closes the L88 "name the value to add a pool colour" route.
 *
 * The double mask in the first block is load-bearing: a single `& 0xFFFFU`, a
 * `(u16)` cast or no mask at all is 72 words, first difference +0x30.  It
 * costs no instruction (ugen emits a bare `addu $t3, $2, 12288`), so it is a
 * pure web-structure edit.  Both `volatile` casts in INITIALIZE_RECORD are
 * load-bearing at eight words each.
 *
 * What is exhausted, all flat at four words.  Earlier lanes: statement order
 * and record placement across all four blocks (3,888); physical line grouping
 * within and across blocks (1,203); mask spelling per block (1,110); macro
 * body order (576); the record-pointer spelling lattice (256); local and
 * prototype types (163); inner-block scope placement and declaration
 * permutation (136); `register` hints and a sixth local (50); naming the
 * shared value (48); signature return type and extra parameters (12); the
 * driver flag lattice (13 -- `-Wab,-r4300_mul` confirmed).  Added here: all
 * 4,096 identity-operation forms of the three `verticalAngle + 0x2000` sites
 * (`^ 0`, `| 0`, `+ 0`, `- 0`, `* 1`, `-(-x)`, `(s16)`, `(u16)`, single and
 * double mask, reversed operands), of which 1,868 keep the size and every one
 * scores four; all 721 statement orders of block 2; the `angles` array
 * respelled as a two-field struct with either field order and three argument
 * spellings; a 96-cell prototype lattice over the four relocated callees'
 * return and parameter types; and per-phase optimisation levels through
 * `ido-phases.py` (uopt/ugen/as1 at -O3, uopt at -O1).
 *
 * The one genuinely new fact is a reachability bound.  A 4,000-point
 * randomised cross of macro body order, both `volatile` casts, and the mask
 * spelling of all five angle expressions moves this carrier to `v0` (2,745
 * cells), `v1` (603), `t6` (250), `t7` (177), `t5` (107), `t8` (82) and `t9`
 * (29) -- and never once to `a0`-`a3`, at any score.  The 96-cell prototype
 * lattice and the 721 statement orders are also uniformly `v0`.  So the
 * target's `a2` is not in this candidate's reachable allocation set: the next
 * lever has to change the allocation regime, not the spelling or the
 * schedule.  Note the permuter is not usable here -- its scratch scores this
 * body 135 against a measured 4. */
void func_overlay_029_F00010C4_187E374(Overlay29Object *object, s32 mode) {
    Overlay29Record *record;
    Overlay29State *state;
    s16 angles[2];
    s16 baseAngle;
    s16 verticalAngle;

    state = object->state;
    if ((mode & 1) != 0) {
        func_80006EA0();
    } else if ((mode & 2) != 0) {
        baseAngle = overlay29AngleReloc(state->direction.x, state->direction.z);
        verticalAngle = overlay29AngleReloc(
            sqrtf((state->direction.z * state->direction.z) +
                               (state->direction.x * state->direction.x)),
            state->direction.y);

        record = &state->records[0];
        angles[0] = baseAngle;
        angles[1] = ((verticalAngle + 0x3000) & 0xFFFFU) & 0xFFFFU;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD_THEN(record, angles[1] = verticalAngle + 0x2000;
                                       angles[0] = baseAngle);

        record = &state->records[1];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);

        record = &state->records[2];
        angles[0] = baseAngle - 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);

        record = &state->records[3];
        angles[0] = baseAngle + 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);
        state->active = 1;
    }

    if ((mode & 4) != 0) {
        object->flags80 |= 2;
        partUpdateTriggers(object, 1);
    }
    if ((mode & 8) != 0) {
        amSndPlayXYZ(0x27A, object->position.x, object->position.y,
                           object->position.z, 4, 0);
    }
    object->resource->flags &= ~1;
    object->flags06 |= 0x400;
}
