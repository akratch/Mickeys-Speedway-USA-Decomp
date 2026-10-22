#include "PR/ultratypes.h"

typedef struct Overlay29SamplePoint {
    f32 x;
    f32 y;
} Overlay29SamplePoint;

typedef struct Overlay29SampleObject {
    u8 pad0[0xC];
    f32 frame;
} Overlay29SampleObject;

extern Overlay29SamplePoint *gOverlay29Base0;
extern Overlay29SamplePoint *gOverlay29Base1;
extern Overlay29SamplePoint *gOverlay29Base2;
extern Overlay29SamplePoint *gOverlay29Base3;
extern Overlay29SamplePoint *gOverlay29Selected0;
extern Overlay29SamplePoint *gOverlay29Selected1;
extern Overlay29SamplePoint *gOverlay29Selected2;
extern Overlay29SamplePoint *gOverlay29Selected3;
extern void overlay29RotateForward(s32 frame);
extern f32 splinePos(f32, f32, f32, f32, f32);

/* Exact at +0x304; DKR v77/v80 and JFG have no exact donor for this sampler. */
void overlay29Sample(Overlay29SampleObject *object, f32 *out0, f32 *out1,
                     f32 *out2, f32 advance) {
    f32 frame;
    f32 fraction;
    s32 whole;

    frame = object->frame + advance;
    whole = (s32)frame;
    overlay29RotateForward(whole);
    fraction = frame - (f32)whole;

    *out0 = splinePos(
        gOverlay29Selected0->x, gOverlay29Selected1->x,
        gOverlay29Selected2->x, gOverlay29Selected3->x, fraction);
    *out1 = splinePos(
        gOverlay29Base0->y, gOverlay29Base1->y,
        gOverlay29Base2->y, gOverlay29Base3->y, fraction);
    *out2 = splinePos(
        gOverlay29Selected0->y, gOverlay29Selected1->y,
        gOverlay29Selected2->y, gOverlay29Selected3->y, fraction);
}
