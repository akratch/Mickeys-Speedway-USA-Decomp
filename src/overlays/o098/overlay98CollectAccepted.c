#include "PR/ultratypes.h"

typedef struct O98Object {
    u8 pad00[0x8E];
    u8 accepted;
} O98Object;

typedef struct O98Entry {
    O98Object *object;
    f32 value;
} O98Entry;

extern u8 *camGetPtr(void);
extern s32 overlay98CheckObject(O98Object *, u8 *, f32 *);
extern s32 gOverlay98AcceptedCount;
extern O98Entry gOverlay98AcceptedEntries[0x50];

/* PROVENANCE: reproduced from the retired public mirror, commit
 * f56d08c746f891f76c4b7bab8e3a2a4332894634, which no longer resolves. All
 * retained measurements and the ABI correction above were independently
 * derived from Mickey's own source, object, relocation tables, retail bytes. */
/* Matched 2026-08-31 by tracing IDO's automatic stack-home producers. Removing
 * two transient entry aliases and making the address-taken result the third
 * surviving automatic reproduces the 60-word body, 0x50 frame, and all six
 * relocation sites; the complete linked US ROM is byte-identical. */
void overlay98CollectAccepted(s32 count, O98Object **objects) {
    u8 *context;
    s32 index;
    f32 value;

    context = camGetPtr();
    gOverlay98AcceptedCount = 0;
    index = count - 1;
    if (index >= 0) {
        do {
            O98Object *object = objects[index];

            index--;
            if (overlay98CheckObject(object, context, &value) != 0) {
                object->accepted = 1;
                gOverlay98AcceptedEntries[gOverlay98AcceptedCount].object =
                    object;
                gOverlay98AcceptedEntries[gOverlay98AcceptedCount++].value =
                    value;
                if (gOverlay98AcceptedCount >= 0x50) {
                    index = -1;
                }
            } else {
                object->accepted = 0;
            }
        } while (index >= 0);
    }
}
