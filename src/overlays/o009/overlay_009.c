#include "overlays/overlay_009.h"
#include "tools/m2c/m2c_macros.h"

#undef NULL
#define NULL 0

/* The TU's literal pool starts with func_overlay_009_F0000000_1866678's
 * three thresholds at +0x0..+0xC; the later functions' literals follow. */

/*
 * Overlay 9, ADR 0006 consolidation. Functions remain in retail ROM order.
 * The module uses the R4300 multiply-hazard schedule; applying that flag to
 * the intervening empty overlay9Ignore function does not change its bytes.
 */

extern void func_overlay_009_F00010A4_186771C(void *object, void *state,
                                               f32 steps);

/* Tier D: field widths and offsets from this caller's loads and stores. */
typedef struct O9Entry {
    void *data;
    u8 pad04[4];
    s16 count;
    s16 index;
    void *items[1];
} O9Entry;

typedef struct O9Model {
    u8 pad00[0x1E];
    s8 flags[1];
} O9Model;

typedef struct O9Params {
    f32 unk00;
    f32 unk04;
    f32 unk08;
    u8 pad0C[0x10];
    f32 unk1C;
    s16 angle20;
    s16 angle22;
    u8 pad24[6];
    s16 angle2A;
    f32 unk2C;
} O9Params;

/* The parameter block at .data +0x2D0 is one object whose address the four
 * helpers receive. as1 shares one high half between the stores to angle22
 * and angle2A only for a symbol the TU defines, so the TU lays the block
 * out at its recorded offset. The bytes belong to the retained overlay
 * image: these zero initializers only fix the layout, and the object's
 * .data is dropped at POSTPROCESS with this function's records rebound to a
 * zero-valued base (mk/overlays.mk). The other functions keep the D_ names. */
static u8 sOverlay9Data000[0x2D0] = { 0 };
static O9Params sOverlay9Params = { 0 };

/* PROVENANCE: Mickey-derived from the assigned overlay assembly range; no donor body was imported.
 *
 * Matched 2026-10-02, 120 words to 0. The parameter block is reached through
 * a pointer local, so every field access is an indirect one with its own
 * high half: stores through the packet cursor then force the angle reloads,
 * the angle stores force the cursor reloads, and no address web forms
 * (checklist item 10). The mode test is a two-case switch, which numbers
 * the selector ahead of the cursor's address and gives them v1 and t0. The
 * step conversion is written at both calls, the model flag and the entry's
 * item are array subscripts, and the locals are declared in home order. */
