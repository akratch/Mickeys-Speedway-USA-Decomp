#include "PR/ultratypes.h"

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

typedef struct Overlay79Object Overlay79Object;
typedef struct Overlay79MotionState Overlay79MotionState;

struct Overlay79MotionState {
    /* 0x00 */ s16 targetAngle;
    /* 0x02 */ s8 mode;
    /* 0x03 */ s8 active;
    /* 0x04 */ s16 event;
    /* 0x06 */ s16 step;
    /* 0x08 */ u32 collisionFlags;
    /* 0x0C */ s32 effectTimer;
    /* 0x10 */ f32 targetX;
    /* 0x14 */ f32 targetZ;
    /* 0x18 */ f32 acceleration;
    /* 0x1C */ f32 speed;
    /* 0x20 */ f32 heightOffset;
    /* 0x24 */ f32 homeX;
    /* 0x28 */ f32 homeY;
    /* 0x2C */ f32 homeZ;
    /* 0x30 */ f32 radiusSquared;
    /* 0x34 */ f32 travelRadius;
    /* 0x38 */ f32 previousHeight;
    /* 0x3C */ void *effect;
    /* 0x40 */ Overlay79Object *target;
};

struct Overlay79Object {
    /* 0x00 */ s16 angle;
    u8 pad02[6];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    u8 pad18[4];
    /* 0x1C */ f32 velocityX;
    /* 0x20 */ f32 velocityY;
    /* 0x24 */ f32 velocityZ;
    /* 0x28 */ f32 floorHeight;
    u8 pad2C[0xF];
    /* 0x3B */ s8 mode;
    /* 0x3C */ s32 field3C;
    u8 pad40[0x24];
    /* 0x64 */ Overlay79MotionState *state;
    u8 pad68[0x18];
    /* 0x80 */ u32 flags;
};

typedef struct Overlay79SpawnDesc {
    /* 0x00 */ s16 objectId;
    /* 0x02 */ u8 kind;
    /* 0x03 */ u8 flags;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 angle;
    /* 0x0C */ Overlay79Object *parent;
    /* 0x10 */ f32 scale;
    u8 pad14[4];
} Overlay79SpawnDesc;

/*
 * Overlay 79's initialized image (ROM 0x18CE480..0x18CE4E0, module offsets
 * +0x14E0..+0x1540).  The shipped relocation table splits it at +0x1500: the
 * first eight words are one runtime-indexed table (LOCAL records patch a base
 * of +0x14E0 with a zero addend and a scaled `object->mode`), and the sixteen
 * words after it are the constant pool the module's code loads with LOCAL
 * records whose base is +0x1500 and whose addends run +0x4..+0x2C.  Tier A for
 * the split and for the addends; the byte values are unchanged from the
 * previously adopted image, so the module's data bytes are identical.
 */
f32 gOverlay79ModeFactors[8] = {
    0.04f, 0.01f, 0.02f, 0.1f, 0.01f, 0.0f, 0.0f, 0.0f,
};

f32 gOverlay79Constants[16] = {
    0.05f,  -0.2f, 0.2f, 0.9f, -0.1f, 0.707f, 0.1f, 0.9f,
    0.46f,  0.9f,  -0.1f, -0.2f, 0.707f, 0.1f, 0.01f, 0.01f,
};

/*
 * The module reaches its own data through the runtime linker rather than
 * through a link-time address, so the adopted C spells each base as the raw
 * stored addend the shipped record carries (`docs/reloc-surface.md`).
 * `gOverlay79FlagsReloc` is the reserved-BSS selector 0xFFF entry at
 * +0x4D6E8 that `func_overlay_079_F0001290_18CE230` already names; ROM-table
 * index 1579 covers both uses, so this is the same object (Tier A).
 */
extern u8 gOverlay79FlagsReloc[];
extern f32 gOverlay79ModeFactorsReloc[];
extern f32 gOverlay79ConstantsReloc[];
extern u8 D_FA0[];

