#include "overlays/overlay_094.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay94InitializeController(Overlay94Object *object,
                                   const Overlay94Init *init) {
    Overlay94State *state = object->state;
    Overlay94Entity *entity;
    Overlay94SavedValue saved;
    f32 out0;
    f32 out1;
    f32 out2;

    state->current = 0.0f;
    state->selector = init->selector;
    object->kind = init->kind;
    object->scale = init->scale;

    func_8005AD64(object, 0, -1, 0.0f);
    state->command = 0x2000;

    entity = *object->entityRef;
    saved.value = entity->savedValue;
    func_8005ABA8(object, state->current, 0.0f);
    func_8005AF14(entity, saved.value, object);

    lightObject(object, entity, object->renderResource,
                         entity->records[entity->recordIndex]);
    entity->status = 0;

    func_8002B040(entity->records[entity->recordIndex]->queryState,
                        0.0f, 0.0f, -1.0f, &out0, &out1, &out2);
    state->angle = Arctanf(out2, out0);
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this body. */
void overlay94UpdateController(Overlay94Object *object, s32 updateRate) {
    Overlay94State *state;
    Overlay94Entity *entity;
    s32 savedValue;
    f32 rate;
    s16 *command;
    f32 out0;
    f32 out1;
    f32 out2;
    f32 weight;
    s32 angle;

    state = object->state;
    command = &state->command;

    if (o94ControllerValue_o094Reloc != 0 &&
        (func_800254FC(state->selector) & 0x2000) != 0) {
        state->active = 1;
    } else if (o94ControllerValue_o094Reloc == 0) {
        state->active = 0;
    }

    if (state->active != 0) {
        if (state->velocity != 0) {
            state->accumulator +=
                (u32)(s32)state->velocity * (u32)updateRate;
            if (state->velocity > 0) {
                if (state->accumulator >= 0x10000) {
                    state->accumulator = 0;
                    state->velocity = 0;
                }
            } else if (state->velocity < 0 &&
                       state->accumulator < -0xFFFF) {
                state->accumulator = 0;
                state->velocity = 0;
            }

            *command++ = 4;
            *command++ = (s16)state->accumulator;

            weight = func_8002A878(o94MovingDecayBase_o094Reloc, updateRate);
            state->current =
                ((1.0f - weight) * (0.0f - state->current)) + state->current;
        } else if ((func_800254FC(state->selector) & 0x2000) != 0) {
            f32 target;

            target = (f32)func_8002565C(state->selector) / 60.0f;
            if (target > 1.0f) {
                target = 1.0f;
            } else if (target < 0.0f) {
                target = 0.0f;
            }
            target *= o94InputTargetScale_o094Reloc;
            weight = func_8002A878(o94InputDecayBase_o094Reloc, updateRate);
            state->current =
                ((1.0f - weight) * (target - state->current)) + state->current;
        } else {
            weight = func_8002A878(o94NeutralDecayBase_o094Reloc, updateRate);
            state->current =
                ((1.0f - weight) * (0.0f - state->current)) + state->current;
        }
    } else {
        weight = func_8002A878(o94InactiveDecayBase_o094Reloc, updateRate);
        state->current =
            ((1.0f - weight) * (o94InactiveTarget_o094Reloc - state->current)) + state->current;
    }

    *command = 0x2000;
    entity = *object->entityRef;
    savedValue = entity->savedValue;
    rate = (f32)updateRate;

    func_8005ABA8(object, state->current, rate);
    func_8005AF14(entity, savedValue, object);
    lightObject(object, entity, object->renderResource,
                  entity->records[entity->recordIndex]);
    entity->status = 0;

    func_8002B040(entity->records[entity->recordIndex]->queryState,
                  0.0f, 0.0f, -1.0f,
                  &out0, &out1, &out2);
    angle = func_8002A910(out2, out0);

    if (state->active != 0 && state->velocity == 0) {
        s32 reverse;
        s32 delta;

        delta = (angle - state->angle) & 0xFFFF;
        reverse = (state->angle - angle) & 0xFFFF;
        if (reverse < delta) {
            delta = -reverse;
        }

        weight = (f32)delta / rate;
        if (weight < o94NegativeTurnThreshold_o094Reloc || o94PositiveTurnThreshold_o094Reloc < weight) {
            if (weight < 0.0f) {
                state->velocity = (s16)(s32)(state->current * o94NegativeTurnVelocityScale_o094Reloc);
                if (state->velocity < 500) {
                    state->velocity = 500;
                }
            } else if (weight > 0.0f) {
                s32 minimumVelocity;

                state->velocity =
                    (s16)-(s32)(s16)(state->current * o94PositiveTurnVelocityScale_o094Reloc);
                if (state->velocity >= -499) {
                    minimumVelocity = -500;
                    state->velocity = minimumVelocity;
                }
            }
        }
    }

    state->angle = angle;
}

void overlay94SetValue(s32 value) {
    o94ControllerValue_o094Reloc = value;
}
