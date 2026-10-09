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

/* Matched 2026-10-01. The 1/255 scale is written as a literal at each use, so
 * uopt hoists it and reloads it after each call from its own pool entry; the
 * earlier candidate carried it in a declared float, which took an argument
 * register and hid that web. The intensity and fade fields are read directly
 * where the earlier candidate copied them into shared scalars, which keeps
 * each reload on the field's own web. The source state and the volume are two
 * locals rather than one union, which lands the frame homes. */
void func_overlay_027_F0000064_187BA3C(O27Object *object, s32 updateRate) {
    s32 pulseStep;
    O27State *sourceState;
    O27State *state;
    s32 initialPhase;
    s32 phase;
    s32 value;
    O27Object *source;
    s32 volume;
    f32 fraction;

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
        do {
            switch (state->primaryState) {
                case 0:
                    state->timer += updateRate;
                    fraction = 1.0f - func_8002A878(0.9625f, updateRate);
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
                        1.0f + ((f32)state->intensity * 0.003921568f);
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
                        updateRate = 0;
                        if (state->intensity >= 0x100) {
                            state->intensity = 0xFF;
                            object->scale = 2.0f;
                        } else {
                            object->scale =
                                1.0f + ((f32)state->intensity * 0.003921568f);
                        }
                    } else {
                        state->fade += updateRate * 4;
                        updateRate = 0;
                        if (state->fade >= 0xFF) {
                            state->primaryState = 2;
                            state->timer = 0;
                            state->fade = 0xFF;
                            state->fadeFloat = 1.0f;
                        } else {
                            state->fadeFloat =
                                (f32)state->fade * 0.003921568f;
                        }
                    }
                    break;

                default:
                    if (state->fade >= updateRate * 8) {
                        state->fade -= updateRate * 8;
                        updateRate = 0;
                        state->fadeFloat =
                            (f32)state->fade * 0.003921568f;
                    } else {
                        state->intensity -= updateRate * 4;
                        state->fade = 0;
                        updateRate = 0;
                        state->fadeFloat = 0.0f;
                        if (state->intensity <= 0) {
                            func_80006EA0(object);
                        } else {
                            object->scale = 1.0f +
                                ((f32)state->intensity * 0.003921568f);
                        }
                    }
                    break;
            }
        } while (updateRate != 0);
    }

    if (state->fade == 0) {
        return;
    }

    if (state->pulseState == 0) {
        if (state->fade == 0xFF && func_800299E8(0, 0x1FFF) >= 0x1FD7) {
            sourceState = source->state;
            state->pulseState = 1;
            if (state->secondaryHandle != 0) {
                amSndStopXYZ(state->secondaryHandle);
            }
            amSndPlayXYZ(0x1BB, object->x, object->y, object->z, 4,
                          &state->secondaryHandle);
            if (!(sourceState->flags1A8 & 1)) {
                func_8002BD58(*(s8 *)&sourceState->primaryState,
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
        amSndPlayXYZ(0x1B8, object->x, object->y, object->z, 1,
                      &state->primaryHandle);
    }
    volume = state->fade >> 1;
    if (volume >= 0x80) {
        volume = 0x7F;
    }
    if (state->primaryHandle != 0) {
        amSndSetXYZ(state->primaryHandle, object->x, object->y, object->z);
        amSndSetVolXYZ(state->primaryHandle, volume);
    }
    if (state->secondaryHandle != 0) {
        amSndSetXYZ(state->secondaryHandle, object->x, object->y, object->z);
        amSndSetVolXYZ(state->secondaryHandle, volume);
    }
}

/* Matched 2026-10-05. Each display-list word is its own pointer temporary:
 * a shared command local leaves both vertex addresses one register too low.
 * Those temporaries are the frame. camGetPtr's first halfword is the angle
 * this routine negates; the child scale and the closing submit are the
 * resident object helpers at those two call sites. */
#define O27_WRITE_COMMAND(word0, word1) \
    { O27Command *_g = (*commands)++; _g->w0 = (word0); _g->w1 = (word1); }

extern s16 *camGetPtr(void);
extern f32 func_80009F08(void *arg);
extern void camPushModelMtx(O27Command **commands, void *mtx,
                            O27Transform *transform, f32 scale, f32 scaleY);
extern void func_800349A4(O27Command **commands, void *texture, s32 flags,
                          s32 frame);
extern void camPopModelMtx(O27Command **commands);
extern void func_80009E78(O27Command **commands, void *mtx, s16 *vertices,
                          O27Object *object);

void func_overlay_027_F0000624_187BFFC(O27Command **commands, void *mtx,
                                       s16 *vertices, O27Object *object) {
    s32 intensity;
    f32 scale;
    f32 oldScale;
    O27Transform transform;
    void *displayList;
    O27Child *child;
    O27State *state;
    s16 *value;
    u8 *verts;

    value = camGetPtr();
    state = object->state;
    child = (O27Child *)state->source;
    if (child != 0) {
        scale = func_80009F08(child);
    } else {
        scale = 1.0f;
    }

    if (state->fade != 0) {
        if (child != 0) {
            transform.positionX = child->x;
            transform.positionY = child->y;
            transform.positionZ = child->z;
        } else {
            transform.positionX = object->x;
            transform.positionY = object->y;
            transform.positionZ = object->z;
        }

        transform.x = -*value;
        transform.y = 0;
        transform.z = 0;
        transform.scale = scale;
        transform.positionY += 24.0f * scale;

        if (child != 0 && child->factor != 0) {
            intensity = (s32)(*child->factor * 256.0f);
        } else {
            intensity = 0x100;
        }

        displayList = *object->renderResource->displayList;
        camPushModelMtx(commands, mtx, &transform, 1.0f, 0.0f);
        func_800349A4(commands, displayList, 0x214, 0);

        O27_WRITE_COMMAND(0xFA000000, ((((intensity * 0x60) >> 8) & 0xFF) << 24) | ((((intensity * 0xE0) >> 8) & 0xFF) << 16) | ((((intensity * 0xFF) >> 8) & 0xFF) << 8) | (state->fade & 0xFF));

        O27_WRITE_COMMAND(0xFB000000, ((((intensity << 7) >> 8) & 0xFF) << 8) | 0xFF);

        verts = D_80000000;
        O27_WRITE_COMMAND((0x04 << 24) | (((0x40 | ((u32)verts & 6)) & 0xFF) << 16) | 0x58, (u32)verts);

        O27_WRITE_COMMAND(0x059100A0, (u32)D_80000050);

        O27_WRITE_COMMAND(0xE7000000, 0);

        if (state->pulseTimer != 0) {
            func_800349A4(commands, NULL, 5, 0);

            O27_WRITE_COMMAND(0xFA000000, (state->pulseTimer & 0xFF) | 0xFFFF0000);
            verts = D_80000118;

            O27_WRITE_COMMAND((0x04 << 24) | (((0x38 | ((u32)verts & 6)) & 0xFF) << 16) | 0x4E, (u32)verts);

            O27_WRITE_COMMAND(0x05400050, (u32)D_80000160);

            O27_WRITE_COMMAND(0xE7000000, 0);
        }

        O27_WRITE_COMMAND(0xFA000000, 0xFFFFFFFF);

        O27_WRITE_COMMAND(0xFB000000, 0xFFFFFF00);

        camPopModelMtx(commands);
    }

    oldScale = object->scale;
    if (child != 0) {
        object->y = child->y + (state->scaleTarget * scale);
    }
    object->scale *= scale;
    object->alpha = state->intensity;
    func_80009E78(commands, mtx, vertices, object);
    object->scale = oldScale;
}

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
    if (object != NULL && object->blocked == 0) {
        if (object->state->primaryState == 4) {
            object->state->primaryState = 3;
        } else if (object->state->primaryState == 2) {
            object->state->timer = 0;
        }
        return 1;
    }
    return 0;
}



