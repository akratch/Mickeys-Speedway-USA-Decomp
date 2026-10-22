#include "PR/ultratypes.h"

typedef struct Overlay86VecOutput {
    u8 pad00[4];
    f32 x;
    f32 y;
    f32 z;
} Overlay86VecOutput;

typedef struct Overlay86RingRecord {
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
    u16 pad0E;
    u16 flags;
} Overlay86RingRecord;

typedef struct Overlay86Node {
    s16 angle;
    u8 pad02[0x0A];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x2C];
    s16 kind;
    u8 pad46[0x3E];
    s32 ownerIndex;
} Overlay86Node;

typedef struct Overlay86State {
    s8 ownerIndex;
    u8 pad001[0x37B];
    s16 selectedIndex;
    u8 pad37E[0x62];
    Overlay86RingRecord *record;
} Overlay86State;

typedef struct Overlay86Object {
    u8 pad00[0x64];
    Overlay86State *state;
} Overlay86Object;

extern Overlay86RingRecord *overlay1FindPreviousUsable(
    s32 index, s32 *selectedIndex);
extern s32 overlay1NextIndex(s32 index);
extern Overlay86Node **func_8000572C(s32 *start, s32 *end);

/*
 * Near-miss exactness note: IDO assigns the five homed locals in reverse
 * declaration order here; keeping the owner comparison in node-first order
 * also preserves the target's two-load schedule.
 */
s16 overlay86SelectPosition(Overlay86Object *object, Overlay86VecOutput *output) {
    s32 start;
    s32 end;
    s32 selected;
    Overlay86Node **nodes;
    Overlay86Node *node;
    Overlay86State *state;
    Overlay86RingRecord *record;

    state = object->state;
    record = overlay1FindPreviousUsable(state->selectedIndex, &selected);
    state->selectedIndex = overlay1NextIndex(selected);
    output->x = 0.0f;
    output->y = 0.0f;
    output->z = 0.0f;

    if (record != 0) {
        output->x = record->x;
        output->y = record->y + 96.0f;
        output->z = record->z;
        state->record = record;
        record->flags |= 8;
        return record->angle + 0x4000;
    }

    nodes = func_8000572C(&start, &end);
    while (start < end) {
        node = nodes[start++];
        if ((node->kind == 5) && (node->ownerIndex == state->ownerIndex)) {
            output->x = node->x;
            output->y = node->y + 96.0f;
            output->z = node->z;
            state->record = 0;
            return node->angle;
        }
    }
    return 0;
}
