#include "PR/ultratypes.h"

typedef struct Overlay99Influence {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    f32 longitudinalScale;
    f32 widthScale;
    f32 edgeWidth;
    f32 intensity;
    f32 angleDegrees;
    f32 angleScale;
} Overlay99Influence;

typedef struct Overlay99GridPoint {
    s16 reserved00;
    s16 reserved02;
    s16 height;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay99GridPoint;

extern s32 gOverlay99CurrentGrid;
extern Overlay99GridPoint *gOverlay99Grids[];
extern s32 gOverlay99GridWidth;
extern s32 gOverlay99GridHeight;
extern s32 gOverlay99WidthMinusOne;
extern s32 gOverlay99HeightMinusOne;
extern s32 gOverlay99Arg4;
extern s32 gOverlay99Arg5;

/* 65536 / 360 as the shipped pool spells it, four decimals. */
#define G_ANGLE_UNITS_PER_DEGREE 182.0444f

extern f32 overlay99AngleWaveReloc(s32 angle);
extern f32 overlay99AngleWavePhaseReloc(s32 angle);
extern f32 overlay99ProjectVector(f32 x, f32 y, f32 z, f32 dx, f32 dy);

/* Matched 2026-10-02, 190 words to 0, written from the listing. The
 * parameter t is reused for the side distance and one local carries both the
 * along fraction and the edge wave: those two reuses are what order the six
 * saved float registers. The plane terms are plain locals (the middle ones
 * stay in memory on their own, no volatile), the segment ends are re-read
 * from the record, the grid pointer is loaded last, and the final product
 * goes through a local before the conversion. -Wab,-r4300_mul supplies the
 * three multiply-hazard pads. */
void overlay99ApplySegment(Overlay99Influence *influence, f32 t) {
    s32 i;
    s32 j;
    Overlay99GridPoint *point;
    s32 phase;
    f32 x;
    f32 z;
    f32 distance;
    f32 along;
    f32 wave;
    f32 amount;
    s32 pad;
    f32 invLength;
    f32 invAngle;
    f32 width;
    f32 edge;
    f32 planeD;
    f32 normalZ;
    f32 deltaX;
    f32 sideD;
    f32 normalX;

    x = influence->x0 + ((influence->x1 - influence->x0) * t);
    z = influence->z0 + ((influence->z1 - influence->z0) * t);
    normalZ = influence->z0 - z;
    deltaX = x - influence->x0;
    normalX = -deltaX;
    sideD = -((influence->x0 * normalZ) + (influence->z0 * deltaX));
    planeD = -((x * normalX) + (z * normalZ));
    invLength = 1.0f / influence->longitudinalScale;
    phase = (s32)(influence->angleDegrees * G_ANGLE_UNITS_PER_DEGREE);
    invAngle = 1.0f / influence->angleScale;
    point = gOverlay99Grids[gOverlay99CurrentGrid];
    if (point != 0) {
        for (j = 0; j < gOverlay99GridHeight; j++) {
            for (i = 0; i < gOverlay99GridWidth; i++) {
                x = (f32)(i - (gOverlay99WidthMinusOne >> 1)) *
                    (f32)gOverlay99Arg4;
                z = (f32)((gOverlay99HeightMinusOne >> 1) - j) *
                    (f32)gOverlay99Arg5;
                distance = overlay99ProjectVector(normalX, normalZ, planeD,
                                                  x, z);
                if (distance > 0.0f) {
                    along = distance * invLength;
                    if ((along > 0.0f) && (along < 1.0f)) {
                        wave = overlay99AngleWaveReloc(
                            (s32)(distance * invLength * 16384.0f));
                        width = influence->widthScale * along;
                        t = overlay99ProjectVector(normalZ, deltaX, sideD,
                                                      x, z);
                        if (t < 0.0f) {
                            t = -t;
                        }
                        if (t <= width) {
                            t = width - t;
                            edge = influence->edgeWidth;
                            if (t < edge) {
                                along = overlay99AngleWaveReloc(
                                    (s32)((t * 16384.0f) / edge));
                                amount = overlay99AngleWavePhaseReloc(
                                             (s32)(t * 65536.0f *
                                                   invAngle) +
                                             phase) *
                                         (influence->intensity * along *
                                          wave);
                                point->height += (s32)amount;
                            }
                        }
                    }
                }
                point++;
            }
        }
    }
}