void func_overlay_009_F0000000_1866678(void *object, s32 steps) {
    void *state;
    O9Params *params = &sOverlay9Params;
    f32 vector[3];
    s16 angles[3];
    void *handle;
    void *entryData;
    void *savedEntry;
    f32 current;
    f32 target;
    f32 savedY;
    f32 rate;
    s32 remaining;

    state = M2C_FIELD(object, void **, 0x64);
    gOverlay9PacketCursor = (s16 *)((u8 *)state + 0x1B8);
    M2C_FIELD(object, s32 *, 0x80) = 0;
    savedEntry = *M2C_FIELD(object, void ***, 0x68);
    ext_o0_1ee14(state, M2C_FIELD(state, s8 *, 0));
    G_rt_458c4 = 0.1f;

    if (M2C_FIELD(state, f32 *, 4) < -30.0f) {
        M2C_FIELD(state, f32 *, 4) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 4) > 30.0f) {
        M2C_FIELD(state, f32 *, 4) = 30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) < -30.0f) {
        M2C_FIELD(state, f32 *, 8) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) > 30.0f) {
        M2C_FIELD(state, f32 *, 8) = 30.0f;
    }

    ext_o0_1d4c0(object, state);
    angles[0] = -M2C_FIELD(state, s16 *, 0xF0);
    angles[1] = -M2C_FIELD(object, s16 *, 2);
    angles[2] = -M2C_FIELD(object, s16 *, 4);
    vector[2] = 0.0f;
    vector[0] = 0.0f;
    vector[1] = -1.0f;
    ext_o0_29adc(angles, vector);
    M2C_FIELD(state, f32 *, 0x60) = vector[0];
    M2C_FIELD(state, f32 *, 0x64) = vector[1];
    M2C_FIELD(state, f32 *, 0x5C) = vector[2];

    if ((ext_o0_1312c(M2C_FIELD(object, f32 *, 0xC),
                      M2C_FIELD(object, f32 *, 0x14),
                      (u8 *)state + 0x68, 0x10000, 0) & 0x10000) &&
        ((M2C_FIELD(object, f32 *, 0x10) - 16.0f) <
         M2C_FIELD(state, f32 *, 0x68))) {
        M2C_FIELD(state, f32 *, 0x6C) =
            M2C_FIELD(state, f32 *, 0x68) -
            (M2C_FIELD(object, f32 *, 0x10) - 16.0f);
    } else {
        M2C_FIELD(state, f32 *, 0x6C) = 0.0f;
    }

    switch (M2C_FIELD(state, u8 *, 0x16C)) {
        case 0:
        case 1:
            func_overlay_009_F0000744_1866DBC(object, state, params, steps);
            break;
    }
    func_overlay_009_F0000CE4_186735C(object, state, params, (f32)steps);
    func_overlay_009_F00010A4_186771C(object, state, (f32)steps);
    func_overlay_009_F0000F6C_18675E4(object, params, steps);
    func_overlay_009_F0000540_1866BB8(object, state, params, steps);

    params->angle20 = (s16)(params->angle20 +
        (((s32)(3072.0f * params->unk1C) + 0x400) * steps));
    *gOverlay9PacketCursor++ = 0x22;
    *gOverlay9PacketCursor++ = params->angle20;
    params->angle22 = (s16)(params->angle22 +
        (((s32)(3072.0f * (params->unk08 - 1.0f)) + 0x400) * steps));
    *gOverlay9PacketCursor++ = 0x24;
    *gOverlay9PacketCursor++ = params->angle22;
    params->angle2A = (s16)(params->angle2A + (steps << 8));
    *gOverlay9PacketCursor++ = 0x2000;

    if ((M2C_FIELD(object, O9Model **, 0x40)
             ->flags[M2C_FIELD(object, u8 *, 0x93)] == 0) &&
        (savedEntry != NULL) && (M2C_FIELD(savedEntry, s16 *, 8) != 0)) {
        entryData = M2C_FIELD(savedEntry, void **, 0);
        savedY = M2C_FIELD(object, f32 *, 0x10);
        current = M2C_FIELD(state, f32 *, 4);
        if ((current < -2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) < -0x14) ||
            (current > 2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) >= 0x15)) {
            target = 0.0f;
            rate = 0.03f;
        } else {
            target = 4.0f;
            rate = 0.0125f;
        }
        remaining = steps - 1;
        if (steps != 0) {
            current = params->unk2C;
            do {
                current += (target - current) * rate;
            } while (remaining--);
            params->unk2C = current;
        }
        M2C_FIELD(object, f32 *, 0x10) +=
            params->unk2C * ext_o0_2a470(params->angle2A);
        ext_o0_5aac4(savedEntry, entryData, object);
        ext_o0_19668(object, savedEntry, M2C_FIELD(object, void **, 0x50),
                      ((O9Entry *)savedEntry)->items[((O9Entry *)savedEntry)->index]);
        M2C_FIELD(savedEntry, s16 *, 8) = 0;
        M2C_FIELD(object, f32 *, 0x10) = savedY;
    }

    M2C_FIELD(state, s8 *, 0x186) = 0;
    ext_o0_1d510(object, state, NULL, NULL, steps);
    if (M2C_FIELD(state, u8 *, 0x349) != 0) {
        if (M2C_FIELD(state, u8 *, 0x16C) == 1) {
            handle = M2C_FIELD(state, void **, 0xB8);
            M2C_FIELD(state, u8 *, 0x16C) = 0;
            M2C_FIELD(state, s8 *, 0x16E) = 8;
            if (handle != NULL) {
                ext_o0_2d98(handle);
            }
            amSndPlayXYZ(6, M2C_FIELD(object, f32 *, 0xC),
                        M2C_FIELD(object, f32 *, 0x10),
                        M2C_FIELD(object, f32 *, 0x14), 4,
                        (void **)((u8 *)state + 0xB8));
        } else {
            if (M2C_FIELD(state, s8 *, 0x16E) > 0) {
                M2C_FIELD(state, s8 *, 0x16E) =
                    M2C_FIELD(state, s8 *, 0x16E) - steps;
            }
        }
    } else {
        M2C_FIELD(state, s8 *, 0x16E) = 0;
    }
    ext_o0_3e99c(object, steps);
}

