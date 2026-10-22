#include "PR/ultratypes.h"

typedef struct Overlay68Payload {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
} Overlay68Payload;

typedef struct Overlay68Entry {
    void *object;
    s8 kind;
    u8 field5;
    s16 generation;
    s16 width;
    s16 height;
    Overlay68Payload *payload;
} Overlay68Entry;

typedef struct Overlay68CreateInfo {
    s16 type;
    s16 pad2;
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 padD[3];
    Overlay68Entry *entry;
} Overlay68CreateInfo;

extern s16 gOverlay68Types[];
extern s32 levelGetNumber(void);
extern void *func_8000590C(Overlay68CreateInfo *info, s32 count);

/* Pinned DKR v77/v80 and JFG object/source checks found payload-construction
 * relatives only; none is an exact donor for this descriptor layout. */
void overlay68CreatePayload(Overlay68Entry *entry) {
    s32 kind;
    Overlay68CreateInfo info;
    Overlay68Payload *payload;

    if (entry != 0) {
        if (entry->width != 0) {
            if (entry->height != 0) {
                if (levelGetNumber() == entry->generation) {
                    kind = entry->kind;
                    payload = entry->payload;
                    if ((kind < 0) || (kind >= 10)) {
                        kind = 0;
                    }
                    info.type = gOverlay68Types[kind];
                    info.x = payload->x;
                    info.y = payload->y;
                    info.z = payload->z;
                    info.red = payload->red;
                    info.green = payload->green;
                    info.blue = payload->blue;
                    info.entry = entry;
                    entry->object = func_8000590C(&info, 1);
                    return;
                }
            }
        }
        entry->object = 0;
    }
}
