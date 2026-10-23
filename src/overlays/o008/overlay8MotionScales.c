#include "overlays/overlay_008.h"

/* Tier B: the channel update indexes selector scales with a low nibble
 * and wraps phase indices with 0x1F. The neighboring parameter records
 * bound this complete 16+32-entry family. Both arrays already have const
 * declarations; retain their order and read-only C contract. */

const f32 gOverlay8SelectorScales[16] = {
    0.02f, 0.025f, 0.03f, 0.0425f,
    0.05f, 0.06f, 0.0425f, 0.055f,
    0.075f, 0.075f, 0.05f, 0.06f,
    0.0425f, 0.0325f, 0.055f, 0.0f,
};

const f32 gOverlay8PhaseScales[32] = {
    0.9f, -0.49f, 0.13f, 0.11f,
    -0.43f, -0.28f, -0.76f, 0.42f,
    -0.75f, 0.21f, 0.87f, 0.66f,
    0.13f, 0.59f, -0.59f, -0.4f,
    0.0f, -0.86f, 0.34f, -0.08f,
    0.97f, -0.09f, 0.64f, -0.71f,
    0.54f, -0.4f, 0.6f, 0.03f,
    -0.48f, 0.6f, 0.24f, -0.83f,
};

typedef char O8MotionScaleWidth[(sizeof(f32) == 4) ? 1 : -1];