#define O79_ACTIVE_ACCELERATION   gOverlay79ConstantsReloc[1]
#define O79_INACTIVE_ACCELERATION gOverlay79ConstantsReloc[2]
#define O79_TURN_POWER            gOverlay79ConstantsReloc[3]
#define O79_FORWARD_ACCELERATION  gOverlay79ConstantsReloc[4]
#define O79_TARGET_DOT            gOverlay79ConstantsReloc[5]
#define O79_BRAKE_ACCELERATION    gOverlay79ConstantsReloc[6]
#define O79_APPROACH_POWER        gOverlay79ConstantsReloc[7]
#define O79_LAUNCH_HEIGHT         gOverlay79ConstantsReloc[8]
#define O79_TURN_IN_POWER         gOverlay79ConstantsReloc[9]
#define O79_GRAVITY_HALF          gOverlay79ConstantsReloc[10]
#define O79_GRAVITY               gOverlay79ConstantsReloc[11]

extern Overlay79Object *overlay79FindNearby(Overlay79Vector *position,
                                            f32 radiusSquared);
extern void func_8005AD64(Overlay79Object *object, s32 mode, s32 index,
                          f32 value);
extern f32 sqrtf(f32 value);
extern void func_800031E8(void *handle);
extern s32 mathRnd(s32 lower, s32 upper);
extern void func_80002FE0(u16 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern s32 Arctanf(f32 y, f32 x);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern void mathOneFloatRPY(Overlay79Object *object, Overlay79Vector *vector);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);
extern Overlay79Object *func_8000590C(Overlay79SpawnDesc *desc, s32 count);
extern void func_80008128(Overlay79Object *object, f32 x, f32 y, f32 z);
extern s32 func_8005ABA8(Overlay79Object *object, f32 factor, f32 updateRate);
extern void func_800031C0(void *handle, f32 x, f32 y, f32 z);
extern void partUpdateTriggers(Overlay79Object *object, s32 updateRate);
extern void trackMakePolylist(s32 mode, Overlay79Vector *start,
                              Overlay79Vector *end, f32 *height, void *unused,
                              s32 flags);
extern s32 func_80010900(Overlay79Vector *start, Overlay79Vector *end,
                         f32 height, Overlay79Object *object, void *callback);

