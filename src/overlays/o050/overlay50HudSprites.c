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

O50ResourceGlyph o50TimeHudSprite[2] = {
    { { 25 }, { 0 }, 0, 189, 26 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50LapCaption[2] = {
    { { 30 }, { 31 }, 0, 182, 53 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O50ResourceGlyph o50ItemCountDigit[2] = {
    { { 18 }, { 19 }, 0, -6, -8 },
    { { 0 }, { 0 }, 0, 0, 0 },
};
