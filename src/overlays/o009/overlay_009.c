#include "overlays/overlay_009.h"
#include "tools/m2c/m2c_macros.h"

#undef NULL
#define NULL 0

/* Retail's runtime relocation table addresses this pool from offset 0x20. */
static const u8 sOverlay9ConstantPoolBase[0x20] = { 0 };

typedef struct Overlay9LevelScaleReloc {
    u8 pad00[0x18];
    f32 value;
} Overlay9LevelScaleReloc;

typedef struct Overlay9YawScaleReloc {
    u8 pad00[0x1C];
    f32 value;
} Overlay9YawScaleReloc;

extern Overlay9LevelScaleReloc gOverlay9LevelScaleReloc;
extern Overlay9YawScaleReloc gOverlay9YawScaleReloc;

/*
 * Overlay 9, ADR 0006 consolidation. Functions remain in retail ROM order.
 * The module uses the R4300 multiply-hazard schedule; applying that flag to
 * the intervening empty overlay9Ignore function does not change its bytes.
 */

extern void func_overlay_009_F00010A4_186771C(void *object, void *state,
                                               f32 steps);

/* PLATEAU: exact 0x78 frame. Narrowed scalar lifetimes removed the excess.
 * Static relocation proof remains closed on the ambiguous consolidated
 * F00010A4 identity (static 52, runtime 63). GPR spill homes follow
 * declaration order; the handoff has the current score and next lever. */