/*
 * Extent closed, 882 candidate instructions against 882, frame 0xB8 exact,
 * 88 static relocations matching the module's shipped records by count, type
 * histogram (58 R_MIPS_26, 15 HI16, 15 LO16) and per-callee multiplicity.
 * 198 positional words still differ, raw and relocation-masked alike; the
 * integer temp ring is closed and 167 of the 198 are fp rows.
 *
 * There is no low-based-overlay rendering artifact here, and the count is
 * zero rather than small.  The project's comparators mask the union of both
 * sides' relocation fields per word, which equalizes splat's bare `lui`
 * literal against the C build's `%hi` relocated form, so raw and masked agree
 * at 288.  The trap is on the other side: reading the ROM's own stored word
 * out of splat's comment field and diffing that against a compiled object
 * reports two phantom rows here -- the link-time-resolved internal `jal` at
 * +0x80 and the `D_FA0` addend at +0xC8C, both of which the assembled target
 * carries as zero-field relocations.  Any hand-rolled word scorer must mask
 * the same union and rebase objdump's section coordinates onto the function
 * base, or it invents residual that is not there.
 *
 * The residual is two ring rotations plus structure, not 288 decisions.
 * Substituting a single step of either scratch ring into the candidate row by
 * row accounts for 170 of the 288:
 *
 *   106 rows  one step of the fp scratch ring (f4 -> f18 -> f8 -> f10 -> f6),
 *             a single contiguous phase from +0x4B4 to +0xB80
 *    46 rows  one step of the integer temp ring (t6..t9, t0..t5), from +0x58
 *             to +0xD40
 *     7 rows  the integer ring one step back, +0x3F8..+0x930
 *     9 rows  two and three fp steps in the epilogue, +0xD0C..+0xD68
 *     2 rows  mixed single steps
 *
 * Each phase is one step and each has one onset row, so each is a count
 * question: one coloured web the candidate does not have.  The integer phase
 * opens at +0x58, where rows 0..21 are identical and the target then takes t3
 * against the candidate's t2; the same absent temporary is why the race-state
 * flag is homed at 0x40 where the target puts it at 0x3C, one slot later in
 * spilltemp order, with the rest of the 34-slot stack-home census exact.  The
 * fp phase opens at +0x4B4, where the target colours `dot` into the fp pool
 * (`add.s $f0`) and gives `dx` f12, while the candidate spends an fp scratch
 * register on `dot` (`add.s $f4`) and gives `dx` f0; one extra pool web in the
 * target displaces the ring by one for the next 106 rows.
 *
 * The 22 head rows that are not phase are the flag home (+0x7C, +0x88), the
 * pool-coloured pointer carrier (+0x3AC, +0x3B0, below), the comparison
 * operand order at +0x68C -- law L67 makes that a readout of the carrier, not
 * a lever -- and 16 scattered fp-pool colours.  The remaining 96 are the tail.
 *
 * What closed 7 words this pass.  Naming the two `Arctanf` arguments before
 * the call -- `dx = object->x - state->targetX; dz = object->z -
 * state->targetZ;` at both the mode-2 and mode-3 retarget sites -- makes ugen
 * evaluate them in source order.  Written inline, the candidate evaluated the
 * second argument first and put the first in the delay slot; the target does
 * the reverse.  It also drops the alignment gap count from 46 to 18 and moves
 * the fp-pool lane's first divergence from slot 1 to slot 19.
 *
 * The tail is one unfilled delay slot.  Ordering the four integration
 * statements D(istance), Y(=dy), S(peed +=), V(elocityY +=) as D,S,Y,V puts
 * the `object->velocityY` load under a single web for both the `dy` term and
 * the `velocityY +=` statement -- with the speed store between them the alias
 * kills the CSE and the `swc1 dy,0x74(sp)` disappears -- and that restores the
 * store, the nop pattern at +0xB94/+0xB9C/+0xBA4/+0xBD8/+0xBE0 and the
 * +0xC74..+0xCF4 alignment, worth 24 shape rows.  It costs one instruction:
 * `dy` is single-use, so uopt sinks it into the call statement, its `mfc1`
 * into an argument register must then be scheduled last, as1 cannot sink an
 * instruction that defines a `.livereg` register into a delay slot, and the
 * `func_80008128` slot is padded.  All 24 orderings of those four statements
 * were compiled: exactly two hold 882 instructions (D,Y,S,V and S,Y,D,V) and
 * both drop the store; the structural win is only available at 883.  D,Y,S,V
 * is kept.
 *
 * Ruled out with measurement, in this lane and the one before it: the flag
 * lattice (`-O1`/`-O2`/`-O3` x `-mips1`/`-mips2` x `-g3` x with/without
 * `-Wab,-r4300_mul`); all six orderings of the three displacement components
 * at `func_80008128`, every inlined spelling of them, and every alternative
 * carrier for `dy` -- uopt canonicalizes all of them to one IR, confirmed
 * against the ugen listing, which shows `dx` at its own line, `dz` at its own
 * line and `dy` sunk into the call; every carrier for the first `dot`
 * (`distance`, `factor`, `range`, `dy`), all four byte-identical, so the fp
 * phase is not reachable by renaming -- though commuting that sum costs 8
 * words, which is L92 biting; five respellings of the race-state condition
 * (`== 0` for `!`, `> 2` for `>= 3`, the signed-shift form, nested ifs, and
 * initialise-then-override), of which the first three are byte-identical and
 * the last two cost 500+ words; and the declaration census as an explanation
 * for the 0x3C/0x40 slot -- the local block is quantized in 8-byte steps, so
 * adding any local grows the frame to 0xC0 and shifts every home, re-measured
 * here with an unused `s32`.  A 25-minute randomizing permuter run found no
 * improvement on the pre-edit base.
 *
 * What closed 90 words this pass, and why none of it was reachable one edit
 * at a time.  The integer ring's onset and the pool-colour lever above are
 * one question, and the composition is the whole result:
 *
 *   1. The race-state bit test spelled `((flags << 0xD) & 0x80000000U) == 0`
 *      in place of `!(flags & 0x40000)` burns exactly one ugen ring temp
 *      between the `bne` at +0x54 and the `sll` at +0x58 and emits nothing:
 *      both forms compile to the same two words, but the target's `sll t3`
 *      against the candidate's `sll t2` becomes exact and the integer ring
 *      stays aligned for the next 210 rows.  Alone it measures 328, because
 *      the ring is then one step AHEAD from +0x3A4 onward -- the 7 rows the
 *      decomposition above records as "one step back".
 *   2. `spawned = (Overlay79Object *)state->target->state; *(s32 *)spawned = 1;`
 *      is the lever the previous lane recorded and declined: it spends a
 *      pool colour where the candidate spent a ring temp, which is exactly
 *      the one step the ring needs giving back at +0x3A4.  Alone it measures
 *      374.  Together with (1) the pair measures 229.
 *   3. `dx = state->targetX; dx -= object->x;` at the mode-0 dot product, in
 *      place of the single subtraction, is the recorded copy-back-into-the-
 *      carrier lever; it closes 31 more, to 198.  It is site-specific: the
 *      same rewrite measured at seven other dx/dz sites is flat at best, and
 *      at the mode-0 retarget site it drops four instructions outright.
 *
 * Each of (1) and (2) is a regression alone and neither is reachable by a
 * single-edit accept rule.  `spawned` is safe: its only other definition
 * dominates its only other uses, in the mutually exclusive `else` arm.
 *
 * Remaining, and measured flat here: eleven further spellings of the
 * race-state clauses (`>= 0xF`, `> 0xE`, `^ 0xF) == 0`, `- 0xF) == 0`,
 * `!(... != 0xF)`, `0xFU`, the `>> 23 & 0xF` form, `(... < 0xF) == 0`,
 * `(... != 0xF) == 0`, `> 2` for `>= 3`, and the `& 7` form) -- the ones that
 * do advance the ring emit a real `sltiu` into it and cost 40+; six
 * respellings of the first `dot` including the split, the copy-back and a
 * second carrier; `do { } while (0)` and `if (1) { }` regions around the dot
 * statement and around its two operand statements, all byte-identical to
 * nothing; and `(f32)` on `update`.
 *
 * One recorded hypothesis is falsified by this pass and should not be
 * re-run: the race-state flag's home at 0x40 against the target's 0x3C was
 * attributed to the same absent temporary as the integer ring onset.  The
 * ring onset is closed and the home still differs, so it is a separate
 * spilltemp-order question, and +0x7C/+0x88 are now the first two differing
 * words.  The tail is unchanged and still not buyable: all 24 orderings of
 * D, Y, S and V were re-measured on this base and only D,Y,S,V (198) and
 * S,Y,D,V (203) hold 882 instructions, every other ordering adding four to
 * twelve.
 *
 * The residual is now the fp side of the same question: the fp scratch ring
 * is one web short from +0x368, where the target gives `dy` the pool colour
 * f2 and the candidate spends f16 on it, so `update` then reads f12 in the
 * tail where the target reads f16.  Measured flat against that: four other
 * carriers for that `dy`, three orderings and two associations of the
 * distance sum it feeds, and four spellings of the two tail integration
 * products.  The four fp families plus the +0xB80 tail are the 198.
 */
