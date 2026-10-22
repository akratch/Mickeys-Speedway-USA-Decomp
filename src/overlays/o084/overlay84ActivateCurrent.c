#include "PR/ultratypes.h"

typedef struct Overlay84Choice {
    u8 pad00[0x11];
    u8 resource0;
    u8 pad12[2];
    u8 resource1;
    u8 pad15[5];
    u8 resource2;
    u8 resource3;
} Overlay84Choice;

typedef struct Overlay84Node {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84Choice *choice;
} Overlay84Node;

typedef struct Overlay84ResourceRecord {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x14];
} Overlay84ResourceRecord;

typedef struct Overlay84Resource {
    u8 pad00[0x16];
    u8 flags;
    u8 recordIndex;
    u8 pad18[8];
    Overlay84ResourceRecord *records;
} Overlay84Resource;

typedef struct Overlay84State {
    u8 pad00;
    s8 current;
    u8 pad02;
    u8 timer;
    u8 phase;
    u8 pad05[4];
    u8 mode;
    u8 pad0A[6];
    s16 tilt;
    s16 targetTilt;
    s16 savedTilt;
    s16 currentTilt;
    u8 pad18[8];
    s32 angle;
    f32 height;
    f32 targetHeight;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[8];
    Overlay84Node *nodes[32];
    u8 selectedResource;
    u8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad00[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;
extern Overlay84Resource *func_800508B4(u8 resource);
extern void func_8005055C(u8 resource);
extern void animseqStartPath(u8 resource);

/* Reusing the word-sized selector parameter preserves the retail register
 * lifetime and stack frame while the selected byte is stored in state. */
void overlay84ActivateCurrent(s32 kind) {
    Overlay84State *state;
    Overlay84Node *node;
    Overlay84Choice *choice;
    Overlay84Resource *resource;
    Overlay84ResourceRecord *record;
    s16 tilt;
    f32 height;

    if (gOverlay84Object != NULL) {
        state = gOverlay84Object->state;
        node = state->nodes[state->current];
        state->active = 0;
        choice = node->choice;

        switch (kind) {
        case 0:
            kind = choice->resource0;
            break;
        case 1:
            kind = choice->resource1;
            break;
        case 2:
            kind = choice->resource2;
            break;
        case 3:
            kind = choice->resource3;
            break;
        }

        resource = func_800508B4(kind);
        if (resource != NULL) {
            record = &resource->records[resource->recordIndex - 1];
            record->x = node->x;
            record->y = node->y;
            record->z = node->z;
            record->angle = node->angle;
            record->tilt = node->tilt;
            record->roll = node->roll;

            state->selectedResource = kind;
            state->phase = 0;
            state->timer = 20;
            state->scale = 1.0f;
            tilt = -node->tilt;
            state->tilt = tilt;
            state->targetTilt = tilt;
            state->savedTilt = state->currentTilt;
            state->angle = 0x8000 - node->angle;
            height = node->y;
            state->height = height;
            state->targetHeight = height;
            state->x = node->x;
            state->y = node->y;
            state->z = node->z;

            func_8005055C(state->selectedResource);
            animseqStartPath(state->selectedResource);
            resource->flags |= 2;
        }
        state->mode = 1;
    }
}
