#include "overlays/overlay_082.h"

/* Overlay 82 unflagged update/accessor tail, consolidated in ROM order. */


































void overlay82Update(O82Object *object, f32 updateRate) {
    O82State *state;
    O82Resource *resource;
    const s16 (*targets)[6];
    const s16 *targetRow;
    s32 *currentValues;
    const s8 *channelMap;
    const s8 *channelValues;
    O82DisplayPair *displayCursor;
    s32 movementEvent;
    s32 edgeEvent;
    s32 shift;
    s32 index;
    s32 value;

    state = object->state;

    if (object->progress > 0.5f) {
        if (state->active == 0) {
            movementEvent = -1;
            edgeEvent = -1;

            if (overlay82InputXReloc < -30) {
                state->changed = 1;
                state->selection--;
                if (state->selection < 0) {
                    state->selection = 0;
                    edgeEvent = 14;
                } else if (state->selection == 3) {
                    state->selection = 4;
                    edgeEvent = 14;
                } else {
                    movementEvent = 0x26D;
                }
            } else if (overlay82InputXReloc >= 31) {
                state->selection++;
                state->changed = 1;
                if (state->selection == 10) {
                    state->selection = 9;
                    edgeEvent = 14;
                } else if (state->selection == 4) {
                    state->selection = 3;
                    edgeEvent = 14;
                } else {
                    movementEvent = 0x26D;
                }
            }

            if (overlay82InputYReloc >= 31) {
                if (state->selection >= 5 && state->selection < 9) {
                    state->selection -= 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            } else if (overlay82InputYReloc < -30) {
                if (state->selection < 4) {
                    state->selection += 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            }

            if (movementEvent != -1) {
                overlay82PlayEventReloc(movementEvent, ((void *)0) );
            }
            if (edgeEvent != -1) {
                overlay82PlayEventReloc(edgeEvent, ((void *)0) );
            }
            if ((overlay82InputButtonsReloc & 0x9000) != 0) {
                state->active = 1;
                overlay82PlayEventReloc(12, ((void *)0) );
            }
        } else if ((overlay82InputButtonsReloc & 0x4000) != 0 && state->disabled == 0) {
            state->active = 0;
            overlay82PlayEventReloc(13, ((void *)0) );
        }
    }

    targets = ((const s16 (*)[6]) (gO82DataBase + 0x00)) ;
    if (object->progress > 0.75f) {
        resource = object->resourceOwner->resource;
        targetRow = (const s16 *)targets + state->selection * 6;
        shift = state->changed != 0 ? 2 : 3;
        for (index = 0; index < 6; index++) {
            state->values[index] += (((s32)targetRow[index] << 16) - state->values[index]) >> shift;
        }

        channelMap = ((const s8 *) (gO82DataBase + 0x78)) ;
        index = 0;
        do {
            if (index == state->selection) {
                overlay82SetChannelReloc(resource, *channelMap, 0x100);
            } else {
                overlay82SetChannelReloc(resource, *channelMap, 0);
            }
            index++;
            channelMap++;
        } while (index != 10);

        channelValues = ((const s8 *) (gO82DataBase + 0x84)) ;
        if (state->active != 0) {
            overlay82SetChannelReloc(resource, 6,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
            overlay82SetChannelReloc(resource, 7,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
        } else {
            overlay82SetChannelReloc(resource, 6,
                *(const s8 *)((u32)channelValues + state->selection) << 8);
            overlay82SetChannelReloc(resource, 7,
                *(const s8 *)((u32)channelValues + state->selection) << 8);
        }
    }

    displayCursor = state->display;
    currentValues = state->values;
    for (index = 0; index < 6; index++) {
        displayCursor->label = 14 + index * 3; displayCursor->value = state->values[index] >> 16;
        displayCursor++;
        currentValues++;
    }
    displayCursor->label = 0x2000;
}

/* DKR v77/v80 semantic-source search: negative (generic field access only). */
s32 overlay82GetSelection(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    return state->selection;
}

u32 overlay82IsActive(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    return state->active;
}

void overlay82Disable(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    state->disabled = 1;
}

void overlay82Enable(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    state->disabled = 0;
}
