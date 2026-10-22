#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG exact-object scans are negative for this radius query. */
typedef struct Overlay79Point {
    f32 x;
    f32 pad4;
    f32 z;
} Overlay79Point;

typedef struct Overlay79Entry {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay79Entry;

extern Overlay79Entry **func_80005750(s32 *count);

Overlay79Entry *overlay79FindNearby(Overlay79Point *point, f32 radiusSquared) {
    Overlay79Entry **entries;
    Overlay79Entry *entry;
    Overlay79Entry *result;
    f32 dx;
    f32 dz;
    volatile s32 scratch;
    s32 count;

    entries = func_80005750(&count);
    result = NULL;
    while (count != 0) {
        entry = *entries++;
        dx = entry->x - point->x;
        dz = entry->z - point->z;
        if ((dx * dx) + (dz * dz) < radiusSquared) {
            result = entry;
        }
        count--;
    }
    return result;
}
