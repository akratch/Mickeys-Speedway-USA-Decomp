#include "overlays/overlay_082.h"

/* Tier B: ORT export 1910 identifies the target table. Its ten six-value
 * rows follow this module's signed-halfword reads and selection range. */
const s16 D_overlay_082_F00004D0_18CF650[10][6] = {
    { 26304, -23232, -32448, 19200, 29376, -9024 },
    { 24192, -21120, -31488, 16896, 28416, -8768 },
    { 21632, -17792, -31488, 17792, 24704, -8768 },
    { 19072, -20864, -23936, 17920, 21376, -10048 },
    { 30976, -27648, 32128, 21888, 27904, -11968 },
    { 28288, -27008, 32512, 20992, 27264, -6208 },
    { 26752, -24704, -31616, 19456, 23296, -6208 },
    { 24448, -21248, -30208, 16000, 19968, -1600 },
    { 21120, -20736, -27008, 14976, 19840, -960 },
    { 17152, -18048, -23936, 14336, 18560, -3392 },
};

/* Tier D: bounded serialized view of initialized bytes 0x78..0xA0.
 * Ten channel/value entries are used. Each pair of following bytes and
 * the final 0x90..0xA0 range are reserved with no observed consumer.
 * The module image bounds this view; the original C declaration is unknown. */
typedef struct Overlay82ChannelLayout {
    s8 channels[12];
    s8 values[12];
    u8 reserved90[16];
} Overlay82ChannelLayout;

static const Overlay82ChannelLayout sChannelLayout = {
    { 17, 23, 3, 24, 1, 18, 27, 12, 25, 26, 0, 0 },
    { 1, 2, 3, 4, 0, 9, 8, 7, 6, 5, 0, 0 },
    { 0 },
};

void overlay82Init(Overlay82Object *object, void *entry) {
    Overlay82State *state;
    s32 index;

    state = object->state;
    state->selection = 0;
    state->changed = 0;
    state->active = 0;
    state->disabled = 0;
    for (index = 0; index < 6; index++) {
        state->values[index] = 0;
    }
}

void overlay82Update(O82Object *object, s32 updateRate) {
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

            if (D_800D31BC < -30) {
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
            } else if (D_800D31BC >= 31) {
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

            if (D_800D31BE >= 31) {
                if (state->selection >= 5 && state->selection < 9) {
                    state->selection -= 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            } else if (D_800D31BE < -30) {
                if (state->selection < 4) {
                    state->selection += 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            }

            if (movementEvent != -1) {
                amSndPlay(movementEvent, ((void *)0) );
            }
            if (edgeEvent != -1) {
                amSndPlay(edgeEvent, ((void *)0) );
            }
            if ((D_800D31B8 & 0x9000) != 0) {
                state->active = 1;
                amSndPlay(12, ((void *)0) );
            }
        } else if ((D_800D31B8 & 0x4000) != 0 && state->disabled == 0) {
            state->active = 0;
            amSndPlay(13, ((void *)0) );
        }
    }

    targets = D_overlay_082_F00004D0_18CF650;
    if (object->progress > 0.75f) {
        resource = object->resourceOwner->resource;
        targetRow = (const s16 *)targets + state->selection * 6;
        shift = state->changed != 0 ? 2 : 3;
        for (index = 0; index < 6; index++) {
            state->values[index] += (((s32)targetRow[index] << 16) - state->values[index]) >> shift;
        }

        channelMap = sChannelLayout.channels;
        index = 0;
        do {
            if (index == state->selection) {
                modSetTextureFrame(resource, *channelMap, 0x100);
            } else {
                modSetTextureFrame(resource, *channelMap, 0);
            }
            index++;
            channelMap++;
        } while (index != 10);

        channelValues = sChannelLayout.values;
        if (state->active != 0) {
            modSetTextureFrame(resource, 6,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
            modSetTextureFrame(resource, 7,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
        } else {
            modSetTextureFrame(resource, 6,
                *(const s8 *)((u32)channelValues + state->selection) << 8);
            modSetTextureFrame(resource, 7,
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
