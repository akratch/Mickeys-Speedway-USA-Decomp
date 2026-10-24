#include "PR/ultratypes.h"

/* Tier B: the initializer and updater read frame/x/y from exactly nine
 * 16-byte template records. The initial cache-index words remain unused
 * here; no pointer-conversion phase or following zero record is claimed. */
typedef struct O50LapGlyphTemplate {
    s32 textureCacheIndex;
    s32 alternateCacheIndex;
    s32 packedGlyph;
    s16 x;
    s16 y;
} O50LapGlyphTemplate;

O50LapGlyphTemplate o50LapTimeTemplate[9] = {
    { 20, 21, 0, -7, 0 },
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
    { 20, 21, 11 << 16, 14, 0 },
    { 20, 21, 0, 20, 0 },
    { 20, 21, 0, 27, 0 },
    { 20, 21, 10 << 16, 33, 0 },
    { 20, 21, 0, 40, 0 },
    { 20, 21, 0, 47, 0 },
};