#ifdef NON_MATCHING
void func_overlay_009_F0000000_1866678(void *object, s32 steps) {
    f32 vector[3];
    s16 angles[3];
    void *entry;
    f32 savedY;
    f32 stepFloat;
    void *entryData;
    void *savedEntry;
    void *state;

    state = M2C_FIELD(object, void **, 0x64);
    D_410 = (s16 *)((u8 *)state + 0x1B8);
    M2C_FIELD(object, s32 *, 0x80) = 0;
    savedEntry = *M2C_FIELD(object, void ***, 0x68);
    ext_o0_1ee14(state, M2C_FIELD(state, s8 *, 0));
    G_rt_458c4 = D_390;

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

    if ((M2C_FIELD(state, u8 *, 0x16C) == 0) ||
        (M2C_FIELD(state, u8 *, 0x16C) == 1)) {
        func_overlay_009_F0000744_1866DBC(object, state, &D_2D0, steps);
    }
    stepFloat = (f32)steps;
    func_overlay_009_F0000CE4_186735C(object, state, &D_2D0, stepFloat);
    func_overlay_009_F00010A4_186771C(object, state, stepFloat);
    func_overlay_009_F0000F6C_18675E4(object, &D_2D0, steps);
    func_overlay_009_F0000540_1866BB8(object, state, &D_2D0, steps);

    D_2F0 = (s16)(D_2F0 + (((s32)(3072.0f * D_2EC) + 0x400) * steps));
    *D_410++ = 0x22;
    *D_410++ = D_2F0;
    D_2F2 = (s16)(D_2F2 +
        (((s32)(3072.0f * (D_2D8 - 1.0f)) + 0x400) * steps));
    *D_410++ = 0x24;
    *D_410++ = D_2F2;
    D_2FA = (s16)(D_2FA + (steps << 8));
    *D_410++ = 0x2000;

    if ((M2C_FIELD((u8 *)M2C_FIELD(object, void **, 0x40) +
                       M2C_FIELD(object, u8 *, 0x93), s8 *, 0x1E) == 0) &&
        (savedEntry != NULL) && (M2C_FIELD(savedEntry, s16 *, 8) != 0)) {
        f32 current;
        f32 target;
        f32 rate;
        s32 remaining;

        entryData = M2C_FIELD(savedEntry, void **, 0);
        savedY = M2C_FIELD(object, f32 *, 0x10);
        current = M2C_FIELD(state, f32 *, 4);
        if ((current < -2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) < -0x14) ||
            (current > 2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) >= 0x15)) {
            target = 0.0f;
            rate = D_394;
        } else {
            target = 4.0f;
            rate = D_398;
        }
        remaining = steps - 1;
        if (steps != 0) {
            current = D_2FC;
            do {
                current += (target - current) * rate;
            } while (remaining--);
            D_2FC = current;
        }
        M2C_FIELD(object, f32 *, 0x10) +=
            D_2FC * ext_o0_2a470(D_2FA);
        ext_o0_5aac4(savedEntry, entryData, object);
        ext_o0_19668(object, savedEntry, M2C_FIELD(object, void **, 0x50),
                      M2C_FIELD((u8 *)savedEntry +
                          (M2C_FIELD(savedEntry, s16 *, 0xA) * 4), void **, 0xC));
        M2C_FIELD(savedEntry, s16 *, 8) = 0;
        M2C_FIELD(object, f32 *, 0x10) = savedY;
    }

    M2C_FIELD(state, s8 *, 0x186) = 0;
    ext_o0_1d510(object, state, NULL, NULL, steps);
    if (M2C_FIELD(state, u8 *, 0x349) != 0) {
        if (M2C_FIELD(state, u8 *, 0x16C) == 1) {
            void *handle = M2C_FIELD(state, void **, 0xB8);
            M2C_FIELD(state, u8 *, 0x16C) = 0;
            M2C_FIELD(state, s8 *, 0x16E) = 8;
            if (handle != NULL) {
                ext_o0_2d98(handle);
            }
            ext_o0_2b90(6, M2C_FIELD(object, f32 *, 0xC),
                        M2C_FIELD(object, f32 *, 0x10),
                        M2C_FIELD(object, f32 *, 0x14), 4,
                        (u8 *)state + 0xB8);
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000000_1866678.s")
#endif
/* PROVENANCE: Mickey-derived from the assigned overlay assembly range; no donor body was imported.
 * Bounded reproof (2026-08-29): unchanged configured full-TU C under canonical
 * -O2 -mips2 -32 -Wab,-r4300_mul emits the exact 0x204-byte extent and frame
 * 0x58. It matches 120/129 raw words and 121/129 after runtime relocation or
 * real-offset link resolution, first +0x4C. The eight linked residual sites are
 * +0x4C/+0x54/+0x64/+0x6C/+0xA4/+0xB4/+0x128/+0x138.
 *
 * All ten runtime records agree by offset, type, and identity: LOCAL pairs at
 * +0x48/+0x4C, +0x50/+0x54, and +0x58/+0x5C use module base +0x18B0 with
 * addends +0xC/+0x10/+0x14; SYMBOL calls at +0x80/+0xDC are ORT 707
 * mathDiffAngle and ORT 279 func_8002A8C0; the +0x158/+0x168 LOCAL pair uses
 * BSS base +0x1930 with addend zero. A synthetic-VMA link preserves the real
 * F0000540 placement and reproduces the same 121/129 owned-range result.
 * The remaining upper/lower/threshold values form the retained four-way
 * saved-FPR color cycle. Per the reproof bound, no flag, trace, source-variant,
 * or permutation search was run.
 *
 * Reproof and exhaustion (2026-09-09): all eight residual words are saved-FPR
 * register numbers and nothing else. The allocation law was measured on this
 * TU: a hoisted memory load and a materialized literal join different groups,
 * loads taking the low callee-saved float registers ascending in emission
 * order and literals the high ones descending, with the direction fixed per
 * group. The target inverts both directions, which no partition of these five
 * values reaches. A 15,360-point lattice (120 hoist orders x 4 declaration
 * placements x 2 initializer styles x 16 comparison operand orders) scores
 * only 8 or 9 and never less, every 8 carrying the identical offsets; the
 * `register` storage class, in-loop assignment, a named zero, negation-derived
 * literals and joined physical lines are all flat; and a 14-point compiler
 * flag lattice makes the canonical -O2 -mips2 -32 -Wab,-r4300_mul the unique
 * optimum. Preserve this body and assembly fallback until a new saved-FPR
 * group-membership mechanism is proved. See the handoff shard.
 *
 * 2026-09-10, lane o7-mid, bounded reopen: the closure holds at 8, and the
 * residual is restated in a way that names a different decision variable.
 * Read from the object rather than the record, the six callee-saved FP
 * assignments in emission order are
 *
 *   target : D_C f30, D_10 f28, D_14 f26, 16.0 f24, -16.0 f22, 0.0 f20
 *   ours   : D_C f22, D_10 f24, D_14 f26, 16.0 f30, -16.0 f28, 0.0 f20
 *
 * so D_14 and the zero are FIXED POINTS and the only difference is that the
 * pair {D_C, D_10} and the pair {16.0, -16.0} exchange their colour blocks.
 * The target's whole assignment is one descending run f30..f20 in emission
 * order; ours is not.
 *
 * That is better described as a colouring ORDER than as group membership.
 * All five hoisted values carry exactly one loop-level reference each, so
 * their `save` terms tie (L7/L100), and a p1 tie is broken by web number and
 * nothing else (L31, L84). Under that reading the residual is a web-number
 * tie-break, and the reopen condition should be read as "find a web-number
 * lever for f32 locals" -- L85, the one measured renumbering spelling, is a
 * narrow-type truncation at the store and does not apply to f32 at all, and
 * L86 already records declaration order, relational operand order, an added
 * local and a hoist as moving no web number. That is consistent with the
 * recorded lattice flooring at 8 and explains WHY, which the group-membership
 * framing did not.
 *
 * This is inference from the two colour sequences (tier D), not a trace. The
 * one instrument that would settle it is a CDX capture: if the five webs tie
 * on save the record says so directly, and if they do not, the save arithmetic
 * names a cost lever the group framing hides.
 *
 * 2026-09-12, lane p8-close: the target's assignment is now a FORCED RESULT,
 * not an inference, and the reopen condition splits into one half that is
 * reached and one that is arithmetic.
 *
 * Forcing this procedure's five callee-saved float webs to c35, c34, c33 for
 * the three loads in hoist order and c31, c32 for the two sixteens returns an
 * object at zero masked words and size delta 0. So the whole residual is that
 * colour order; nothing else in the function is in question, and any future
 * pass can check a candidate against that one ladder.
 *
 * Phase one colours in decreasing save, lowest free colour first, ties on
 * ascending web number. Our ladder is zero at 20/4, the three loads at 11/4
 * and the two sixteens at 10/4; the target's requires the sixteens strictly
 * ABOVE the loads and the loads strictly DESCENDING in web number.
 *
 * Half one is reached. The divisor is not a constant of the procedure: it is
 * per web, and a basic block placed after the sixteens' last use but inside
 * the loads' live range raises only the loads' divisor. An `if (1) { }` at the
 * end of the else arm takes the loads to 11/5 = 2.2 while the sixteens hold
 * 10/4 = 2.5, and the records then show the sixteens coloured first -- the
 * first time this file has produced that order. It costs four bytes in this
 * spelling and it also stops the twenty splitting, so the candidate scores
 * 101; the value here is the mechanism, not the number.
 *
 * Half two is the blocker and it is now arithmetic. With the loads at divisor
 * 5 and the sixteens at 4, separating the three loads needs three integer
 * numerators a > b > c with a/5 < 2.5, so a <= 12, while c cannot fall below
 * 11 -- ten for the single in-loop reference plus one for the definition
 * occurrence -- and no such triple exists. At divisor 6 the triple 13, 12, 11
 * does exist, so reopening needs TWO such zero-cost blocks plus one extra
 * out-of-loop occurrence of the second load and two of the third, against a
 * target pinned at 129 instructions. L109 supplies none of them for an f32.
 *
 * One more negative, because it is the obvious thing to try: occurrence weight
 * is NOT discounted by conditional nesting. Moving the damping multiply behind
 * an `if` leaves its numerator at 11 and raises every web's divisor uniformly,
 * so per-arm placement cannot separate the three loads. */
#ifdef NON_MATCHING
void func_overlay_009_F0000540_1866BB8(O9Angle *angle, void *unused,
                                       O9Motion *motion, s32 steps) {
    s32 delta;

    if (steps--) {
        f32 lower;
        f32 upper;
        f32 upperThreshold;
        f32 lowerThreshold;
        f32 damping;

        upperThreshold = D_C;
        lowerThreshold = D_10;
        damping = D_14;
        upper = 16.0f;
        lower = -16.0f;
        do {
            delta = o9P540MathDiffAngleReloc(motion->angle, -angle->angle);
            if ((delta >= -0x3F) && (delta < 0x40) &&
                (motion->velocity > lower) && (motion->velocity < upper)) {
                motion->velocity = 0.0f;
                motion->angle = -angle->angle;
            } else {
                motion->velocity += 20.0f * o9P540CosReloc(delta);
                motion->angle += (s32) motion->velocity;
            }
            motion->velocity *= damping;
            if ((motion->velocity > lowerThreshold) &&
                (motion->velocity < upperThreshold)) {
                motion->velocity = 0.0f;
            }
        } while (steps--);
    }

    delta = motion->angle;
    if (delta < -0x4000) delta = -0x8000 - delta;
    if (delta >= 0x4001) delta = 0x8000 - delta;
    *D_0++ = 0xB;
    *D_0++ = motion->angle;
    *D_0++ = 0xA;
    *D_0++ = -delta;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000540_1866BB8.s")
#endif

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

    level = state->scale * gOverlay9LevelScaleReloc.value;
    if (level > 1.0f) {
        level = 1.0f;
    }

    output->pitch = -(s32)(state->x * 8192.0f * level);
    output->yaw = (s32)(state->throttle * state->y *
                        gOverlay9YawScaleReloc.value * level);

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
        ext_o0_2b90(0x16, output->x, output->y, output->z, 1,
                     &control->handle);
    }

    if (control->handle != 0) {
        amount = (state->magnitude * 100.0f) + 50.0f;
        amount += (f32)ext_o0_2952c(-5, 5);
        ext_o0_2d70(control->handle, output->x, output->y, output->z);
        ext_o0_2c64(control->handle, (u8)(u32)amount);
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

void overlay9Ignore(volatile s32 arg0, volatile s32 arg1, volatile s32 arg2) {
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


/* PLATEAU-HANDOFF:func_overlay_009_F0000000_1866678:start
 * symbol: func_overlay_009_F0000000_1866678
 * score: 120/336 words
 * frame: 0x78
 * relocations: 63
 * first-mismatch: +0xE4
 * summary: GPR spills now match 0x50 and 0x54 after declaration reorder. 125 to 120 masked at delta 0. Arrays still +8, savedY -16, stepFloat temp +4.
 * PLATEAU-HANDOFF:func_overlay_009_F0000000_1866678:end
 */

/* PLATEAU-HANDOFF:func_overlay_009_F0000540_1866BB8:start
 * symbol: func_overlay_009_F0000540_1866BB8
 * score: 8/129 words
 * frame: 0x58
 * relocations: 10
 * first-mismatch: +0x4C
 * summary: saved-FPR ordering requires unreachable source net and load-order changes
 * PLATEAU-HANDOFF:func_overlay_009_F0000540_1866BB8:end
 */
