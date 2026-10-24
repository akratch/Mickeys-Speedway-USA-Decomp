#include "PR/ultratypes.h"

/* Tier B: the motion/view consumer masks both mode indices to 0..3,
 * authenticating sixteen float targets and four angles/cycle entries.
 * The cycle entries are genuinely writable. Read-only use of the targets
 * does not establish original const qualification; definitions remain
 * conservatively mutable. Tier D: names express these observed uses.
 * The last four physical bytes are ordinary compiler section alignment. */

f32 gOverlay9CameraTargetX[16] = {
    125.0f, 155.0f, 85.0f, 67.0f,
    180.0f, 215.0f, 145.0f, 127.0f,
    100.0f, 135.0f, 65.0f, 47.0f,
    100.0f, 135.0f, 65.0f, 47.0f,
};

f32 gOverlay9CameraTargetY[16] = {
    40.0f, 50.0f, 40.0f, 45.0f,
    50.0f, 50.0f, 40.0f, 45.0f,
    40.0f, 40.0f, 30.0f, 35.0f,
    40.0f, 40.0f, 30.0f, 35.0f,
};

s16 gOverlay9CameraAngles[4] = {
    1024, 512, 1024, 1024,
};

u8 gOverlay9CameraCycles[4] = {
    0, 0, 0, 0,
};
