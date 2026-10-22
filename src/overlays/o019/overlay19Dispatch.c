#include "PR/ultratypes.h"

typedef struct Overlay19Header {
    u8 pad0[0x22];
    s8 count;
    u8 pad23[0x4E];
    s8 selected;
} Overlay19Header;

typedef struct Overlay19Object {
    u8 pad0[0x40];
    Overlay19Header *header;
    u8 pad44[0x24];
    void **items;
} Overlay19Object;

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 19 as none. */
extern void overlay19BuildOutput(void *item, void *data, void *end);

void overlay19Dispatch(Overlay19Object *object) {
    Overlay19Header *header;
    s32 selected;
    s32 index;
    s32 offset;
    void *item;
    void *data;

    header = object->header;
    selected = header->selected;
    if (selected >= 0) {
        item = object->items[selected];
        data = *(void **)item;
        overlay19BuildOutput(item, data, (u8 *)data + 0x58);
    } else {
        index = 0;
        offset = 0;
        if (header->count > 0) {
            do {
                item = *(void **)((u8 *)object->items + offset);
                data = *(void **)item;
                overlay19BuildOutput(item, data, (u8 *)data + 0x58);
                index++;
                offset += 4;
            } while (index < object->header->count);
        }
    }
}
