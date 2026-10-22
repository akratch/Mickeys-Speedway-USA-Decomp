#include "overlays/overlay_081.h"

/*
 * Overlay 81, ADR 0006 consolidation: one translation unit in ROM order.
 * The canonical layouts combine the compatible partial struct views from
 * the former per-function files; the small radius/index state remains a
 * distinct Overlay81NearbyState because it is genuinely different.
 */

void overlay81Init(Overlay81Object *object, Overlay81Init *init, s32 unused) {
    Overlay81State *state;
    Overlay81Collision *collision;
    s32 bit;

    object->scale = (f32) init->valueA.scale * gOverlay81Scale;
    state = object->state;
    if (object->dimensions != 0) {
        object->dimensions[0] = object->transform->width * object->scale;
        object->dimensions[1] = object->transform->depth * object->scale;
    }
    object->scale *= object->transform->scale;
    collision = object->collision;
    collision->value.radius *= object->scale;
    state->x = object->x.value;
    state->y = object->y.value;
    state->z = object->z.value;
    state->timer = init->valueC.timer;
    bit = init->bit;
    if (bit != -1) {
        state->mask = 1 << bit;
    }
    collision = object->collision;
    collision->flags |= 2;
}

/* Trigger/update path; exact DKR and JFG scans are negative. */
void overlay81Update(Overlay81Object *object, s32 updateRate) {
    Overlay81State *state;
    f32 savedY;
    Overlay81Collision *collision;
    u8 active;

    state = object->state;
    if (state->mask & gOverlay81Mask) {
        if (state->timer != 0) {
            state->timer -= updateRate;
            if (state->timer <= 0) {
                state->timer = 0;
                collision = object->collision;
                collision->flags &= ~2;
            }
        }

        active = object->trigger->active;
        if ((active != 0) && (state->active == 0)) {
            amSndPlayXYZ(0x276, object->x.bits, object->y.bits,
                                      object->z.bits, 4, 0);
            active = object->trigger->active;
        } else if (active == 0) {
            collision = object->collision;
            if (collision->active != 0) {
                savedY = object->y.value;
                object->y.value -= collision->value.yOffset;
                object->flags |= 1;
                partUpdateTriggers(object, 1);
                amSndPlayXYZ(
                    mathRnd(0x22A, 0x22B) & 0xFFFF,
                    object->x.bits, object->y.bits, object->z.bits, 4, 0);
                object->y.value = savedY;
                active = object->trigger->active;
            }
        }
        if (active != 0) {
            state->active = 1;
        } else {
            state->active = 0;
        }
    }
}

/* Small state helpers; the exact DKR and JFG donor scans are negative. */
void overlay81SetMaskBit(s32 bit) {
    gOverlay81Mask |= 1 << bit;
}

void overlay81InitState(Overlay81Object *object, Overlay81Init *init) {
    Overlay81NearbyState *state = object->state;

    state->radius = init->valueA.radius;
    state->index = init->valueC.index;
}

/* Radius query; exact DKR and JFG scans are negative. */
void overlay81CheckNearby(Overlay81Object *object, s32 unused) {
    Overlay81NearbyState *state;
    Overlay81NearbyObject **nearby;
    Overlay81NearbyObject *candidate;
    f32 radiusSquared;
    f32 deltaX;
    f32 deltaZ;
    u8 found;
    s32 count;
    volatile u64 scratch;

    state = object->state;
    radiusSquared = state->radius;
    radiusSquared *= radiusSquared;
    nearby = func_80005750(&count, object, state);
    found = 0;
    if (count != 0) {
        do {
            candidate = *nearby++;
            deltaX = candidate->x - object->x.value;
            deltaZ = candidate->z - object->z.value;
            if (((deltaX * deltaX) + (deltaZ * deltaZ)) < radiusSquared) {
                found = 1;
            }
            count = count - 1;
        } while (count != 0);
    }
    if (found != 0) {
        gOverlay81Mask |= 1 << state->index;
    }
}
