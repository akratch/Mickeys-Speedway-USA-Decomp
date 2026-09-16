#ifndef OVERLAYS_OVERLAY_015_H
#define OVERLAYS_OVERLAY_015_H

#include "PR/ultratypes.h"

typedef struct Overlay15Star {
    f32 x;
    f32 y;
    f32 z;
} Overlay15Star;

typedef struct Overlay15InitBounds {
    f32 xMin;
    f32 xMax;
    f32 xRange;
    f32 yMin;
    f32 yMax;
    f32 yRange;
    f32 zMin;
    f32 zMax;
    f32 zRange;
    f32 colorDivisor;
    f32 colorStep;
    s32 zero;
} Overlay15InitBounds;

typedef struct Overlay15Gfx {
    u32 w0;
    u32 w1;
} Overlay15Gfx;

typedef struct Overlay15MovingStarCamera {
    u8 pad[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay15MovingStarCamera;

typedef struct Overlay15CameraState {
    s16 angle;
} Overlay15CameraState;

extern void *gOverlay15Resource4;
extern void *gOverlay15Resource10;
extern void *gOverlay15Resource48;
extern s32 gOverlay15ValueC;
extern s32 gOverlay15Value7C;

extern s32 gOverlay15StarCount;
extern Overlay15Star *gOverlay15Stars;
extern u16 *gOverlay15StarPalette;
extern u32 *gOverlay15StarColors;
extern Overlay15InitBounds gOverlay15InitBounds;
extern Overlay15Gfx gOverlay15StarSetup[];
extern f32 gOverlay15StarBound0;
extern f32 gOverlay15StarBound1;
extern f32 gOverlay15StarBound2;
extern f32 gOverlay15StarBound3;
extern f32 gOverlay15StarBound4;
extern f32 gOverlay15StarBound5;
extern f32 gOverlay15StarBound6;
extern f32 gOverlay15StarBound7;
extern f32 gOverlay15StarBound8;
extern Overlay15Star gOverlay15StarMovement;

extern s32 gOverlay15CameraReadyRead;
extern s32 gOverlay15CameraReadyWrite;
extern f32 gOverlay15CurrentPositionX;
extern f32 gOverlay15CurrentPositionY;
extern f32 gOverlay15CurrentPositionZ;
extern f32 gOverlay15PreviousCameraX;
extern f32 gOverlay15PreviousCameraY;
extern f32 gOverlay15PreviousCameraZ;
extern s32 gOverlay15MovingStarCount;
extern Overlay15Star *gOverlay15MovingStars;
extern f32 gOverlay15MovingBound0;
extern f32 gOverlay15MovingBound1;
extern f32 gOverlay15MovingBound2;
extern f32 gOverlay15MovingBound3;
extern f32 gOverlay15MovingBound4;
extern f32 gOverlay15MovingBound5;
extern f32 gOverlay15MovingBound6;
extern f32 gOverlay15MovingBound7;
extern f32 gOverlay15MovingBound8;

extern s32 gOverlay15RainCapacity;
extern s32 gOverlay15RainEnabled;
extern Overlay15Star *gOverlay15RainPositions;
extern f32 gOverlay15RainOffsetX;
extern f32 gOverlay15RainOffsetY;
extern f32 gOverlay15RainOffsetZ;
extern u32 *gOverlay15RainColors;

extern void *overlay15Allocate(s32 size, s32 tag);
extern s32 overlay15RandomRange(s32 minimum, s32 maximum);
extern void overlay15ReleaseReloc(void *resource);
extern void overlay15GetDimensionsReloc(s32 *width, s32 *height);
extern void overlay15FinishDisplayListReloc(Overlay15Gfx **displayList);
extern void *overlay15GetActiveCameraReloc(void);
extern void starfieldFastMove(s32 count, Overlay15Star *stars,
                              f32 movementX, f32 movementY, f32 movementZ,
                              f32 bound0, f32 bound1, f32 bound2,
                              f32 bound3, f32 bound4, f32 bound5,
                              f32 bound6, f32 bound7, f32 bound8);
extern void rainFastDraw(void *framebuffer, s32 width, s32 height, s32 count,
                         Overlay15Star *positions, u32 *colors, s32 angle,
                         f32 offsetX, f32 offsetY, f32 offsetZ,
                         f32 projectionScale);

#endif
