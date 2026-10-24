#include "overlays/overlay_008.h"

/* Tier B: typed emission/camera/output consumers authenticate these fields
 * and masked camera indices. Tier D: ten-record extents are bounded storage
 * views ending at adjacent families/the runtime read-only origin, not recovered
 * original declarations or proof of all signed caller-index preconditions.
 * Read-only use does not establish original const qualification. The camera
 * blend is genuinely writable; the other definitions stay conservatively
 * mutable and remain accessible through the existing read-only views.
 * Motion-row pad002 retains two unread nonzero serialized bytes; they are
 * unknown fields in the partial consumer type, not compiler padding. */

O8P2640Tuning gOverlay8EmissionTuning[10] = {
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
    { 13.0f, 7.0f, 0.0025f },
};

f32 gOverlay8CameraMotion[16] = {
    120.0f, 155.0f, 93.5f, 67.0f,
    180.0f, 232.5f, 127.5f, 100.5f,
    120.0f, 155.0f, 93.5f, 67.0f,
    120.0f, 155.0f, 93.5f, 67.0f,
};

f32 gOverlay8CameraHeight[16] = {
    50.0f, 50.0f, 40.0f, 45.0f,
    50.0f, 50.0f, 40.0f, 45.0f,
    50.0f, 50.0f, 40.0f, 45.0f,
    50.0f, 50.0f, 40.0f, 45.0f,
};

s16 gOverlay8CameraAngles[4] = { 1024, 1024, 1024, 1024 };

f32 gOverlay8CameraBlend[4] = {
    0.0f, 0.0f, 0.0f, 0.0f,
};

f32 gOverlay8CameraHeightOffsets[4] = {
    20.0f, 30.0f, 20.0f, 20.0f,
};

Overlay8MotionRow gOverlay8MotionRows[10] = {
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
    { 3, 10, { 7, 4 }, 8600, 12400 },
};
