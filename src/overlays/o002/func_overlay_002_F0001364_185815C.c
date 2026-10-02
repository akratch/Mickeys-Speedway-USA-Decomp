#include "PR/ultratypes.h"

/* Segment intersection: the second copy of overlay2IntersectSegments in this
 * overlay, written the same way and built with -Wab,-r4300_mul
 * (mk/overlays.mk): the shipped code keeps every mul.s out of branch delay
 * slots and separates back-to-back multiplies, which is that flag's
 * signature. From the 123-word inherited candidate, what closed it: the
 * flag (122 -> 18 once the shape below was in place), the parallel test as a
 * difference of the two products compared with zero and the sign test
 * against an integer zero (both keep the denominator as its own value
 * instead of a compare of the two products), one rounded numerator per
 * coordinate instead of two divides each, and the locals in this order (the
 * last one moves every home and spill cell down by one). */
s32 func_overlay_002_F0001364_185815C(f32 x1, f32 y1, f32 x2, f32 y2,
                                      f32 x3, f32 y3, f32 x4, f32 y4,
                                      f32 *x, f32 *y) {
    f32 side3;
    f32 side4;
    f32 a1;
    f32 b1;
    f32 c1;
    f32 a2;
    f32 side1;
    f32 b2;
    f32 c2;
    f32 side2;
    f32 denom;
    f32 offset;
    f32 numeratorX;
    f32 numeratorY;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = (x2 * y1) - (x1 * y2);
    side3 = (a1 * x3) + (b1 * y3) + c1;
    side4 = (a1 * x4) + (b1 * y4) + c1;
    if (side3 && side4 &&
        (((side3 > 0.0f) && (side4 > 0.0f)) ||
         ((side3 < 0.0f) && (side4 < 0.0f)))) {
        return 0;
    }

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = (x4 * y3) - (x3 * y4);
    side1 = (a2 * x1) + (b2 * y1) + c2;
    side2 = (a2 * x2) + (b2 * y2) + c2;
    if (side1 && side2 &&
        (((side1 > 0.0f) && (side2 > 0.0f)) ||
         ((side1 < 0.0f) && (side2 < 0.0f)))) {
        return 0;
    }

    side1 = a2 * b1;
    side2 = a1 * b2;
    denom = side2 - side1;
    if (denom == 0.0f) {
        *x = x1;
        *y = y1;
        return 2;
    }

    if (x != NULL) {
        if (denom < 0) {
            offset = -denom * 0.5f;
        } else {
            offset = denom * 0.5f;
        }
        side1 = b1 * c2;
        side2 = b2 * c1;
        if (side1 < side2) {
            numeratorX = (side1 - side2) - offset;
        } else {
            numeratorX = (side1 - side2) + offset;
        }
        *x = numeratorX / denom;
        side1 = a2 * c1;
        side2 = a1 * c2;
        if (side1 < side2) {
            numeratorY = (side1 - side2) - offset;
        } else {
            numeratorY = (side1 - side2) + offset;
        }
        *y = numeratorY / denom;
    }

    return 1;
}
