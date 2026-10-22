#include "PR/ultratypes.h"

/* Plane registration; exact DKR and JFG scans are negative. */
typedef struct Overlay21Plane {
    s16 id;
    s8 lowerPriority;
    s8 upperPriority;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
} Overlay21Plane;

typedef struct Overlay21Object {
    s16 angle0;
    s16 angle1;
    s16 unused4;
    u8 pad6[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 id;
    u8 pad30[0x34];
    Overlay21Plane *plane;
} Overlay21Object;

typedef struct Overlay21Init {
    u8 pad0[0xA];
    u8 angle0;
    u8 angle1;
    u8 lowerPriority;
    u8 upperPriority;
} Overlay21Init;

extern s32 gOverlay21ObjectCount;
extern Overlay21Object *gOverlay21Objects[];

f32 func_8002A8C0(s16 angle);
f32 overlay21CosReloc(s16 angle);

void overlay21RegisterPlane(Overlay21Object *object, Overlay21Init *init) {
    Overlay21Plane *plane;
    f32 value;
    s32 count;

    object->angle0 = init->angle0 << 8;
    object->angle1 = init->angle1 << 8;
    object->unused4 = 0;
    plane = object->plane;
    plane->id = object->id;
    value = func_8002A8C0(object->angle0);
    plane->normalX = overlay21CosReloc(object->angle1) * value;
    plane->normalY = -overlay21CosReloc(object->angle1);
    value = overlay21CosReloc(object->angle0);
    plane->normalZ = overlay21CosReloc(object->angle1) * value;
    plane->distance = -((plane->normalX * object->x) +
                        (plane->normalY * object->y) +
                        (plane->normalZ * object->z));
    plane->lowerPriority = init->lowerPriority;
    plane->upperPriority = init->upperPriority;
    count = gOverlay21ObjectCount;
    gOverlay21Objects[count] = object;
    gOverlay21ObjectCount = count + 1;
}
