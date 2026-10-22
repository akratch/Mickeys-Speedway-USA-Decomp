#include "overlays/overlay_041.h"

static s16 overlay41InterpolateAngle(f32 amount, s16 start, s16 end);
static void func_overlay_041_F0001298_18885D0(Overlay41Input *input,
                                               Overlay41Object *state,
                                               s32 step);
static void func_overlay_041_F0001464_188879C(Overlay41Input *input,
                                              Overlay41Object *context,
                                              s32 argument);

/* Updating both accumulators before decrementing the independent remaining
 * field preserves the source semantics and reproduces IDO's retail schedule.
 * Pinned exact-overlay scans found no exact DKR/JFG donor. */
void func_overlay_041_F0000000_1887338(s32 amount) {
    Overlay41StepRecord *record;
    s32 i;
    s32 x;
    s32 y;
    s32 step;

    record = gOverlay41StepRecords;
    i = 7;
    do {
        if (record->id != 0xFF) {
            step = record->remaining;
            if (step != 0) {
                if (amount < step) {
                    step = amount;
                }
                record->x += record->dx * step;
                record->y += record->dy * step;
                record->remaining -= step;
            }
            x = ((record->x * amount) >> 8) + (record->residual & 0xF);
            y = ((record->y * amount) >> 8) + (record->residual >> 4);
            overlay41EmitStep(record->id, x >> 3, y >> 3);
            record->residual = x & 7;
            record->residual |= (y & 7) << 4;
        }
        record++;
    } while (i--);
}

/* Matched 2026-09-12, lane p9-tight. Fifteen words to zero, and the four
 * `initial*` locals were never there: the target's snapshot is a RE-READ of
 * the record field, which uopt common-subexpressions back to the first load,
 * so the load lands in the CSE temp and the working value is the copy. Both
 * the plateau's direction (load into the working local, copy into the
 * snapshot) and its mirror put the load and the copy in the wrong groups, and
 * no colour reachable from either closes it -- the plateau measured 15 and the
 * mirror 25.
 *
 * Read off an instrumented uopt whose object is byte-identical to the
 * configured build. On the plateau body the four working webs take v1/t0/t1/s2
 * at save 20 and the four snapshot webs a1/a2/a3/t2 at save 10, and `remaining`
 * at save 15 lands between them and steals $a0. Forcing `remaining` to c9 and
 * the snapshots to c3-c6 is accepted and scores 8, not 0, which is how the
 * load/copy half was separated from the colour half: the colour was worth 7
 * and the group membership 8, and only a source change reaches the second.
 * With the re-read the CSE temp is its own web and both halves fall together.
 *
 * The 0x80000000 `lui` and the four direct-write `sb` displacements carry
 * relocations in the extracted target and literals here; they are masked, and
 * `promotion-proof` covers the linked identity. */
void func_overlay_041_F0000124_188745C(s32 amount) {
    Overlay41ColorRecord *record;
    s32 i;
    s32 handle;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 remaining;
    s32 factor;
    u8 *direct;

    record = D_800D6C58;
    i = 11;
    do {
        handle = record->handle;
        if (handle != 0) {
            red = record->red;
            green = record->green;
            blue = record->blue;
            alpha = record->alpha;
            remaining = record->remaining;
            if (amount >= remaining) {
                record->handle = 0;
            } else {
                record->remaining = remaining - amount;
                factor = (record->remaining << 16) / record->duration;
                red += ((record->targetRed - record->red) * factor) >> 16;
                green += ((record->targetGreen - record->green) * factor) >> 16;
                blue += ((record->targetBlue - record->blue) * factor) >> 16;
                alpha += ((record->targetAlpha - record->alpha) * factor) >> 16;
            }
            if (handle < 0) {
                changeLightColour(handle, red & 0xFF, green & 0xFF, blue & 0xFF);
                changeLightIntensity(handle, alpha & 0xFF);
            } else {
                direct = (u8 *)((u32)handle | 0x80000000U);
                direct[0x38] = red;
                direct[0x39] = green;
                direct[0x3A] = blue;
                direct[0x3B] = alpha;
            }
        }
        record++;
    } while (i--);
}

