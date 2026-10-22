#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Plane {
    f32 x, y, z;
    f32 distance;
    f32 radiusSquared;
    f32 height;
    u16 first;
    u16 second;
} Overlay97Plane;

typedef struct Overlay97PlaneObject {
    u8 pad0[0xC];
    f32 x, y, z;
    u8 pad18[0x4C];
    Overlay97Plane *plane;
} Overlay97PlaneObject;

typedef struct Overlay97PlaneInit {
    u8 pad0[0xA];
    u16 angle;
    u16 height;
    u16 radius;
    u16 first;
    u16 second;
} Overlay97PlaneInit;

extern void mathOneFloatRPY(s16 *angles, Overlay97Plane *plane,
                                     Overlay97PlaneObject *object,
                                     Overlay97PlaneInit *init);

void overlay97InitPlane(Overlay97PlaneObject *object, Overlay97PlaneInit *init) {
    Overlay97Plane *plane;
    s16 angles[3];
    f32 radius;

    plane = object->plane;
    angles[0] = init->angle - 0x4000;
    angles[1] = 0;
    angles[2] = 0;
    plane->x = 0.0f;
    plane->y = 0.0f;
    plane->z = -1.0f;
    mathOneFloatRPY(angles, plane, object, init);
    plane->distance = -((plane->z * object->z) +
                        ((object->x * plane->x) + (object->y * plane->y)));
    plane->radiusSquared = (f32)init->radius;
    radius = plane->radiusSquared;
    plane->radiusSquared = radius * radius;
    plane->height = object->y + (f32)init->height;
    plane->first = init->first;
    plane->second = init->second;
}
