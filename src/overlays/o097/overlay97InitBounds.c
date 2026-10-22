#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding bounds initializer. */
typedef struct Overlay97Bounds {
    f32 first;
    f32 topA;
    f32 bottomA;
    f32 second;
    f32 topB;
    f32 bottomB;
    u32 third;
    f32 width;
    f32 height;
    f32 x;
    f32 y;
    f32 z;
} Overlay97Bounds;

typedef struct Overlay97BoundsObject {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay97Bounds *bounds;
} Overlay97BoundsObject;

typedef struct Overlay97BoundsInit {
    u8 pad0[6];
    s16 center;
    u8 pad8[2];
    u16 first;
    u16 second;
    u16 third;
    s16 width;
    s16 height;
    s16 x;
    s16 y;
    s16 z;
    s16 halfA;
    s16 halfB;
} Overlay97BoundsInit;

extern void func_8001C054(void);

void overlay97InitBounds(Overlay97BoundsObject *object,
                         Overlay97BoundsInit *init, s32 preserve) {
    Overlay97Bounds *bounds;

    bounds = object->bounds;
    bounds->first = (f32)init->first;
    bounds->topA = (f32)(init->center + init->halfA);
    bounds->bottomA = (f32)(init->center - init->halfA);
    bounds->second = (f32)init->second;
    bounds->topB = (f32)(init->center + init->halfB);
    bounds->bottomB = (f32)(init->center - init->halfB);
    bounds->third = (u32)(f32)init->third;
    bounds->width = (f32)init->width;
    bounds->height = (f32)init->height;
    bounds->x = object->x + (f32)init->x;
    bounds->y = object->y + (f32)init->y;
    bounds->z = object->z + (f32)init->z;
    if (preserve == 0) {
        func_8001C054();
    }
}