/* PROVENANCE: Mickey-derived from the assigned overlay assembly range; no donor body was imported.
 *
 * Matched 2026-10-01. The three thresholds are float literals, not globals:
 * their runtime records are LOCAL against this module's rodata base at
 * +0xC/+0x10/+0x14, which is this function's slice of the TU's own literal
 * pool. Written as literals they are constant webs like the two sixteens, so
 * all five hoisted values colour in one descending run, which is the shipped
 * saved-FPR order the load-versus-literal closures called unreachable. The
 * loop is the plain `while (steps--)`. */
void func_overlay_009_F0000540_1866BB8(O9Angle *angle, void *unused,
                                       O9Motion *motion, s32 steps) {
    s32 delta;

    while (steps--) {
        delta = o9P540MathDiffAngleReloc(motion->angle, -angle->angle);
        if ((delta >= -0x3F) && (delta < 0x40) &&
            (motion->velocity > -16.0f) && (motion->velocity < 16.0f)) {
            motion->velocity = 0.0f;
            motion->angle = -angle->angle;
        } else {
            motion->velocity += 20.0f * o9P540CosReloc(delta);
            motion->angle += (s32) motion->velocity;
        }
        motion->velocity *= 0.995f;
        if ((motion->velocity > -0.1f) &&
            (motion->velocity < 0.1f)) {
            motion->velocity = 0.0f;
        }
    }

    delta = motion->angle;
    if (delta < -0x4000) delta = -0x8000 - delta;
    if (delta >= 0x4001) delta = 0x8000 - delta;
    *D_0++ = 0xB;
    *D_0++ = motion->angle;
    *D_0++ = 0xA;
    *D_0++ = -delta;
}

void func_overlay_009_F0000744_1866DBC(O9OutputRecord *output, O9OutputControl *control,
                                       O9OutputState *state, s32 updateCount) {
    f32 level;
    f32 amount;
    s32 i;

    i = updateCount - 1;
    if (updateCount != 0) {
        do {
            func_overlay_009_F00009BC_1867034(output, control, state);
        } while (i--);
    }

    level = state->scale * 0.0125f;
    if (level > 1.0f) {
        level = 1.0f;
    }

    output->pitch = -(s32)(state->x * 8192.0f * level);
    output->yaw = (s32)(state->throttle * state->y *
                        6000.0f * level);

    state->magnitude = control->lean / 20.0f;
    if (state->magnitude < 0.0f) {
        state->magnitude = -state->magnitude;
    }

    amount = state->minimum / 3.0f;
    if (amount < 0.0f) {
        amount = 0.0f;
    }
    if (state->magnitude < amount) {
        state->magnitude = amount;
    }

    if (control->handle == 0) {
        amSndPlayXYZ(0x16, output->x, output->y, output->z, 1,
                     &control->handle);
    }

    if (control->handle != 0) {
        amount = (state->magnitude * 100.0f) + 50.0f;
        amount += (f32)mathRnd(-5, 5);
        amSndSetXYZ(control->handle, output->x, output->y, output->z);
        amSndSetPitchXYZ(control->handle, (u8)(u32)amount);
    }
}

void func_overlay_009_F00009BC_1867034(s16 *angleOut, O9InputControl *control,
                                       O9InputState *state) {
    f32 target;
    f32 one = 1.0f;
    s32 input;

    input = control->inputX;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->x += (target - state->x) * 0.02f;
    if (state->x > 0.0f) {
        control->lean = -state->x * 20.0f;
    } else {
        control->lean = -state->x * 10.0f;
    }

    input = control->inputY;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->y += (-target - state->y) * 0.075f;

    if (control->flags & 0x10) {
        state->throttle += (2.0f - state->throttle) * 0.025f;
    } else {
        state->throttle += (one - state->throttle) * 0.05f;
    }

    state->turnRate +=
        ((state->throttle * state->y * 365.0f) - state->turnRate) * 0.1f;
    control->angleStep = (s16)(s32)state->turnRate;
    control->angle += control->angleStep;
    *angleOut = control->angle;

    if (control->flags & 0x8000) {
        state->acceleration += 0.1f;
        if (state->acceleration > 3.0f) {
            state->acceleration = 3.0f;
        }
    } else if (control->flags & 0x4000) {
        state->acceleration *= 0.95f;
        if ((state->acceleration > -0.01f) &&
            (state->acceleration < 0.01f)) {
            state->acceleration = 0.0f;
        }
    } else {
        state->acceleration -= 0.1f;
    }

    state->position += state->acceleration;
    if (state->position < 40.0f) {
        state->position = (40.0f - state->position) + 40.0f;
        state->acceleration *= -0.4f;
        /* `.1f` and `0.1f` are one value but two literal-pool entries: IDO's
         * pool is keyed on the constant's spelling, not its value, and the
         * shipped pool carries 0x3DCCCCCD twice for this function. Spelling
         * the second one differently is what reproduces the second entry --
         * and with it the +0x50 pool offset the next function's 0.65f needs. */
        if ((state->acceleration > -0.1f) &&
            (state->acceleration < .1f)) {
            state->acceleration = 0.0f;
            state->position = 40.0f;
            return;
        }
    } else if (state->position > 200.0f) {
        state->position = 200.0f;
    }
}

