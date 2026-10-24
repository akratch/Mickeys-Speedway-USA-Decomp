#include "PR/ultratypes.h"

/* Tier B: initialization interprets these words as front-end cache indices.
 * overlay50PatchIndices replaces indices with CPU pointers from D_800D31C8;
 * the texture renderer then consumes pointer/pointer/frame/x/y nodes.
 * The legacy helper's separate integer view remains outside this leaf. */
typedef union O50GlyphResource {
    s32 cacheIndex;
    void *resource;
} O50GlyphResource;

typedef struct O50ResourceGlyph {
    O50GlyphResource texture;
    O50GlyphResource alternate;
    s32 packedGlyph;
    s16 x;
    s16 y;
} O50ResourceGlyph;

O50ResourceGlyph o50SpeedGaugeGlyphs[4] = {
    { { 36 }, { 37 }, 0, 230, 160 },
    { { 34 }, { 35 }, 0, 230, 176 },
    { { 32 }, { 33 }, 0, 230, 194 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50PlaceDigits[3] = {
    { { 38 }, { 39 }, 0, 28, 24 },
    { { 41 }, { 42 }, 0, 40, 24 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50SinglePlaceDigit[2] = {
    { { 38 }, { 39 }, 6 << 16, 28, 24 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50HudCounterDigits[3] = {
    { { 18 }, { 19 }, 0, 100, 30 },
    { { 18 }, { 19 }, 0, 110, 30 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50RaceTimeGlyphs[10] = {
    { { 18 }, { 19 }, 0, 214, 30 },
    { { 18 }, { 19 }, 0, 224, 30 },
    { { 18 }, { 19 }, 11 << 16, 231, 30 },
    { { 18 }, { 19 }, 0, 238, 30 },
    { { 18 }, { 19 }, 0, 248, 30 },
    { { 18 }, { 19 }, 10 << 16, 255, 30 },
    { { 18 }, { 19 }, 0, 262, 30 },
    { { 18 }, { 19 }, 0, 272, 30 },
    { { 23 }, { 24 }, 0, 189, 25 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50BannerGlyphs[5] = {
    { { 97 }, { 98 }, 0, 0, 61 },
    { { 95 }, { 96 }, 0, 0, 77 },
    { { 93 }, { 94 }, 0, 0, 93 },
    { { 91 }, { 92 }, 0, 0, 109 },
    { { 0 }, { 0 }, 0, 0, 0 },
};