/* PROVENANCE: Diddy Kong Racing, src/objects.c
 * (cubic_spline_interpolation), adapted coefficient source shape; Jet Force
 * Gemini's animseqSampleCurve is a structural/name analogue only. Mickey's
 * ROM decides every detail. */
/* Keeping each leading coefficient term last reproduces the retail
 * floating-point evaluation order and temporary schedule. */
void func_overlay_041_F00002AC_18875E4(Overlay41CurveNode *node, f32 t,
                                        f32 *valueOut, f32 *tangentOut,
                                        s32 component) {
    Overlay41CurveNode *p0;
    Overlay41CurveNode *p2;
    Overlay41CurveNode *p3;
    f32 difference;
    volatile f32 ret;
    f32 coefficient0;
    f32 coefficient1;
    f32 coefficient2;
    f32 values[4];
    s32 angle;
    s32 i;
    s32 j;


    p0 = node->previous;
    p2 = node->next;
    p3 = p2->next;
    angle = 0;

    switch (component) {
        case 0:
            values[0] = p0->x;
            values[1] = node->x;
            values[2] = p2->x;
            values[3] = p3->x;
            break;
        case 1:
            values[0] = p0->y;
            values[1] = node->y;
            values[2] = p2->y;
            values[3] = p3->y;
            break;
        case 2:
            values[0] = p0->z;
            values[1] = node->z;
            values[2] = p2->z;
            values[3] = p3->z;
            break;
        case 3:
            angle = 1;
            values[0] = p0->angleX;
            values[1] = node->angleX;
            values[2] = p2->angleX;
            values[3] = p3->angleX;
            break;
        case 4:
            angle = 1;
            values[0] = p0->angleY;
            values[1] = node->angleY;
            values[2] = p2->angleY;
            values[3] = p3->angleY;
            break;
        case 5:
            angle = 1;
            values[0] = p0->angleZ;
            values[1] = node->angleZ;
            values[2] = p2->angleZ;
            values[3] = p3->angleZ;
            break;
        default:
            values[0] = p0->scale;
            values[1] = node->scale;
            values[2] = p2->scale;
            values[3] = p3->scale;
            break;
    }

    if (angle != 0) {
        for (i = 0; i < 3; i++) {
            difference = values[i + 1] - values[i];
            if (difference > 32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    values[j] -= 65536.0f;
                }
            } else if (difference < -32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    values[j] += 65536.0f;
                }
            }
        }
    }

    if (node->linear != 0) {
        if (valueOut != 0) {
            *valueOut = values[1] + ((values[2] - values[1]) * t);
        }
        if (tangentOut != 0) {
            *tangentOut = values[2] - values[1];
        }
    } else {
        ret = values[1];
        coefficient0 = (-0.56f * values[0]) + (0.56f * values[2]);
        coefficient1 = (1.12f * values[0]) + (-2.44f * values[1]) +
                       (1.88f * values[2]) + (values[3] * -0.56f);
        coefficient2 = (-0.56f * values[0]) + (1.44f * values[1]) +
                       (-1.44f * values[2]) + (values[3] * 0.56f);
        if (valueOut != 0) {
            *valueOut = (((coefficient2 * t) + coefficient1) * t +
                         coefficient0) * t + ret;
        }
        if (tangentOut != 0) {
            *tangentOut = ((((coefficient2 * 3.0f) * t) +
                            (2.0f * coefficient1)) * t) + coefficient0;
        }
    }
}

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
static s16 overlay41InterpolateAngle(f32 amount, s16 start, s16 end) {
    s32 delta;
    s32 reverse;

    delta = (u16)(end - start);
    reverse = (u16)(start - end);
    if (reverse < delta) {
        delta = -reverse;
    }
    return start + (s32)(delta * amount);
}

