/* Mickey-only active seed rows, Tier D original declaration inference.
 * Each real constructor iteration consumes asset ID and signed XY.
 * The retained terminator/unread tail is deliberately not part of this owner.
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
};
#ifdef TARGET_N64
typedef char O63SeedRowSize[(sizeof(Overlay63ParticleSeed) == 6) ? 1 : -1];
typedef char O63SeedArraySize[(sizeof(overlay63ParticleSeeds) == 120) ? 1 : -1];
#endif