/* Matched 2026-09-10 (lane c6-close). The residual was a single displaced
 * stack home: the target puts `angle` at sp+0x2A, the candidate had it at
 * sp+0x32. The home offset is a linear readout of the declaration index --
 * measured across nine positions it is 78 - 4*index with nothing else moving --
 * so `angle` had to be the tenth declared cell, not the eighth. Hoisting
 * `distance` out of the `if` body into the declaration block (position 8) and
 * declaring `angle` last supplies the two missing cells at an unchanged frame
 * of 0x50, and the 0.65f pool addend follows. */
void func_overlay_009_F0000CE4_186735C(O9IntegrateOutput *out, O9IntegrateControl *control,
                                       void *unused, f32 step) {
    f32 xVelocity;
    f32 yVelocity;
    f32 zVelocity;
    volatile f32 unusedExtra;
    f32 xExtra;
    f32 yExtra;
    f32 zExtra;
    f32 distance;
    f32 fraction;
    s16 angle = control->angle;

    if (control->active != 0) {
        distance = (control->velocity * step) +
            (0.5f * control->acceleration * step * step);
        xExtra = control->dirX * distance;
        yExtra = control->dirY * distance;
        zExtra = control->dirZ * distance;
        if (distance < 0.0f) {
            control->velocity = 0.0f;
            control->acceleration = 0.0f;
            control->active = 0;
        }
        fraction = 1.0f - (control->velocity / control->speedLimit);
        control->velocity += control->acceleration * step;
        xVelocity = ext_o0_2a470(angle) * control->scaleX * fraction;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX * fraction;
    } else {
        xExtra = 0.0f;
        yExtra = 0.0f;
        zExtra = 0.0f;
        xVelocity = ext_o0_2a470(angle) * control->scaleX;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX;
    }
    yVelocity = 0.0f;
    out->zero = 0.0f;
    if (control->mode == 1) {
        xVelocity *= 0.65f;
        zVelocity *= 0.65f;
        if ((control->scaleX < -0.5f) || (control->scaleX > 0.5f))
            control->scaleX *= 0.65f;
        else
            control->scaleX = 0.0f;
        if ((control->scaleZ < -0.5f) || (control->scaleZ > 0.5f))
            control->scaleZ *= 0.65f;
        else
            control->scaleZ = 0.0f;
    }
    ext_o0_7cd8(out, (xVelocity * step) + xExtra,
                 (yVelocity * step) + yExtra,
                 (zVelocity * step) + zExtra);
    fraction = 1.0f / step;
    out->dx = (out->x - control->originX) * fraction;
    out->dz = (out->z - control->originZ) * fraction;
    ext_o0_1d920(out, control, step);
}

void func_overlay_009_F0000F6C_18675E4(O9Point *point, O9Height *offset,
                                       s32 steps) {
    f32 current, result, distance, candidate;
    s32 count;
    O9Hit **hits;
    s32 i;

    count = ext_o0_1353c(point->x, point->z, 0x1000, &hits);
    current = point->y;
    result = current;
    distance = 1000.0f;
    i = count - 1;
    if (count != 0) {
        do {
            candidate = current - hits[i]->height;
            if (candidate < 0.0f) candidate = 0.0f;
            if (candidate < distance) {
                distance = candidate;
                result = hits[i]->height + offset->height;
            }
        } while (i--);
    }
    candidate = current;
    while (steps--) {
        candidate += (result - candidate) * D_54;
    }
    result += 40.0f - offset->height;
    if (candidate < result) candidate = result;
    ext_o0_7cd8(point, 0.0f, candidate - current, 0.0f);
}

