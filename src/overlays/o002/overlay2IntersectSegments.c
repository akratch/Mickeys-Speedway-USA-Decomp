#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans have no exact donor. */
s32 overlay2IntersectSegments(f32 ax, f32 ay, f32 bx, f32 by,
                              f32 cx, f32 cy, f32 dx, f32 dy,
                              f32 *outX, f32 *outY) {
    f32 a1;
    f32 b1;
    f32 c1;
    f32 side1;
    f32 side2;
    f32 a2;
    f32 b2;
    f32 c2;
    f32 denominator;
    f32 bias;
    f32 numerator;

    a1 = by - ay;
    b1 = ax - bx;
    c1 = (bx * ay) - (ax * by);
    side1 = (a1 * cx) + (b1 * cy) + c1;
    side2 = (a1 * dx) + (b1 * dy) + c1;
    if (side1 != 0.0f) {
        if ((side2 != 0.0f) &&
            (((side1 > 0.0f) && (side2 > 0.0f)) ||
             ((side1 < 0.0f) && (side2 < 0.0f)))) {
            return 0;
        }
    }
    b2 = cx - dx;
    a2 = dy - cy;
    c2 = (dx * cy) - (cx * dy);
    side1 = (a2 * ax) + (b2 * ay) + c2;
    side2 = (a2 * bx) + (b2 * by) + c2;
    if ((side1 != 0.0f) && (side2 != 0.0f) &&
        (((side1 > 0.0f) && (side2 > 0.0f)) ||
         ((side1 < 0.0f) && (side2 < 0.0f)))) {
        return 0;
    }
    side1 = a2 * b1;
    side2 = a1 * b2;
    if (side1 == side2) {
        *outX = ax;
        *outY = ay;
        return 2;
    }
    if (outX != NULL) {
        denominator = side2 - side1;
        if (denominator < 0.0f) {
            bias = -denominator * 0.5f;
        } else {
            bias = denominator * 0.5f;
        }
        side1 = b1 * c2;
        side2 = b2 * c1;
        if (side1 < side2) {
            numerator = (side1 - side2) - bias;
        } else {
            numerator = (side1 - side2) + bias;
        }
        *outX = numerator / denominator;
        side1 = a2 * c1;
        side2 = a1 * c2;
        if (side1 < side2) {
            numerator = (side1 - side2) - bias;
        } else {
            numerator = (side1 - side2) + bias;
        }
        *outY = numerator / denominator;
    }
    return 1;
}
