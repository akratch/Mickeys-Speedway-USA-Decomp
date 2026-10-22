#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Selection {
    s16 index;
    u8 pad2[2];
    s16 first;
    s16 second;
    s16 third;
    s16 fourth;
} Overlay97Selection;

typedef struct Overlay97SelectionObject {
    u8 pad0[0x64];
    Overlay97Selection *selection;
} Overlay97SelectionObject;

typedef struct Overlay97SelectionInit {
    u8 pad0[0xA];
    s16 index;
    s8 first;
    s8 second;
} Overlay97SelectionInit;

typedef struct Overlay97SelectionTable {
    u8 pad0[0x18];
    s16 count;
} Overlay97SelectionTable;

extern Overlay97SelectionTable *trackGetTrack(void);

void overlay97InitSelection(Overlay97SelectionObject *object,
                            Overlay97SelectionInit *init, s32 preserve) {
    Overlay97Selection *selection;
    Overlay97SelectionTable *table;
    s16 count;

    selection = object->selection;
    table = trackGetTrack();
    selection->index = init->index;
    if (selection->index < 0) {
        selection->index = 0;
    }
    count = table->count;
    if (selection->index >= count) {
        selection->index = count - 1;
    }
    selection->first = init->first;
    selection->second = init->second;
    if (preserve == 0) {
        selection->third = 0;
        selection->fourth = 0;
    }
}