void overlay9Ignore(volatile s32 unused0, volatile s32 unused1, volatile s32 unused2) {
}

/* Workbench: allocation-mismatch, 282/282 instructions/frame -152, 20 masked (28 raw)
 * words; first code divergence +0xAC. Was 41, and 52 before that.
 * Three levers moved it, and each is a statement- or carrier-identity fact
 * rather than a declaration-order one -- the 344,946-evaluation declaration
 * search that preceded them could not reach any of the three.
 * 1. Computing `steps` before the D_388 block numbers its spill web below the
 *    D_388 cursor's, so the two GPR homes land at 0x34/0x38 the way the target
 *    has them (-8).  `i` stays at 0x30 on both sides, and as1 still schedules
 *    the conversion itself back down to its old rows, so only the homes move.
 * 2. Spelling the table index as `((u8)(rand & 3) * 4) + D_388[mode]` (-8).
 *    Measured on `cc -S`, ugen emits a commutative add's RIGHT operand subtree
 *    first and puts its result in `rs`; the target's `addu` has the `lbu` of
 *    D_388[mode] in `rs`, so D_388[mode] is the right operand.  The swap alone
 *    is +25 because it also drops the redundant `& 0xFF` node; by L52 an
 *    unsigned conversion is a node, and `(u8)` restores the temp count.
 * 3. Carrying the tilt target and its blend rate in `yawA`/`yawB`, which are
 *    dead by then, rather than in `targetTilt` and `blend` (-5).  That removes
 *    two FP webs from the colouring and puts `cross` on f16 and the D_70 rate
 *    on f2 exactly as the target has them.  `targetTilt` still has to be
 *    DECLARED -- dropping it moves every home below it and costs 15 words --
 *    so it now reserves a home and carries nothing, the same shape overlay 8's
 *    `motionTarget` has.  Carrying the speed target in it instead is
 *    byte-identical, so the object cannot say which of the two is the original.
 * Residual, 20 words, three classes.  Ten of them are one ugen ring fact: the
 * target spends three ring temps inside the table-index statement and lets the
 * scaled index take the fourth ring slot, while every spelling that emits the
 * memory read first (the operand order the target needs) either spends four
 * temps and colours the scaled index into a0, or spends three and then colours
 * the scaled index one ring slot early.  The two requirements have not been
 * satisfiable together in 18 forms of that statement; the deciding variable is
 * uopt's colour for the scaled-index web, not the operand order, which is now
 * right.  The other classes are 3 words of ring rotation at the second
 * `0x8000 - state->angle` call, downstream of the same offset, 3 words where
 * yawA wants FP pool colour f18 and gets f14, and 2 where the tilt constant
 * wants f12 and gets f14 -- both want a HIGHER colour, so the target has an
 * interfering FP web this body does not.  Opcodes, schedule, size, frame and
 * all 31 relocation offsets/types align.  Retain NON_MATCHING.
 *
 * 2026-09-11, lane f9-audit: 20 -> 6.  The "ugen ring fact" above was a
 * globalcolor fact read from the wrong side: the candidate spent a coloured
 * web on the scaled index (a temp, save 3.0, cost 0, always coloured) where
 * the target holds it in a ring temp.  No spelling that keeps `tableIndex` as
 * the element index can remove that web, because the shared `<< 2` between the
 * two reads is a uopt CSE temp.  Making the declared local carry the BYTE
 * offset -- `tableIndex = (... + D_388[mode]) * 4` and
 * `*(f32 *)((u8 *)D_300 + tableIndex)` -- moves the CSE onto the pre-scale
 * sum (coloured a0, as the target) and leaves the local itself as the ring
 * temp: 20 -> 8, with the `(u8)` node no longer needed.  Then the first angle
 * accumulator as `+=` is 8 -> 6 (it was flat at the old base; the ring phase
 * moved with the first edit), and the tilt target carried in `speedTarget`
 * rather than `yawA` puts the +-10 constant on f12.  What remains is two FP
 * colours, proved complete by force (p1 cross=c28, yawA piece=c29 gives
 * masked 0 at delta 0): both webs take the first free colour after c24..c26
 * and the target takes one higher, so the target has a c27 (f14) web that
 * interferes with both and this body does not.  In the target f14 carries
 * only the two loop rates (D_58, D_78); neither is live in the cross block
 * here, and hoisting `blend = D_78` above the calls is dead-store-eliminated
 * or +12 bytes.  The census is 915 p1 / 0 p2 records, so [L108] retires
 * definition/declaration/statement order for this procedure.  A second
 * round measured the obvious suppliers of that web (a state->tilt carrier,
 * the D_70 rate or the +-10 select defined before cross, a named post-loop
 * tilt read): every one changes the instruction count or moves the select's
 * blocks; the select in targetTilt before cross does reproduce both target
 * colours (28 words) and is the receipt for the mechanism, not the answer. */