/* Four spellings in this body are load-bearing; each is a regression on its
 * own and only the four together reach byte identity.
 *
 * 1. The arc-length walk is a top-tested `while`, not the m2c
 *    `do { } while`. Both run the same iterations, but only the top-tested
 *    form gives `t` a lower uopt web number than the loop's 1.0f compare
 *    temporary, so `t` is coloured f12 and the constant f0 rather than the
 *    reverse; that one web number decides the whole FP pool (94/94 slots).
 * 2. `node = node->next` inside the restart, rather than a copy of a
 *    separately named `next`, leaves `node->next` available as a common
 *    subexpression on both edges into the sample block. That is what lets
 *    `sampleX` read through the already-loaded pointer while `sampleY` and
 *    `sampleZ` reload it -- the store to the address-taken `sampleX` kills
 *    the CSE between them. It also keeps `node` in s1 and `object` in s2.
 * 3. `node->next != node->next->next`, written in that order: both operands
 *    are memory loads here, so neither is a copy-propagated carrier and the
 *    written order does reach the emitted `beql`/`bnel`.
 * 4. The `(s16)` in `targetX += (s16)(s32)(dx * frameAmount)` is redundant
 *    for the value -- the s16 destination truncates either way -- but it
 *    changes when ugen takes the `targetX` reload out of the temp ring, and
 *    with it the ring position of every later block. All three axes need it.
 */
