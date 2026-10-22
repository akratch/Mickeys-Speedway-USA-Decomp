#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG exact-object scans are negative for this radius query. */
typedef struct Overlay87Query {
    u8 pad0[4];
    f32 radius;
    u8 pad8[4];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay87Query;

typedef struct Overlay87Entry {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay87Entry;

extern Overlay87Entry **func_80005750(s32 *count);

s32 overlay87HasNearby(void *unused, Overlay87Query *query) {
    Overlay87Entry **entries;
    Overlay87Entry *entry;
    f32 radiusSquared;
    f32 dx;
    f32 dz;
    u8 result;
    s32 count;

    radiusSquared = query->radius;
    radiusSquared *= radiusSquared;
    entries = func_80005750(&count);
    result = FALSE;
    while (count != 0) {
        entry = *entries++;
        dx = entry->x - query->x;
        dz = entry->z - query->z;
        if ((dx * dx) + (dz * dz) < radiusSquared) {
            result = TRUE;
        }
        count--;
    }
    return result;
}
