#include "overlays/overlay_025.h"

/* Exact 95-word match. Reading the retained owner through state reproduces the
 * shipped s0/s1 carrier allocation; -Wab,-r4300_mul supplies its FP hazard nop. */
void overlay25InitializeEffect(Overlay25Object *object,
                               const Overlay25Init *init) {
    Overlay25InitState *state;
    s32 combinedAngle;
    s32 paletteIndex;

    state = &object->state->init;
    state->lifetime = 600;
    state->duration = 60;
    state->currentValue = 0.0f;

    state->owner = init->owner;

    if (init->useOwner != 0 && state->owner != NULL) {
        Overlay25OwnerState *ownerState;

        ownerState = state->owner->state;
        combinedAngle = ownerState->baseAngle + ownerState->relativeAngle;

        state->activeDuration = 60;
        state->velocityX =
            (func_8002A8C0(combinedAngle) * ownerState->scale) +
            (func_8002A8C0(ownerState->baseAngle) * -32.0f);
        state->lift = 16.0f;
        state->velocityZ =
            (func_8002A8BC(combinedAngle) * ownerState->scale) +
            (func_8002A8BC(ownerState->baseAngle) * -32.0f);
    } else {
        state->activeDuration = 0;
        object->flags |= 0x0800;
    }

    if (gOverlay25GlobalFlagsReloc & 0x10) {
        paletteIndex = func_800299E8(0, 7) * 3;
    } else {
        paletteIndex = 9;
    }
    state->color[0] = gOverlay25ColorsReloc[paletteIndex + 0];
    state->color[1] = gOverlay25ColorsReloc[paletteIndex + 1];
    state->color[2] = gOverlay25ColorsReloc[paletteIndex + 2];

    if (object->vector != NULL) {
        object->vector->x = 0.0f;
        object->vector->y = 0.0f;
    }
}

/* 2026-10-02 rewrite from the listing (lane x-ovla), 74 -> 19 masked at
 * delta 0: no m2c carriers; the movement loop as `steps = updateRate - 1;
 * while (steps--)` over the state fields; `radius = 4` (an int literal, so
 * the 4.0f the else arm compares and stores is a separate constant and the
 * radius takes a ring temporary); radius declared before position; the
 * lifetime decrement before the duration one; the hit loop as
 * `while (count--)`; and objects[6] declared last, after the four hit-loop
 * locals whose cells sit between position and the array. The height
 * difference in the hit loop reuses `y`, dead in that arm, which puts it in
 * the movement accumulator's f2 (19 -> 16); `delta` keeps its cell. Open:
 * the hit loop's index and its strength-reduced cursor take s3/s4 the wrong
 * way round (13 words), and one as1 slot order (3). */
#ifdef NON_MATCHING
void overlay25UpdateEffect(Overlay25Object *object, s32 updateRate) {
    void *unused;
    s32 hitSomething;
    Overlay25EffectState *state;
    f32 x;
    f32 y;
    f32 z;
    s32 steps;
    f32 radius;
    Overlay25Vector position;
    s32 count;
    Overlay25Object *other;
    f32 delta;
    Overlay25EntityState *otherState;
    Overlay25Object *objects[6];

    state = &object->state->effect;
    if (state->activeDuration != 0) {
        state->activeDuration -= updateRate;
        object->value = object->transform->value * 2.0f;
        if (state->activeDuration <= 0) {
            overlay25DestroyReloc(object);
            return;
        }
        y = state->lift;
        x = state->velocityX;
        z = state->velocityZ;
        state->lift -= 1.1034483f;
        steps = updateRate - 1;
        while (steps--) {
            x += state->velocityX;
            y += state->lift;
            z += state->velocityZ;
            state->lift -= 1.1034483f;
        }
        position.x = object->x;
        position.y = object->y;
        position.z = *(f32 *)&object->z;
        overlay25MoveReloc(object, x, y, z);
        radius = 4;
        overlay25SweepReloc(1, &position, (Overlay25Vector *)&object->x, &radius, 0, 0);
        if (overlay25TraceReloc(&position, (Overlay25Vector *)&object->x, radius, object,
                                overlay25SetVectorFlagsReloc)) {
            if (state->flags & 4) {
                overlay25DestroyReloc(object);
                return;
            }
            state->activeDuration = 0;
            object->value = object->transform->value;
            state->multiplier = 0.4f;
            object->flags |= 0x800;
        }
    } else {
        state->lifetime -= updateRate;
        state->duration -= updateRate;
        if (state->duration < 0) {
            state->duration = 0;
        }
        if (state->lifetime <= 0) {
            updateRate = -state->lifetime;
            state->lifetime = 0;
            state->multiplier -= 0.4f * updateRate;
            if (state->multiplier <= 0.1f) {
                overlay25DestroyReloc(object);
                return;
            }
        } else {
            state->multiplier += 0.4f * updateRate;
            if (state->multiplier > 4.0f) {
                state->multiplier = 4.0f;
            }
            count = overlay25QueryObjectsReloc(object->x, object->y, object->z,
                                               object->value * state->multiplier * 16.0f,
                                               1, objects);
            hitSomething = 0;
            while (count--) {
                other = objects[count];
                y = other->y - object->y;
                if ((other != state->owner) || (state->duration == 0)) {
                    otherState = &other->state->entity;
                    if ((otherState->height < -5.0f) && (y > -24.0f) &&
                        (y < 24.0f) && (otherState->enabled != 0)) {
                        hitSomething = 1;
                        if (overlay25CanHitReloc(other, otherState)) {
                            overlay7DispatchModesReloc(state->owner, other);
                            state->owner->state->entity.ownerHitCount++;
                            otherState->selfHitCount++;
                            if (overlay25GetStatusReloc()->type == 5) {
                                overlay25NotifyHitReloc(other);
                            }
                        }
                    }
                }
            }
            if (hitSomething) {
                state->lifetime = 0;
            }
        }
    }

    if (object->vector != NULL) {
        object->vector->x = state->multiplier * object->transform->scaleX;
        object->vector->y = state->multiplier * object->transform->scaleY;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay_025/func_overlay_025_F000017C_1879E04.s")
#endif

/* No corresponding DKR/JFG source or object match was found. */
void overlay25SetVectorFlags(s32 unused0, Overlay25Vector *out, s32 unused2,
                             s32 unused3, Overlay25Source *source,
                             Overlay25VectorObject *object) {
    Overlay25VectorState *state;

    state = object->state;
    out->x = source->vector.x;
    out->y = source->vector.y;
    out->z = source->vector.z;
    if ((gOverlay25Threshold < source->value) || (source->flags & 0x10000000)) {
        state->flags |= 2;
        return;
    }
    state->flags |= 4;
}

/* PLATEAU-HANDOFF:overlay25UpdateEffect:start
 * symbol: overlay25UpdateEffect
 * score: 16/259 words
 * frame: 0xA0
 * relocations: 25
 * first-mismatch: +0x2A0
 * summary: Height difference reuses dead y (f2): 19 to 16. Open: hit-loop index (tot 32) outranks its cursor (tot 31) for s3 (13 words); one as1 slot (3).
 * PLATEAU-HANDOFF:overlay25UpdateEffect:end
 */