void func_overlay_009_F00010B4_186772C(O9MotionResult *out, O9MotionOwner *owner,
                                       f32 stepsFloat) {
    O9MotionState *state = owner->state;
    s32 mode = state->mode & 3;
    s32 steps;
    s32 i;
    s32 tableIndex;
    s16 targetAngle;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 crossA;
    f32 dot;
    f32 crossB;
    f32 trigA;
    f32 trigB;
    f32 yawA;
    f32 smoothY;
    f32 yawB;
    f32 targetX;
    f32 targetY;
    f32 targetTilt;
    f32 speedTarget;
    f32 blend;

    ext_o0_210b4(60.0f, 0);
    steps = (s32) stepsFloat;
    if (state->flags & 8) D_388[mode]++;
    D_388[mode] &= 3;
    tableIndex = (((ext_o0_214c8() & 3) * 4) + D_388[mode]) * 4;
    targetX = *(f32 *)((u8 *)D_300 + tableIndex) + (D_2D0 * 75.0f);
    targetY = *(f32 *)((u8 *)D_340 + tableIndex);
    targetAngle = D_380[mode];
    i = steps - 1;

    if (steps != 0) {
        do {
            state->angle += ext_o0_2a5bc(state->angle, 0x8000 - state->angleTarget) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        do {
            out->targetAngle += ext_o0_2a5bc(out->targetAngle,
                                             targetAngle) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        blend = D_58;
        do {
            out->smoothX += (targetX - out->smoothX) * blend;
            out->smoothY += (targetY - out->smoothY) * blend;
        } while (i--);
        i = steps - 1;
    }

    trigA = ext_o0_2a470(0x8000 - state->angle);
    trigB = ext_o0_2a46c(0x8000 - state->angle);
    yawA = ext_o0_2a470(out->targetAngle - targetAngle);
    yawB = ext_o0_2a46c(out->targetAngle - targetAngle);
    /* baseX, not a cross of its own: its live range already overlaps
     * blend's c27 web, so the merged symbol inherits that edge (L115).
     * And smoothY is cached for this one use only.  The copy is deleted by
     * as1's peephole, so it costs no instruction, but the web it creates
     * survives into the colouring, takes f14, and interferes with yawA --
     * which is what puts yawA on f18 instead.  Caching it at its second use
     * as well, or caching smoothX at both, is 28 and 34 words. */
    smoothY = out->smoothY;
    baseX = (out->smoothX * yawB) - (smoothY * yawA);
    crossA = baseX * trigA;
    dot = (out->smoothX * yawA) + (out->smoothY * yawB);
    crossB = baseX * trigB;

    if (state->direction == 0) speedTarget = -10.0f;
    else speedTarget = 10.0f;
    if (steps != 0) {
        yawB = D_70;
        do {
            state->tilt += (speedTarget - state->tilt) * yawB;
        } while (i--);
        i = steps - 1;
    }

    baseX = owner->x + (state->axisX * state->tilt);
    baseY = owner->y + (state->axisY * state->tilt);
    baseZ = owner->z + (state->axisZ * state->tilt);
    trigA = ext_o0_2a470(state->angle + 0x4000);
    trigB = ext_o0_2a46c(state->angle + 0x4000);
    blend = D_78;
    speedTarget = state->input;
    if (speedTarget < 0.0f) speedTarget = -speedTarget;
    if (speedTarget > 1.0f) speedTarget = 1.0f;
    speedTarget *= (f32) state->speedScale * D_7C;
    if (steps != 0) {
        do {
            state->speed += (speedTarget - state->speed) * blend;
        } while (i--);
        i = steps - 1;
    }

    baseX += state->speed * trigA;
    baseZ -= state->speed * trigB;
    out->x = baseX + crossA;
    out->y = baseY + dot;
    out->z = baseZ + crossB;
    out->angle = state->angle;
    if (steps != 0) {
        do {
            out->bank += ext_o0_2a5bc(out->bank,
                                      owner->bankLimit >> 1) >> 5;
        } while (i--);
    }
}