#ifdef NON_MATCHING
void func_overlay_079_F0000134_18CD0D4(Overlay79Object *object,
                                       s32 updateRate) {
    Overlay79MotionState *state;
    Overlay79Object *nearby;
    Overlay79Object *spawned;
    Overlay79SpawnDesc desc;
    Overlay79Vector start;
    Overlay79Vector end;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 dot;
    f32 update;
    f32 range;
    f32 factor;
    Overlay79Vector forward;
    s8 mode;
    s32 delta;
    s32 angle;
    s32 raceActive;

    state = object->state;
    update = updateRate;
    if (((*(u16 *)&gOverlay79FlagsReloc[0xE] & 0x1C0) >> 6) >= 3 &&
        (((*(u32 *)&gOverlay79FlagsReloc[0] << 5) >> 28) == 0xF) &&
        (((*(u32 *)&gOverlay79FlagsReloc[0] << 0xD) & 0x80000000U) == 0)) {
        raceActive = 1;
    } else {
        raceActive = 0;
    }

    nearby = overlay79FindNearby((Overlay79Vector *)&object->x, 22500.0f);
    if (nearby != NULL) {
        state->active = 1;
    }

    if (state->active != 0) {
        if (state->collisionFlags & 2) {
            if (object->mode != 0) {
                func_8005AD64(object, 0, -1, 0.0f);
            }
        } else if (((state->collisionFlags & 2) == 0) &&
                   (object->mode != 3)) {
            func_8005AD64(object, 3, -1, 0.0f);
        }

        if (nearby != NULL) {
            if (state->speed > -25.0f) {
                state->acceleration = O79_ACTIVE_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
            }
            if (nearby != NULL) {
                dx = nearby->x - object->x;
                dz = nearby->z - object->z;
                if ((sqrtf((dx * dx) + (dz * dz)) < 70.0f) &&
                    (state->collisionFlags & 2)) {
                    object->velocityY = 5.0f;
                    if (state->effect != NULL) {
                        func_800031E8(state->effect);
                    }
                    func_80002FE0(mathRnd(0x21F, 0x226), object->x, object->y,
                                  object->z, 4, &state->effect);
                    state->effectTimer = mathRnd(0x78, 0xF0);
                }
            }
        } else if (state->speed < 0.0f) {
            state->acceleration = O79_INACTIVE_ACCELERATION;
        } else {
            state->active = 0;
            state->step = 0;
            state->mode = 2;
            state->acceleration = 0.0f;
            state->speed = 0.0f;
        }

        dx = object->x - state->homeX;
        dz = object->z - state->homeZ;
        if ((state->radiusSquared <= ((dx * dx) + (dz * dz))) ||
            (nearby == NULL)) {
            state->targetAngle = Arctanf(dx, dz);
        } else {
            state->targetAngle = Arctanf(nearby->x - object->x,
                                         nearby->z - object->z);
        }
        object->angle = dAngle(object->angle, state->targetAngle,
                               1.0f - Powerf(O79_TURN_POWER, updateRate));
        delta = (object->angle - state->targetAngle) & 0xFFFF;
        if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
            delta = -((state->targetAngle - object->angle) & 0xFFFF);
        }
        if ((delta >= -99) && (delta < 100)) {
            object->angle = state->targetAngle;
        }
        if (state->target != NULL) {
            dx = state->target->x - object->x;
            dy = state->target->y - object->y;
            dz = state->target->z - object->z;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 30.0f) {
                spawned = (Overlay79Object *)state->target->state;
                *(s32 *)spawned = 1;
            }
        }
        if (state->effectTimer != 0) {
            state->effectTimer -= updateRate;
            if (state->effectTimer < 0) {
                state->effectTimer = 0;
            }
        }
        if ((state->effect == NULL) && (state->effectTimer == 0)) {
            func_80002FE0(mathRnd(0x227, 0x229), object->x, object->y,
                          object->z, 4, &state->effect);
            state->effectTimer = mathRnd(0x78, 0xF0);
        }
    } else {
        mode = state->mode;
        if (mode == 0) {
            if (state->speed > -1.0f) {
                state->acceleration = O79_FORWARD_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
            }
            forward.x = 0.0f;
            forward.y = 0.0f;
            forward.z = -1.0f;
            mathOneFloatRPY(object, &forward);
            dx = state->targetX;
            dx -= object->x;
            dz = state->targetZ - object->z;
            dot = (forward.z * dz) + (dx * forward.x);
            if ((dot < 0.0f) || (state->collisionFlags & 4)) {
                state->mode = 1;
            } else if (state->target != NULL) {
                dx = object->x - state->target->x;
                dz = object->z - state->target->z;
                distance = sqrtf((dx * dx) + (dz * dz));
                forward.x = 0.0f;
                forward.y = 0.0f;
                forward.z = -1.0f;
                mathOneFloatRPY(object, &forward);
                dx = state->target->x - object->x;
                dz = state->target->z - object->z;
                dot = (forward.z * dz) + (dx * forward.x);
                if ((distance < 50.0f) && (O79_TARGET_DOT < dot)) {
                    state->mode = 1;
                }
            }
            if (state->effectTimer != 0) {
                state->effectTimer -= updateRate;
                if (state->effectTimer < 0) {
                    state->effectTimer = 0;
                }
            }
            if ((state->effect == NULL) && (state->effectTimer == 0)) {
                func_80002FE0(mathRnd(0x21B, 0x21E), object->x, object->y,
                              object->z, 4, &state->effect);
                state->effectTimer = mathRnd(0x78, 0xF0);
            }
        } else if (mode == 1) {
            if (state->speed < 0.0f) {
                state->acceleration = O79_BRAKE_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
                state->speed = 0.0f;
                state->mode = (state->target == NULL && raceActive != 0)
                                  ? 3
                                  : 2;
            }
        } else if (mode == 2) {
            if (state->step == 0) {
                func_8005AD64(object, 1, -1, 0.0f);
                state->step++;
            } else if ((state->step == 1) && (state->event != 0)) {
                if (state->effect == NULL) {
                    func_80002FE0(mathRnd(0x218, 0x21A), object->x, object->y,
                                  object->z, 4, &state->effect);
                }
                func_8005AD64(object, 2, -1, 0.0f);
                state->step++;
            } else if ((state->step == 2) && (state->event != 0)) {
                func_8005AD64(object, 1, -1, 0.0f);
                state->step++;
            } else if ((state->step == 3) && (state->event != 0)) {
                func_8005AD64(object, 0, -1, 0.0f);
                state->step = 0;
                angle = mathRnd(-0x7FFF, 0x8000);
                range = mathRnd(0, (s32)state->travelRadius);
                state->targetX = state->homeX - (func_8002A8C0(angle) * range);
                state->targetZ = state->homeZ - (func_8002A8BC(angle) * range);
                dx = object->x - state->targetX;
                dz = object->z - state->targetZ;
                state->targetAngle = Arctanf(dx, dz);
                state->mode = 4;
            }
        } else if (mode == 3) {
            if (state->step == 0) {
                angle = mathRnd(-0x7FFF, 0x8000);
                range = mathRnd(0, (s32)state->travelRadius);
                state->targetX = state->homeX - (func_8002A8C0(angle) * range);
                state->targetZ = state->homeZ - (func_8002A8BC(angle) * range);
                dx = object->x - state->targetX;
                dz = object->z - state->targetZ;
                state->targetAngle = Arctanf(dx, dz);
                state->step = 1;
            } else if (state->step == 1) {
                object->angle = dAngle(object->angle, state->targetAngle,
                                       1.0f - Powerf(O79_APPROACH_POWER,
                                                     updateRate));
                delta = (object->angle - state->targetAngle) & 0xFFFF;
                if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
                    delta = -((state->targetAngle - object->angle) & 0xFFFF);
                }
                if ((delta >= -99) && (delta < 100)) {
                    object->angle = state->targetAngle;
                    state->step = 2;
                }
            } else if (state->step == 2) {
                func_8005AD64(object, 4, -1, 0.0f);
                if (state->effect == NULL) {
                    func_80002FE0(mathRnd(0x270, 0x272), object->x, object->y,
                                  object->z, 4, &state->effect);
                }
                state->step = 3;
            } else if ((state->step == 3) &&
                       (state->previousHeight < O79_LAUNCH_HEIGHT) &&
                       (O79_LAUNCH_HEIGHT <= object->floorHeight)) {
                desc.objectId = 0x144;
                desc.kind = 0x14;
                desc.flags = 0;
                desc.x = (func_8002A8C0(object->angle) * 18.0f) + object->x;
                desc.y = object->y;
                desc.z = (func_8002A8BC(object->angle) * 18.0f) + object->z;
                desc.parent = object;
                desc.scale = object->scale;
                spawned = func_8000590C(&desc, 1);
                if (spawned != NULL) {
                    spawned->field3C = 0;
                    state->target = spawned;
                }
                func_80002FE0(0x273, object->x, object->y, object->z, 4, NULL);
                state->step = 4;
            } else if ((state->step == 4) && (state->event != 0)) {
                func_8005AD64(object, 0, -1, 0.0f);
                state->step = 0;
                state->mode = 0;
            }
        } else if (mode == 4) {
            object->angle = dAngle(object->angle, state->targetAngle,
                                   1.0f - Powerf(O79_TURN_IN_POWER,
                                                 updateRate));
            delta = (object->angle - state->targetAngle) & 0xFFFF;
            if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
                delta = -((state->targetAngle - object->angle) & 0xFFFF);
            }
            if ((delta >= -99) && (delta < 100)) {
                object->angle = state->targetAngle;
                state->mode = 0;
            }
        }
    }

    distance = (state->speed * update) +
               (0.5f * state->acceleration * update * update);
    dy = (object->velocityY * update) +
         (O79_GRAVITY_HALF * update * update);
    state->speed += state->acceleration * update;
    object->velocityY += O79_GRAVITY * update;
    dx = func_8002A8C0(object->angle) * distance;
    dz = func_8002A8BC(object->angle) * distance;

    start.x = object->x;
    start.y = object->y + state->heightOffset;
    start.z = object->z;
    end.x = start.x + dx;
    end.y = start.y + dy;
    end.z = start.z + dz;
    state->collisionFlags = 0;
    trackMakePolylist(1, &start, &end, &state->heightOffset, NULL, 1);
    if (func_80010900(&start, &end, state->heightOffset, object, D_FA0) != 0) {
        object->velocityY = 0.0f;
    }
    dx = end.x - start.x;
    dy = end.y - start.y;
    dz = end.z - start.z;
    func_80008128(object, dx, dy, dz);
    object->velocityX = dx / update;
    object->velocityZ = dz / update;
    if (object->mode == 0) {
        factor = gOverlay79ModeFactorsReloc[object->mode] * -state->speed;
    } else {
        factor = gOverlay79ModeFactorsReloc[object->mode];
    }
    state->previousHeight = object->floorHeight;
    state->event = func_8005ABA8(object, factor, update);
    if (state->effect != NULL) {
        func_800031C0(state->effect, object->x, object->y, object->z);
    }
    if (object->mode == 3) {
        object->flags |= 1;
        partUpdateTriggers(object, updateRate);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0000134_18CD0D4/func_overlay_079_F0000134_18CD0D4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_079_F0000134_18CD0D4:start
 * symbol: func_overlay_079_F0000134_18CD0D4
 * score: 198/882 words
 * frame: 0xB8
 * relocations: 88
 * first-mismatch: +0x7C
 * summary: L145 declared FP carriers do not add the prefix ring draw. Missing draw is CSE of -1.0f at forward.z. Next: delta-0 CSE-break so z draws onto f4.
 * PLATEAU-HANDOFF:func_overlay_079_F0000134_18CD0D4:end
 */
