#include "PR/ultratypes.h"

typedef struct Overlay29Point {
    f32 x;
    f32 y;
} Overlay29Point;

typedef struct Overlay29Object {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay29Object;

typedef struct Overlay29Ratio {
    u8 pad0[4];
    s16 angle;
    u8 pad6[6];
    f32 ratio;
    f32 value;
} Overlay29Ratio;

extern Overlay29Point *gOverlay29PointA;
extern Overlay29Point *gOverlay29PointB;
extern f32 sqrtf(f32);
extern s32 overlay29AngleReloc(s16);

/* Exact at +0x23C; DKR v77/v80 and JFG have no exact donor for this update. */
void overlay29UpdateRatio(Overlay29Object *object, Overlay29Ratio *ratio) {
    f32 dx;
    f32 dy;
    f32 referenceDistance;
    f32 objectDistance;

    dx = gOverlay29PointA->x - gOverlay29PointB->x;
    dy = gOverlay29PointA->y - gOverlay29PointB->y;
    referenceDistance = sqrtf((dx * dx) + (dy * dy));

    dx = gOverlay29PointA->x - object->x;
    dy = gOverlay29PointA->y - object->y;
    objectDistance = sqrtf((dx * dx) + (dy * dy));

    ratio->ratio = objectDistance / referenceDistance;
    ratio->value = (f32)overlay29AngleReloc(ratio->angle) + ratio->ratio;
}