void func_overlay_041_F0000854_1887B8C(Overlay41Input *input, f32 amount,
                                       s32 updateRate, s32 argument) {
    s32 iteration;
    s16 targetX;
    s16 targetY;
    s16 targetZ;
    f32 currentSpeed;
    f32 averageSpeed;
    f32 distance;
    f32 sampleX;
    f32 sampleY;
    f32 sampleZ;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 precision;
    f32 distanceTotal;
    f32 t;
    f32 precisionDelta;
    f32 probeT;
    f32 remaining;
    f32 probeSpeed;
    f32 elapsed;
    f32 frameAmount;
    f32 blend;
    Overlay41CurveNode *node;
    Overlay41Object *object;

    if (input != 0 && input->object != 0 && !(amount < 0.0f)) {
        object = input->object;
        frameAmount = amount * 60.0f;
        func_overlay_041_F0001298_18885D0(input, object, argument);
        if ((input->flags & 4) || (D_8007D6A4 < 0)) {
            func_overlay_041_F0001464_188879C(input, object, argument);
            func_8003EDEC(object, updateRate);
            return;
        }

        node = input->node;
        t = input->t;
        if (amount == 0.0f) {
            func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &object->x, 0, 0);
            func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &object->y, 0, 1);
            func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &object->z, 0, 2);
            object->positionTag = func_8000FAE0(object->x, object->y, object->z);
        } else if (node->speed != 0.0f || node->next->speed != 0.0f) {
            precision = 0.01f;
            precisionDelta = 0.005f;
            remaining = amount;
            iteration = 0;
            currentSpeed = node->speed + (t * (node->next->speed - node->speed));
            object->velocityX = object->x;
            object->velocityY = object->y;
            object->velocityZ = object->z;
            distanceTotal = 0.0f;

            while (iteration < 9 &&
                   (t != 1.0f || node->next != node->next->next)) {
                if (t == 1.0f) {
                    node = node->next;
                    precision = 0.01f;
                    precisionDelta = 0.005f;
                    t = 0.0f;
                    iteration = 0;
                }

                probeT = t + precision;
                if (probeT > 1.0f) {
                    probeSpeed = node->next->speed;
                    sampleX = node->next->x;
                    probeT = 1.0f;
                    sampleY = node->next->y;
                    sampleZ = node->next->z;
                } else {
                    probeSpeed = node->speed +
                                 (probeT * (node->next->speed - node->speed));
                    func_overlay_041_F00002AC_18875E4_o041Reloc(node, probeT,
                                                       &sampleX, 0, 0);
                    func_overlay_041_F00002AC_18875E4_o041Reloc(node, probeT,
                                                       &sampleY, 0, 1);
                    func_overlay_041_F00002AC_18875E4_o041Reloc(node, probeT,
                                                       &sampleZ, 0, 2);
                }

                dx = sampleX - object->x;
                dy = sampleY - object->y;
                dz = sampleZ - object->z;
                distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                averageSpeed = (currentSpeed + probeSpeed) * 0.5f;
                if (averageSpeed > 0.0f) {
                    elapsed = distance / averageSpeed;
                    remaining -= elapsed;
                    if (((-0.00001f < remaining) &&
                         (remaining < 0.00001f)) || iteration == 8) {
                        iteration = 9;
                        object->x = sampleX;
                        object->y = sampleY;
                        object->z = sampleZ;
                        object->positionTag =
                            func_8000FAE0(sampleX, sampleY, sampleZ);
                        t = probeT;
                        distanceTotal += distance;
                    } else if (iteration == 0 && remaining > 0.0f) {
                        object->x = sampleX;
                        object->y = sampleY;
                        object->z = sampleZ;
                        object->positionTag =
                            func_8000FAE0(sampleX, sampleY, sampleZ);
                        t = probeT;
                        currentSpeed = probeSpeed;
                        distanceTotal += distance;
                    } else {
                        if (remaining > 0.0f) {
                            precision += precisionDelta;
                        } else {
                            precision -= precisionDelta;
                        }
                        precisionDelta *= 0.5f;
                        remaining += elapsed;
                        iteration++;
                    }
                } else {
                    iteration = 9;
                }
            }

            object->velocityX = (object->x - object->velocityX) / frameAmount;
            object->velocityY = (object->y - object->velocityY) / frameAmount;
            object->velocityZ = (object->z - object->velocityZ) / frameAmount;
        }

        input->t = t;
        input->node = node;
        if (input->scaleTimer != 0) {
            if (updateRate < input->scaleTimer) {
                input->scaleAmount += input->scaleRate * frameAmount;
                input->scaleTimer -= updateRate;
            } else {
                input->scaleAmount += input->scaleRate * (f32)input->scaleTimer;
                input->scaleTimer = 0;
            }
        }

        func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &dx, 0, 6);
        object->scale = object->header->scale * input->scaleAmount * dx;
        if (object->scalePair != 0) {
            object->scalePair[0] = object->header->scaleX * dx;
            object->scalePair[1] = object->header->scaleY * dx;
        }

        targetX = object->angleX;
        targetY = object->angleY;
        targetZ = object->angleZ;
        switch (input->orientationMode) {
            case 1:
                dx = (f32)node->tangentX +
                     ((f32)(node->next->tangentX - node->tangentX) * t);
                dy = (f32)node->tangentY +
                     ((f32)(node->next->tangentY - node->tangentY) * t);
                dz = (f32)node->tangentZ +
                     ((f32)(node->next->tangentZ - node->tangentZ) * t);
                targetX += (s16)(s32)(dx * frameAmount);
                targetY += (s16)(s32)(dy * frameAmount);
                targetZ += (s16)(s32)(dz * frameAmount);
                break;
            case 2:
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, 0, &dx, 0);
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, 0, &dy, 1);
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, 0, &dz, 2);
                targetX = func_8002A910(dx, dz) - 0x8000;
                targetY = func_8002A910(dy,
                                        sqrtf((dx * dx) + (dy * dy) +
                                              (dz * dz)));
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &dz, 0, 5);
                targetZ = (s32)dz;
                break;
            case 3:
                if (D_800D6B00[input->targetIndex] != 0) {
                    Overlay41Object *targetObject;

                    targetObject = D_800D6B00[input->targetIndex]->object;
                    if (targetObject != 0) {
                        dx = targetObject->x - object->x;
                        dy = targetObject->y - object->y;
                        dz = targetObject->z - object->z;
                        targetX = func_8002A910(dx, dz) - 0x8000;
                        targetY = func_8002A910(dy,
                                                sqrtf((dx * dx) + (dy * dy) +
                                                      (dz * dz)));
                        targetZ = 0;
                    }
                }
                break;
            default:
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &dx, 0, 3);
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &dy, 0, 4);
                func_overlay_041_F00002AC_18875E4_o041Reloc(node, t, &dz, 0, 5);
                targetX = (s32)dx;
                targetY = (s32)dy;
                targetZ = (s32)dz;
                break;
        }

        if (input->smoothing != 1.0f) {
            blend = 1.0f - func_8002A878(1.0f - input->smoothing, updateRate);
            t = blend;
            object->angleX = overlay41InterpolateAngle(
                t, object->angleX, targetX);
            object->angleY = overlay41InterpolateAngle(
                t, object->angleY, targetY);
            object->angleZ = overlay41InterpolateAngle(
                t, object->angleZ, targetZ);
        } else {
            object->angleX = targetX;
            object->angleY = targetY;
            object->angleZ = targetZ;
        }

        func_overlay_041_F0001464_188879C(input, object, updateRate);
        func_8003EDEC(object, updateRate);
    }
}

