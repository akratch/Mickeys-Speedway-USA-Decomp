#include "overlays/overlay_065.h"

/* Tier B: current packet counts consume twelve particle and sixteen trail
 * triangles; the random color index is 0..6 and pointListRPY reads four
 * corners. Mutable cells retain the texture, camera mode, buffer index,
 * trail gravity and actual random-speed parameters.
 * Tier D: names and original declaration boundaries remain inferred.
 * Physical graphics-address views and the separate text TUs remain debt. */
O65Triangle o65ParticleTriangles[12] = {
    {64, 0, 1, 2, 0, 0, 480, 0, 0, 480},
    {64, 1, 3, 2, 480, 0, 480, 480, 0, 480},
    {64, 4, 5, 6, 0, 0, 480, 0, 0, 480},
    {64, 5, 7, 6, 480, 0, 480, 480, 0, 480},
    {64, 8, 9, 10, 0, 0, 480, 0, 0, 480},
    {64, 9, 11, 10, 480, 0, 480, 480, 0, 480},
    {64, 12, 13, 14, 0, 0, 480, 0, 0, 480},
    {64, 13, 15, 14, 480, 0, 480, 480, 0, 480},
    {64, 16, 17, 18, 0, 0, 480, 0, 0, 480},
    {64, 17, 19, 18, 480, 0, 480, 480, 0, 480},
    {64, 20, 21, 22, 0, 0, 480, 0, 0, 480},
    {64, 21, 23, 22, 480, 0, 480, 480, 0, 480},
};
O65Triangle o65TrailTriangles[16] = {
    {64, 0, 1, 2, 0, 0, 0, 0, 0, 0},
    {64, 1, 3, 2, 0, 0, 0, 0, 0, 0},
    {64, 2, 3, 4, 0, 0, 0, 0, 0, 0},
    {64, 3, 5, 4, 0, 0, 0, 0, 0, 0},
    {64, 4, 5, 6, 0, 0, 0, 0, 0, 0},
    {64, 5, 7, 6, 0, 0, 0, 0, 0, 0},
    {64, 6, 7, 8, 0, 0, 0, 0, 0, 0},
    {64, 7, 9, 8, 0, 0, 0, 0, 0, 0},
    {64, 8, 9, 10, 0, 0, 0, 0, 0, 0},
    {64, 9, 11, 10, 0, 0, 0, 0, 0, 0},
    {64, 10, 11, 12, 0, 0, 0, 0, 0, 0},
    {64, 11, 13, 12, 0, 0, 0, 0, 0, 0},
    {64, 12, 13, 14, 0, 0, 0, 0, 0, 0},
    {64, 13, 15, 14, 0, 0, 0, 0, 0, 0},
    {64, 14, 15, 16, 0, 0, 0, 0, 0, 0},
    {64, 15, 17, 16, 0, 0, 0, 0, 0, 0},
};
O65Color o65ParticleColors[7] = {
    {0, 0, 255},
    {0, 255, 0},
    {0, 255, 255},
    {255, 0, 0},
    {255, 0, 255},
    {255, 255, 0},
    {255, 255, 255},
};
O65Vec3f o65BillboardCorners[4] = {
    {-10.0f, 0.0f, -10.0f},
    {10.0f, 0.0f, -10.0f},
    {-10.0f, 0.0f, 10.0f},
    {10.0f, 0.0f, 10.0f},
};
TextureHeader *o65ParticleTexture = 0;
s32 o65CameraMotionActive = 0;
s32 o65VertexBufferIndex = 0;
f32 o65TrailGravity = 0.3f;
s32 o65TrailHorizontalSpeedRange = 10;
s32 o65TrailVerticalSpeedBase = 11;
