#include "overlays/overlay_027.h"

/*
 * Overlay 27, ADR 0006 consolidation: one translation unit in ROM order.
 * Pinned DKR v77/v80 and JFG scans found no exact donor for this module.
 */

void overlay27Init(O27Object *object, Overlay27InitData *init) {
    O27State *state = object->state;
    state->primaryState = 0; state->pulseState = 0; state->timer = 0;
    state->colorR = 0xFF; state->colorG = 0xFF; state->colorB = 0xFF;
    state->pulseR = 0x40; state->pulseG = 0x40; state->pulseB = 0x40;
    state->intensity = 0; state->fade = 0; state->pulseTimer = 0;
    state->primaryHandle = 0; state->secondaryHandle = 0;
    state->scaleTarget = 96.0f; state->fadeFloat = 0.0f; state->source = init->target;
}

/* Plateau reproof: exact 368-instruction length/frame 0x60; 63 raw (62 relocation-masked) words differ, first +0x18.
 * Moving source below the scalar locals makes its spill pair exact and retains a four-word gain; all 119 flags and ten natural local/lifetime forms are nonexact.
 * Remaining: the update-rate home and persistent FP pool differ; 21/22 relocation offset/type sites and 12/22 static identities are resolved. */
#ifdef NON_MATCHING
void func_overlay_027_F0000064_187BA3C(O27Object *object, s32 updateRate) {
    s32 pulseStep;
    O27State *state;
    union {
        O27State *sourceState;
        s32 intensity;
    } tail;
    s32 initialPhase;
    s32 phase;
    s32 value;
    O27Object *source;
    f32 fraction;
    f32 scaleFactor;

    state = object->state;
    pulseStep = updateRate;
    source = state->source;
    if (source != 0) {
        object->x = source->x;
        object->y = source->y;
        object->z = source->z;
        object->positionTag = source->positionTag;
    } else {
        state->primaryState = 4;
    }

    gO27Active = 1;
    initialPhase = 9;
    func_80036544(*object->updateResource, &initialPhase, 10, &object->reserved30[-8],
                  updateRate);

    if (updateRate != 0) {
        scaleFactor = gO27Scale0;
        do {
            switch (state->primaryState) {
                case 0:
                    state->timer += updateRate;
                    fraction = 1.0f - func_8002A878(gO27EaseInput, updateRate);
                    scaleFactor = gO27Scale8;
                    state->scaleTarget +=
                        (32.0f - state->scaleTarget) * fraction;
                    value = state->timer;
                    updateRate = value - 120;
                    if (value >= 120) {
                        state->primaryState = 1;
                        state->timer = 0;
                        state->scaleTarget = 32.0f;
                        phase = 0x10000;
                    } else {
                        phase = (value << 16) / 120;
                        updateRate = 0;
                    }
                    value = (((-95 * phase) >> 16) + 0xFF);
                    state->colorR = value;
                    state->colorG = value;
                    state->colorB = value;
                    value = (((-64 * phase) >> 16) + 0x40);
                    state->pulseR = value;
                    state->pulseG = value;
                    state->pulseB = value;
                    if (phase >= 0x8000) {
                        state->intensity = 0xFF;
                    } else {
                        state->intensity = (phase * 0xFF) >> 15;
                    }
                    object->scale =
                        1.0f + ((f32)state->intensity * scaleFactor);
                    break;

                case 1:
                    state->timer += updateRate;
                    value = state->timer;
                    updateRate = value - 60;
                    if (value >= 60) {
                        state->primaryState = 2;
                        state->timer = 0;
                        state->fade = 0xFF;
                        state->fadeFloat = 1.0f;
                    } else {
                        updateRate = 0;
                        fraction = (f32)value / 60.0f;
                        state->fadeFloat = fraction;
                        state->fade = (s16)(255.0f * fraction);
                    }
                    break;

                case 2:
                    state->timer += updateRate;
                    value = state->timer;
                    updateRate = value - 480;
                    if (value >= 480) {
                        state->primaryState = 4;
                        state->timer = 0;
                    } else {
                        updateRate = 0;
                    }
                    break;

                case 3:
                    if (state->intensity < 0xFF) {
                        state->intensity += updateRate * 4;
                        phase = state->intensity;
                        updateRate = 0;
                        if (phase >= 0x100) {
                            state->intensity = 0xFF;
                            object->scale = 2.0f;
                        } else {
                            object->scale =
                                1.0f + ((f32)phase * scaleFactor);
                        }
                    } else {
                        state->fade += updateRate * 4;
                        value = state->fade;
                        updateRate = 0;
                        if (value >= 0xFF) {
                            state->primaryState = 2;
                            state->timer = 0;
                            state->fade = 0xFF;
                            state->fadeFloat = 1.0f;
                        } else {
                            state->fadeFloat =
                                (f32)value * scaleFactor;
                        }
                    }
                    break;

                default:
                    value = state->fade;
                    if (value >= updateRate * 8) {
                        state->fade = value - (updateRate * 8);
                        updateRate = 0;
                        state->fadeFloat =
                            (f32)state->fade * scaleFactor;
                    } else {
                        state->intensity -= updateRate * 4;
                        state->fade = 0;
                        updateRate = 0;
                        state->fadeFloat = 0.0f;
                        if (state->intensity <= 0) {
                            func_80006EA0(object);
                            scaleFactor = gO27ScaleC;
                        } else {
                            object->scale = 1.0f +
                                ((f32)state->intensity * scaleFactor);
                        }
                    }
                    break;
            }
        } while (updateRate != 0);
    }

    value = state->fade;
    if (value == 0) {
        return;
    }

    if (state->pulseState == 0) {
        if (value == 0xFF && func_800299E8(0, 0x1FFF) >= 0x1FD7) {
            tail.sourceState = source->state;
            state->pulseState = 1;
            if (state->secondaryHandle != 0) {
                func_800031E8(state->secondaryHandle);
            }
            func_80002FE0(0x1BB, object->x, object->y, object->z, 4,
                          &state->secondaryHandle);
            if (!(tail.sourceState->flags1A8 & 1)) {
                func_8002BD58(*(s8 *)&tail.sourceState->primaryState,
                              0x32, 0.4f);
            }
        }
    } else {
        if (state->pulseState == 1) {
            state->pulseTimer += pulseStep << 6;
        } else {
            state->pulseTimer -= pulseStep << 5;
        }
        value = (pulseStep = state->pulseTimer);
        if (value >= 0x100) {
            state->pulseTimer = 0xFF;
            state->pulseState = 2;
            state->pulseR = 0x80;
            state->pulseG = 0x80;
            state->pulseB = 0;
        } else if (value < 0) {
            state->pulseTimer = 0;
            state->pulseState = 0;
            state->pulseR = 0;
            state->pulseG = 0;
            state->pulseB = 0;
        } else {
            value = (value << 7) >> 8;
            state->pulseR = value;
            state->pulseG = value;
            state->pulseB = 0;
        }
    }

    if (state->primaryHandle == 0) {
        func_80002FE0(0x1B8, object->x, object->y, object->z, 1,
                      &state->primaryHandle);
    }
    tail.intensity = state->fade >> 1;
    if (tail.intensity >= 0x80) {
        tail.intensity = 0x7F;
    }
    if (state->primaryHandle != 0) {
        func_800031C0(state->primaryHandle, object->x, object->y, object->z);
        func_8000309C(state->primaryHandle, tail.intensity);
    }
    if (state->secondaryHandle != 0) {
        func_800031C0(state->secondaryHandle, object->x, object->y, object->z);
        func_8000309C(state->secondaryHandle, tail.intensity);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o027/overlay_027/func_overlay_027_F0000064_187BA3C.s")
#endif

/* Mickey-local rendering reconstruction; donor scans are exact-negative. */
/* Workbench p7: structure-mismatch, 254/254 instructions/frame -152, 195 masked (196 raw) words, first +0x8.
 * Stack-home “constants” are the candidate’s 88-vs-target-100(sp) carrier; context/flag and prior allocation/lifetime/order probes remain exhausted.
 * Coupled object/child/register web and 19 overlay-local relocation identities remain; retain NON_MATCHING. */
#ifdef NON_MATCHING
void func_overlay_027_F0000624_187BFFC(O27Command **commands, void *arg1,
                                       s16 *arg2, O27Object *object) {
    O27Work work;
    O27Command *command;
    O27State *state;
    O27Child *child;
    s16 *value;
    void *displayList;
    void *finishArg;
    f32 oldScale;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    s32 intensity;

    value = overlay27GetValue();
    state = object->state;
    child = (O27Child *)state->source;
    if (child != 0) {
        scale = overlay27GetChildScale(child);
    } else {
        scale = 1.0f;
    }

    if (state->fade != 0) {
        if (child != 0) {
            work.transform.positionX = child->x;
            work.transform.positionY = child->y;
            work.transform.positionZ = child->z;
        } else {
            work.transform.positionX = object->x;
            work.transform.positionY = object->y;
            work.transform.positionZ = object->z;
        }

        work.transform.x = -*value;
        work.transform.y = 0;
        work.transform.z = 0;
        work.transform.scale = scale;
        intensity = 0x100;
        work.transform.positionY += 24.0f * scale;

        if (child != 0 && child->factor != 0) {
            intensity = (s32)(*child->factor * 256.0f);
        }

        displayList = *object->renderResource->displayList;
        overlay27Prepare(commands, arg1, &work.transform, 1.0f, 0.0f);
        overlay27DrawPart(commands, displayList, 0x214, 0);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000;
        command->w1 = (((intensity * 0x60) >> 8) << 24) |
                      ((((intensity * 0xE0) >> 8) & 0xFF) << 16) |
                      ((((intensity * 0xFF) >> 8) & 0xFF) << 8) |
                      (state->fade & 0xFF);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFB000000;
        command->w1 = ((((intensity << 7) >> 8) & 0xFF) << 8) | 0xFF;

        command = *commands;
        *commands = command + 1;
        command->w0 = (((((u32)D_80000000 & 6) | 0x40) & 0xFF) << 16) |
                      0x04000058;
        command->w1 = (u32)D_80000000;

        command = *commands;
        *commands = command + 1;
        command->w0 = 0x059100A0;
        command->w1 = (u32)D_80000050;

        command = *commands;
        *commands = command + 1;
        command->w1 = 0;
        command->w0 = 0xE7000000;

        finishArg = 0;
        if (state->pulseTimer != 0) {
            overlay27SetMode(commands, 0, 5, 0);

            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = (state->pulseTimer & 0xFF) | 0xFFFF0000;
            finishArg = D_80000118;

            command = *commands;
            *commands = command + 1;
            command->w0 = (((((u32)D_80000118 & 6) | 0x38) & 0xFF) << 16) |
                          0x0400004E;
            command->w1 = (u32)D_80000118;

            command = *commands;
            *commands = command + 1;
            command->w0 = 0x05400050;
            command->w1 = (u32)D_80000160;

            command = *commands;
            *commands = command + 1;
            command->w1 = 0;
            command->w0 = 0xE7000000;
        }

        command = *commands;
        *commands = command + 1;
        command->w1 = 0xFFFFFFFF;
        command->w0 = 0xFA000000;

        command = *commands;
        *commands = command + 1;
        command->w1 = 0xFFFFFF00;
        command->w0 = 0xFB000000;

        overlay27Finish(commands, finishArg);
    }

    oldScale = object->scale;
    if (child != 0) {
        object->y = child->y + (state->scaleTarget * scale);
    }
    object->scale *= scale;
    object->alpha = state->intensity;
    overlay27Finalize(commands, arg1, arg2, object);
    object->scale = oldScale;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o027/overlay_027/func_overlay_027_F0000624_187BFFC.s")
#endif

/* DKR v77/v80 and JFG contain no exact donor for this table transform. */
/* Matched 2026-09-16 (lane s1-a). The last 19 words were the leaf's p2 web
 * numbering: the countdown's post-decrement temp must be numbered ahead of the
 * loop's index expressions, which a `while (remaining--)` condition does (it is
 * created before the body) and a `do ... while (remaining--)` cannot. uopt peels
 * the constant-true first test, so the shape is the same do-while with 9. */
void overlay27UpdateCoordinates(s32 amount) {
    Overlay27CoordinateRecord *record;
    s32 xOffset;
    s32 remaining;

    xOffset = gOverlay27XOffset =
        ((amount * -12) + gOverlay27XOffset) & 0x3FF;
    amount = gOverlay27YOffset =
        ((amount * 48) + gOverlay27YOffset) & 0x3FF;

    record = gOverlay27CoordinateRecords;
    remaining = 10;
    while (remaining--) {
        record->firstX = gOverlay27XCoordinates[record->firstIndex] +
                         xOffset;
        record->firstY = gOverlay27YCoordinates[record->firstIndex] +
                         amount;
        record->secondX = gOverlay27XCoordinates[record->secondIndex] +
                          xOffset;
        record->secondY = gOverlay27YCoordinates[record->secondIndex] +
                          amount;
        record->thirdX = gOverlay27XCoordinates[record->thirdIndex] +
                         xOffset;
        record->thirdY = gOverlay27YCoordinates[record->thirdIndex] +
                         amount;
        record++;
    }
}

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
s32 overlay27CanUse(Overlay27UseObject *object) {
    if (object != NULL) {
        if (object->resource->state != 4 || object->resource->value14 > 0.0f) {
            return 1;
        }
    }
    return 0;
}

/* DKR v77/v80 and JFG contain no exact donor for this state transition. */
s32 overlay27Activate(O27Object *object) {
    O27Object *savedObject;

    if (object != 0 && object->blocked == 0) {
        if (object->state->primaryState == 4) {
            (savedObject = object)->state->primaryState = 3;
            /* Inert allocation aid retained by exact C; tracked in
             * docs/cleanup-queue.md. */
            if (object->state == 0 && object->state == 0) {
            }
            return 1;
        }
        if (object->state->primaryState == 2) {
            object->state->timer = 0;
        }
        return 1;
    }
    return 0;
}


/* PLATEAU-HANDOFF:func_overlay_027_F0000064_187BA3C:start
 * symbol: func_overlay_027_F0000064_187BA3C
 * score: 48 differing words
 * frame: 0x60
 * relocations: 22
 * first-mismatch: +0x94
 * summary: Required call-input carrier reuse is byte-inert; fraction reuse adds structural residual. The real-ABI no-spill source obstacle remains.
 * PLATEAU-HANDOFF:func_overlay_027_F0000064_187BA3C:end
 */

/* PLATEAU-HANDOFF:func_overlay_027_F0000624_187BFFC:start
 * symbol: func_overlay_027_F0000624_187BFFC
 * score: 196 differing words
 * frame: 0x98
 * relocations: 15
 * first-mismatch: +0x8
 * summary: Fresh V0 on b4d1624a reproduces 1016B/254w and the 15-site ambiguity; no new identity or donor evidence; body untouched.
 * PLATEAU-HANDOFF:func_overlay_027_F0000624_187BFFC:end
 */
