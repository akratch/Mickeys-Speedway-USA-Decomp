/* Mickey-only serialized seed grammar, Tier D original declaration inference.
 * Twenty active rows consume signed asset/XY fields; the next row terminates
 * on assetId -1. Its stored coordinates are never read on this traversal.
 * This same-grammar terminal model is inferred, not original capacity proof.
 * Existing scalar/weak imported traversal and its C bounds debt are unchanged. */
#include "PR/ultratypes.h"

typedef struct Overlay63ParticleSeed {
    s16 assetId;
    s16 x;
    s16 y;
} Overlay63ParticleSeed;

Overlay63ParticleSeed overlay63ParticleSeeds[] = {
    { 101, -73, 77 },
    { 102, -47, 77 },
    { 103, -26, 77 },
    { 104, -2, 77 },
    { 105, 24, 77 },
    { 106, 49, 77 },
    { 107, 72, 96 },
    { 108, 77, 75 },
    { 109, -99, 32 },
    { 110, -72, 29 },
    { 111, -46, 30 },
    { 112, -24, 30 },
    { 113, 5, 31 },
    { 114, 31, 33 },
    { 115, 59, 32 },
    { 116, 86, 35 },
    { 117, -40, -19 },
    { 118, 1, -15 },
    { 119, 34, -13 },
    { 120, 77, -30 },
    { -1, 200, -80 },
};
#ifdef TARGET_N64
typedef char O63SeedRowSize[(sizeof(Overlay63ParticleSeed) == 6) ? 1 : -1];
typedef char O63SeedArraySize[(sizeof(overlay63ParticleSeeds) == 126) ? 1 : -1];
#endif
