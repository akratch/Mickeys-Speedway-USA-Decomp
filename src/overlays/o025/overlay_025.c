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

/*
 * 2026-09-24: else-arm 12-byte hit struct (other, delta, otherState) is
 * declared before objects[6]. Frame stays 0xA0 and the array homes at
 * 0x4C with the target. Masked 74; the naming residual is unchanged.
 */
/* Ownership trial (2026-08-28): fixed the TU's +0x20..+0x40 .rodata range;
 * linked promotion is text-differs with 386 in-range words, first at +0x0.
 * Module growth is cleared; the remaining gap is codegen/register allocation. */
#ifdef NON_MATCHING
void overlay25UpdateEffect(Overlay25Object *object, s32 updateRate) {
    void *unused; /* L99: declared first so hitSomething homes at 0x98 */
    s32 hitSomething;
    Overlay25EffectState *state;

    state = &object->state->effect;
    if (state->activeDuration != 0) {
        s32 remaining;
        f32 accum;
        f32 moveX;
        f32 moveZ;
        f32 radius;
        Overlay25Vector position;

        state->activeDuration -= updateRate;
        object->value = 2.0f * object->transform->value;
        remaining = updateRate - 2;
        if (state->activeDuration <= 0) {
            overlay25DestroyReloc(object);
            return;
        }

        accum = state->lift;
        moveX = state->velocityX;
        moveZ = state->velocityZ;
        state->lift = accum - 1.1034483f;
        if (updateRate - 1) {
            do {
                f32 current = state->lift;
                moveX += state->velocityX;
                state->lift = current - 1.1034483f;
                moveZ += state->velocityZ;
                accum += current;
            } while (remaining--);
        }

        position.x = object->x;
        position.y = object->y;
        position.z = *(f32 *)&object->z;
        overlay25MoveReloc(object, moveX, accum, moveZ);

        radius = 4.0f;
        overlay25SweepReloc(1, &position, (Overlay25Vector *)&object->x,
                            &radius, 0, 0);
        if (overlay25TraceReloc(&position, (Overlay25Vector *)&object->x,
                                radius, object, overlay25SetVectorFlagsReloc)) {
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
        struct {
            Overlay25Object *other;
            f32 delta;
            Overlay25EntityState *otherState;
        } hit;
        Overlay25Object *objects[6];
        s32 count;
        s32 index;

        state->duration -= updateRate;
        state->lifetime -= updateRate;
        if (state->duration < 0) {
            state->duration = 0;
        }

        if (state->lifetime <= 0) {
            updateRate = -state->lifetime;
            state->lifetime = 0;
            state->multiplier -= 0.4f * (f32)updateRate;
            if (state->multiplier <= 0.1f) {
                overlay25DestroyReloc(object);
                return;
            }
        } else {
            state->multiplier += 0.4f * (f32)updateRate;
            if (state->multiplier > 4.0f) {
                state->multiplier = 4.0f;
            }

            count = overlay25QueryObjectsReloc(
                object->x, object->y, object->z,
                object->value * state->multiplier * 16.0f, 1, objects);
            hitSomething = 0;
            if (count != 0) {
                index = count - 1;
                do {
                    hit.other = objects[index];
                    hit.delta = hit.other->y - object->y;

                    if ((hit.other != state->owner) || (state->duration == 0)) {
                        hit.otherState = &hit.other->state->entity;
                        if ((hit.otherState->height < -5.0f) &&
                            (hit.delta > -24.0f) && (hit.delta < 24.0f) &&
                            (hit.otherState->enabled != 0)) {
                            hitSomething = 1;
                            if (overlay25CanHitReloc(hit.other, hit.otherState)) {
                                overlay7DispatchModesReloc(state->owner, hit.other);
                                state->owner->state->entity.ownerHitCount++;
                                hit.otherState->selfHitCount++;
                                if (overlay25GetStatusReloc()->type == 5) {
                                    overlay25NotifyHitReloc(hit.other);
                                }
                            }
                        }
                    }
                } while (index--);
            }
            if (hitSomething != 0) {
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
 * score: 74/259 words
 * frame: 0xA0
 * relocations: 25
 * first-mismatch: +0x3C
 * summary: hypothesis=source-authentic save-ratio so web 122 follows web 129, or 12 bytes of else-arm home; spellings=post-loop use 106 at +24, second post-loop use 103 at +8, hit struct kept at 74 and delta 0; stall=the save-ratio uses spilled and s4 is unproved, while objects homes at +0x4C
 * PLATEAU-HANDOFF:overlay25UpdateEffect:end
 */