s32 overlay41IsUnitScale(s32 index) {
    Overlay41Entity *object;

    object = D_800D6B00[index];
    if (object != NULL && object->scale == 1.0f) {
        return 1;
    }
    return 0;
}

/* Matched 2026-09-16 (lane lm-c), 14 -> 0 masked words at delta 0 in four
 * measured batches; every step is a carrier deletion, none a colour force:
 *
 *   - `limit` is `u8`, so `divisor = limit` is a widening conversion node
 *     rather than a plain copy, and uopt keeps it as the real `move` the
 *     target emits in the first branch's delay slot instead of coalescing
 *     divisor into limit (14 -> 8, with the start read moved after the
 *     clamp, where the target reads it).
 *   - the compare reads the stored field back (`state->value39 < 0xFF`);
 *     uopt forwards the store into that reload and masks it, which is the
 *     target's `sb` of the unmasked sum beside an `andi` of it (8 -> 4).
 *   - no `value`, `start` or `delta` local at all (L160): the sum is a ring
 *     temporary in the target, the start load is one CSE'd temp used twice,
 *     and a declared carrier for either takes a pool colour that pushes
 *     `divisor` off `a0` (4 -> 1).
 *   - the redundant `& 0xFFFF` on the flags06 read-modify-write was a ring
 *     phase device fitted to the old shape (L146); on this shape it is the
 *     one surplus draw (1 -> 0).
 *
 * Twelve of the 32 cells in the final lattice are at 0 to 5 words and the
 * only zero is this one: the operand order `start + quotient` matters at the
 * last word, the mask device and the value local are each worth 16 to 20
 * when present. */
static void func_overlay_041_F0001298_18885D0(Overlay41Input *input,
                                               Overlay41Object *state,
                                               s32 step) {
    Overlay41Header *descriptor;
    s32 active;
    s32 current;
    u8 limit;
    s32 divisor;

    descriptor = state->header;
    active = descriptor->scaleD4 != 0.0f;
    if ((active && descriptor->status[state->selector93] == 0) ||
        (!active && descriptor->status[0] == 0)) {
        if (input->mode14 != 0) {
            func_8005ABA8(state, -input->amount0C, (f32)step);
        } else {
            func_8005ABA8(state, input->amount0C, (f32)step);
        }
    }

    current = input->current27;
    limit = input->limit26;
    if (current < limit) {
        divisor = limit;
        input->current27 = current + step;
        current = input->current27;
        if (current >= limit) {
            input->current27 = limit;
            current = limit & 0xFF;
        }
        state->value39 = input->start24 + ((input->end25 - input->start24) * current) / divisor;
        if (state->value39 < 0xFF) {
            state->flags06 |= 4;
        } else if (!(state->header->flags14 & 4)) {
            state->flags06 &= ~4;
        }
    } else {
        state->value39 = input->end25;
    }

    if (input->flags & 2) {
        D_800D6B08 = state;
    }
}

/* Matched 2026-09-19 (lane w31-o041p), 120 -> 0 masked words at delta 0.
 *
 *   - `scaled = (s32)(input->scale * 60.0f)` before the alternateMode test
 *     (delete remat): input stays in a3 instead of its incoming home, which
 *     was the extra word at +0x10 and the five-word overrun (L155).
 *   - unused `pad` declared first (L99): an unused pointer is not eliminated
 *     and occupies +0x44 so mode/child sit at +0x40/+0x38. Listed in
 *     docs/cleanup-queue.md.
 */
