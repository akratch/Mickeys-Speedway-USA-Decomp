#include "PR/ultratypes.h"

typedef struct Overlay36Data {
    u8 pad00[9];
    s8 marker;
    u8 intensity;
    u8 enabled;
} Overlay36Data;

typedef struct Overlay36State {
    u8 pad00[4];
    u8 mode;
} Overlay36State;

typedef struct Overlay36Entry {
    s16 unused;
    s16 value;
} Overlay36Entry;

typedef struct Overlay36Object {
    u8 pad00[0x28];
    f32 value;
    u8 pad2C[0x1A];
    s16 type;
    u8 pad48[8];
    Overlay36State *state;
    u8 pad54[0x10];
    Overlay36Data *data;
    Overlay36Entry **entry;
} Overlay36Object;

extern u16 gOverlay36Mode;
extern u8 *overlay36AcquireReloc(void);
extern void overlay36ConfigureReloc(Overlay36Data *data, s32 mode);
extern s32 overlay36QueryReloc(s32 index, s16 value, Overlay36Data *data);

/* DKR v77/v80 and JFG contain no exact donor for this object initializer. */
void overlay36InitObject(Overlay36Object *object, void *entry) {
    Overlay36Data *data;
    u8 *resource;

    data = object->data;
    resource = overlay36AcquireReloc();
    overlay36ConfigureReloc(data, 12);
    data->intensity = 0x80;
    data->marker = -1;

    if (object->type == 0x86 || object->type == 0xEB) {
        if (*resource == 1 || gOverlay36Mode == 0) {
            data->enabled = 1;
        } else {
            Overlay36Entry *entry;
            entry = *object->entry;
            object->value = overlay36QueryReloc(0, entry->value, data);
        }
    }
    if (object->state != 0) {
        object->state->mode = 2;
    }
}
