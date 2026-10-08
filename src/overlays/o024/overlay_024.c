#include "overlays/overlay_024.h"

#define O24_SHIFTL(value, shift, width) \
    ((u32)(((u32)(value) & ((1U << (width)) - 1U)) << (shift)))

/*
 * Overlay 24, ADR 0006 consolidation: one translation unit in ROM order.
 * Exact DKR v77/v80 and JFG scans are negative for the module's three
 * functions.
 */

void overlay24Init(Overlay24Object *object, Overlay24InitData *init) {
    Overlay24State *state;

    state = object->state;
    state->mode = 0;
    state->remaining = init->remaining;
    state->target = init->target;
}

void overlay24Update(Overlay24Object *object, s32 updateRate) {
    Overlay24State *state;
    s32 eventId;
    s32 integrationTicks;
    Overlay24Target *target;
    Overlay24TargetState *targetState;

    state = object->state;
    target = state->target;

    if (target == 0) {
        overlay24QueueCleanupReloc(object);
    } else {
        targetState = target->state;
        eventId = 9;
        overlay24UpdateRelationReloc(*object->relationResource, &eventId, 20,
                                     &object->relationValue, updateRate);

        integrationTicks = 0;
        while (updateRate != 0) {
            switch (state->mode) {
            case 0:
                state->mode = 1;
                state->phaseTicks = 0;
                state->progress = 0;
                state->height = 16.0f;
                state->velocity = 4.5f;
                break;

            case 1:
                if (integrationTicks == 0) {
                    integrationTicks = updateRate;
                }

                state->phaseTicks =
                    (s16)((u32)(s32)state->phaseTicks + (u32)updateRate);
                state->progress =
                    (s32)((u32)state->progress + ((u32)updateRate << 5));
                updateRate = 0;

                if (state->progress >= 257) {
                    state->progress = 256;
                }

                if (state->phaseTicks >= 15) {
                    if (((targetState->flags & 1) == 0) ||
                        (targetState->status == 0)) {
                        overlay24EmitEventReloc(9, 0);

                        if (((u16)gOverlay24InputFlagsReloc & 2) != 0) {
                            if (overlay24GetInputStateReloc()->mode != 1) {
                                if (targetState->adjustment > 0) {
                                    targetState->adjustment =
                                        (u8)(targetState->adjustment - 1);
                                }
                                goto adjustment_done;
                            }
                        }
                        if (targetState->adjustment < 20) {
                            targetState->adjustment =
                                (u8)(targetState->adjustment + 1);
                        }
                    adjustment_done:
                        ;
                    }

                    state->mode = 2;
                    state->remaining = (s8)((s32)state->remaining - 1);
                    updateRate = *(volatile s16 *)&state->phaseTicks;
                    updateRate -= 15;
                }
                break;

            default:
                if (integrationTicks == 0) {
                    integrationTicks = updateRate;
                }

                state->progress =
                    (s32)((u32)state->progress - ((u32)updateRate << 5));
                updateRate = 0;

                if (state->progress < 0) {
                    integrationTicks = 0;
                    if (state->remaining > 0) {
                        state->mode = 0;
                    } else {
                        overlay24QueueCleanupReloc(object);
                    }
                    state->progress = 0;
                }
                break;
            }
        }

        while (integrationTicks--) {
            state->height = state->height + state->velocity;
            state->velocity = state->velocity - 0.3f;
        }

        object->x = target->x;
        object->y = target->y;
        object->z = target->z;
        object->copiedValue = target->copiedValue;
    }
}

void overlay24RenderState(Overlay24Command **commands, void *mtx, void *vertices,
                          Overlay24RenderObject *object) {
    s32 opacity;
    s32 alpha;
    Overlay24RenderState *state;
    Overlay24Source *source;
    Overlay24Command **renderCommands;

    state = (opacity = 0xFF, object->state);
    if (state->enabled == 0) {
        return;
    }

    source = state->source;
    renderCommands = commands;
    if (source == NULL) {
        return;
    }

    object->x = source->x;
    object->y = source->y + state->yOffset;
    object->z = source->z;

    if (source->opacity != NULL) {
        opacity = *source->opacity * 255.0f;
    } else {
        opacity = 0xFF;
    }
    if (gOverlay24FadeActiveReloc != 0) {
        opacity = (opacity * gOverlay24FadeScaleReloc) >> 8;
    }

    alpha = (object->opacity * state->enabled) >> 8;

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = 0;
        command->w0 = O24_SHIFTL(0xE7, 24, 8);
    }

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFA, 24, 8),
                       ((opacity & 0xFF) << 24) |
                       ((opacity & 0xFF) << 16) |
                       ((opacity & 0xFF) << 8) |
                       (alpha & 0xFF));
    }

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFB, 24, 8),
                       O24_SHIFTL(0xFF, 24, 8) |
                       O24_SHIFTL(0xFF, 16, 8) |
                       O24_SHIFTL(0xFF, 8, 8) |
                       O24_SHIFTL(0, 0, 8));
    }

    overlay24RenderHelperReloc(renderCommands, mtx, vertices, object,
                               *object->resource, 0xC, 0xFF);

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFA, 24, 8),
                       O24_SHIFTL(0xFF, 24, 8) |
                       O24_SHIFTL(0xFF, 16, 8) |
                       O24_SHIFTL(0xFF, 8, 8) |
                       O24_SHIFTL(0xFF, 0, 8));
    }
}
