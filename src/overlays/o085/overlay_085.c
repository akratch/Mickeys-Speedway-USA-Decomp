#include "overlays/overlay_085.h"

/*
 * Overlay 85, ADR 0006 consolidation: one translation unit for the whole
 * module (overlay85Configure at +0x000, overlay85Update at +0x0C0).
 */

void overlay85Configure(Overlay85State *state, Overlay85Config *config) {
    f32 scale;
    scale = ((s32)config->scale) & 0xFF;
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale *= 0.015625f;
    state->scale = state->resource->scale * scale;
    if (state->outputScale != NULL) {
        state->outputScale[0] = state->resource->outputScaleX * scale;
        state->outputScale[1] = state->resource->outputScaleY * scale;
    }
    state->frame = config->frame;
    state->angle = (((s32)config->angle) & 0xFF) << 10;
    if (state->frame >= state->resource->frameCount) {
        state->frame = 0;
    }
    *(s32 *)&state->timer = 0;
    state->unk88 = 0;
    if (state->output != NULL) {
        state->output->state = 2;
    }
}

void overlay85Update(Overlay85Object *object, s32 step) {
    volatile s32 reservation;
    Overlay85Trigger *trigger;
    s16 *countdown;
    Overlay85Timer *timer;
    s32 currentTimer;
    s16 type;

    trigger = object->trigger;
    if (trigger != NULL) {
        countdown = &object->timer.timer;
        if (object->timer.timer > 0) {
            *countdown -= step;
        }
        timer = &object->timer;
        currentTimer = *(volatile s16 *)countdown;
        if (currentTimer == 0 && trigger->active != 0) {
            if (object->type == 2) {
                amSndPlayXYZ(0x208, object->x, object->y, object->z, 4,
                                    0);
            } else {
                amSndPlayXYZ(0x1F, object->x, object->y, object->z, 4,
                                    0);
            }
            timer->value = trigger->strength * 182.0f;
            timer->timer = 10;
            type = object->type;
            if (type == 2) {
                func_8003EDD4(object->effectValue);
                partAdjustScaling(object->effectValue);
                object->mode = 1;
                partUpdateTriggers(object, step);
            } else if (type == 0xF0) {
                func_8003EDD4(object->effectValue);
                object->mode = 0x1F;
                partUpdateTriggers(object, 2);
            } else if (type == 0x107) {
                object->mode = 1;
                partUpdateTriggers(object, step);
            }
        } else if (currentTimer <= 0) {
            timer->timer = 0;
        }
        object->value4 = timer->value;
        object->value2 = timer->value;
        timer->value = (timer->value * -200) >> 8;
    }
    if (object->state != NULL) {
        object->state->active = 1;
    }
}