static void func_overlay_041_F0001464_188879C(Overlay41Input *input,
                                              Overlay41Object *context,
                                              s32 argument) {
    Overlay41Input *pad;
    s32 mode;
    Overlay41Entry *entry;
    Overlay41Child *child;
    Overlay41Header *state;
    s32 selected;
    Overlay41Entry *firstEntry;
    s32 scaled;

    state = context->header;
    selected = state->scaleD4 != 0.0f;
    if (((selected != 0) &&
         (((Overlay41Header *)((u8 *)state + context->selector93))->status[0] == 1)) ||
        ((selected == 0) && (state->status[0] == 1))) {
        firstEntry = context->entries[context->entryIndex];
        if (firstEntry != 0) {
            scaled = (s32)(input->amount0C * 60.0f);
            mode = 9;
            if (input->mode14 != 0) {
                mode = 11;
            }
            texAnimateSprite(firstEntry, &mode, scaled, context->work28, argument);
        }
    } else if (((selected != 0) &&
                (((Overlay41Header *)((u8 *)state + context->selector93))->status[0] == 0)) ||
               ((selected == 0) && (state->status[0] == 0))) {
        entry = context->entries[context->entryIndex];
        if (entry->active != 0) {
            child = entry->child;
            if (child->refreshEnabled != 0) {
                func_8005AF14(entry, child, context, input);
                lightObject(context, entry, context->argument,
                                    entry->values[entry->index]);
            } else {
                if (child->switchEnabled != 0) {
                    entry->index = 1 - entry->index;
                }
                lightObject(context, entry, context->argument,
                                    entry->values[entry->index]);
            }
            if (child->switchEnabled > 0) {
                func_80007C68(context, child, entry, entry->active);
            }
            entry->active = 0;
        }
    }
}

/* The twelve-slot scan is a top-tested `while (remaining--)`, not the m2c
 * `do { } while (remaining--)`: both run twelve iterations, but only the
 * top-tested form gives the loop-exit copy a lower uopt web number than the
 * counter, so the counter is coloured a1 and its dead copy v1 rather than the
 * reverse. The fade scale is a float literal (the shipped constant, one ulp
 * below 0.6f); in the whole module it lands in the pool between the path
 * updater's literals and the spawn switch table, as shipped. */
void func_overlay_041_F0001650_1888988(void *object, volatile s32 value1,
                                       s32 value3, s32 value5, s32 value7,
                                       f32 amount, s32 alternateColors) {
    register s32 remaining;
    Overlay41ColorRecord *slot;
    u8 *bytes;

    if (object == 0) {
        return;
    }

    slot = D_800D6C58;
    remaining = 12;
    while (remaining--) {
        if (slot->handle == 0) {
            bytes = object;
            if (alternateColors != 0) {
                slot->handle = (u32)object;
                slot->targetRed = bytes[0x40];
                slot->targetGreen = bytes[0x41];
                slot->targetBlue = bytes[0x42];
                slot->targetAlpha = bytes[0x43];
            } else {
                slot->handle = (u32)object & 0x7FFFFFFF;
                slot->targetRed = bytes[0x38];
                slot->targetGreen = bytes[0x39];
                slot->targetBlue = bytes[0x3A];
                slot->targetAlpha = bytes[0x3B];
            }
            slot->red = value1;
            slot->green = value3;
            slot->blue = value5;
            slot->alpha = value7;
            slot->remaining = amount * 0.59999996f;
            slot->duration = slot->remaining;
            return;
        }
        slot++;
    }
}

/* The resident stream handler passes seven arguments (func_800517E0, command
 * 0x6E) and this export reads none of them. IDO homes each unused register
 * argument, which is the whole body; the three stack arguments leave no
 * trace. */
void overlay41Ignore(void *object, s32 value1, s32 value2, s32 value3, s32 value4,
                     s32 value5, s32 value6) {
}

/* The compiler-generated switch table and scalar constant are linked against
 * the overlay's retained data through an ABS symbol. Their duplicate payload
 * is discarded after a digest check; no instruction word is changed. */
void overlay41SpawnItems(s32 rootIndex, s32 count, s32 mode, s32 centerX,
                         s32 centerY, s32 radiusX, s32 radiusY, s32 centerZ,
                         s32 radiusZ) {
    s32 opacity;
    Overlay41ShortPair input;
    Overlay41FloatVector output;
    Overlay41Entity *root;
    Overlay41Item *item;
    f32 scale;

    root = D_800D6B00[rootIndex];
    scale = 1.0f;
    if (root == 0) {
        return;
    }
    item = root->object;
    if (item == 0) {
        return;
    }

    switch (mode) {
        case 0:
            mode = 0;
            break;
        case 1:
            mode = 1;
            break;
        case 2:
            mode = 1;
            scale = 0.5f;
            break;
        case 3:
            mode = 5;
            scale = 0.75f;
            break;
        case 4:
            mode = 4;
            break;
        default:
            mode = 2;
            break;
    }

    if (item->opacity != 0) {
        opacity = *item->opacity * 255.0f;
    } else {
        opacity = 255;
    }

    while (count--) {
        input.x = mathRnd(-radiusX, radiusX) + centerX;
        input.y = mathRnd(-radiusY, radiusY) + centerY;
        output.z =
            (f32)(mathRnd(-radiusZ, radiusZ) + centerZ) * -0.01f;
        mathOneFloatPY(&input, &output);
        func_overlay_012_F00001B4_186D434(
            item->x, item->y, item->kind, output.x, output.y, output.z, opacity,
            mode, scale);
    }
}

/* Matched as the loop it is: five queue entries scanned with an index and a
 * walking entry pointer. The compiler peels the first iteration onto the
 * array's own address and unrolls the other four, which is the hand-unrolled
 * shape, the eleven alias names and the volatile cursor the earlier candidate
 * spelled out. The pointer has to be a second induction variable beside the
 * index: indexing the array, deriving the pointer inside the body, or
 * bounding the loop by the pointer all unroll differently.
 */
void func_overlay_041_F000195C_1888C94(s32 value2, s32 timer, s32 value4,
                                       s32 value6, s32 value8, s32 value9,
                                       s32 valueA) {
    s32 i;
    Overlay41Transition *entry;

    for (i = 0, entry = D_800D6D18; i < 5; i++, entry++) {
        if (entry->intensity == 0 && entry->remaining <= 0) {
            entry->value = value2;
            entry->remaining = timer;
            entry->x = value4;
            entry->y = value6;
            entry->red = value8;
            entry->green = value9;
            entry->blue = valueA;
            entry->intensity = 0;
            return;
        }
    }
}

void overlay41TickTransitions(s32 step) {
    Overlay41Transition *transition;
    s32 i;
    s32 intensity;

    transition = D_800D6D18;
    for (i = 0; i < 5; i++) {
        if ((transition->remaining > 0) || (transition->intensity > 0)) {
            if (transition->remaining > 0) {
                transition->remaining -= step;
                intensity = transition->intensity + (step * 8);
                if (intensity >= 0x100) {
                    transition->intensity = 0xFF;
                } else {
                    transition->intensity = intensity;
                }
            } else if (transition->intensity > 0) {
                intensity = transition->intensity - (step * 8);
                if (intensity < 0) {
                    transition->intensity = 0;
                } else {
                    transition->intensity = intensity;
                }
            }
        }
        transition++;
    }
}

void overlay41DrawItem(void *displayList, Overlay41DrawItem *item) {
    s32 x;
    s32 y;
    s32 width;

    fontUseFont(2);
    fontBackground(0, 0, 0, 0);
    width = fontStringWidth(D_8007C0B8[item->resourceIndex], 2, 1) + 40;
    x = item->x - (width >> 1);
    y = item->y - 12;
    func_8004B0B8(0, 0, 0, 255, item->intensity);
    func_overlay_067_F0000000_18C6F40(displayList, 0, x, y, width, 24,
                                      item->intensity >> 1);
    fontPrintXY(displayList, item->x + 1, item->y + 1,
                  D_8007C0B8[item->resourceIndex], 12);
    func_8004B0B8(item->red, item->green, item->blue, 255, item->intensity);
    fontPrintXY(displayList, item->x, item->y,
                  D_8007C0B8[item->resourceIndex], 12);
}
