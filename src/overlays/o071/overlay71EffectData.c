#include "PR/ultratypes.h"

/* Tier B: the polygon commands consume 2/8/8 sixteen-byte records; the
 * updater consumes eight signed T-coordinate triplets for each record bank.
 * Its three-iteration loop updates the secondary bank twice. These complete
 * arrays retain that behavior; graphics flag bit meanings remain unknown.
 * Original names/declarations are Tier D. The signed link-slot cell is passed
 * by value to SetLinkSlot; this source does not establish native address or
 * external vertex/state lifetime contracts. */
typedef struct O71Triangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} O71Triangle;
typedef struct O71TextureTTriplet { s16 t0, t1, t2; } O71TextureTTriplet;

s32 o71ResourceSlot = 71;
s16 o71TexturePhase = 0;

O71Triangle o71QuadTriangles[2] = {
    { 64, 0, 2, 1, 0, 0, 0, 0, 0, 0 },
    { 64, 0, 3, 2, 0, 0, 0, 0, 0, 0 },
};

O71Triangle o71PrimaryTriangles[8] = {
    { 0, 0, 5, 4, 0, 0, 992, 0, 992, 0 },
    { 0, 0, 1, 5, 0, 0, 0, 0, 992, 0 },
    { 0, 1, 6, 5, 0, 0, 992, 0, 992, 0 },
    { 0, 1, 2, 6, 0, 0, 0, 0, 992, 0 },
    { 0, 2, 7, 6, 0, 0, 992, 0, 992, 0 },
    { 0, 2, 3, 7, 0, 0, 0, 0, 992, 0 },
    { 0, 3, 4, 7, 0, 0, 992, 0, 992, 0 },
    { 0, 3, 0, 4, 0, 0, 0, 0, 992, 0 },
};

O71TextureTTriplet o71PrimaryTextureT[8] = {
    { 0, 1024, 0 },
    { 0, 1024, 1024 },
    { 0, 1024, 0 },
    { 0, 1024, 1024 },
    { 0, 1024, 0 },
    { 0, 1024, 1024 },
    { 0, 1024, 0 },
    { 0, 1024, 1024 },
};

O71Triangle o71SecondaryTriangles[8] = {
    { 0, 0, 4, 5, 0, 0, 992, 0, 992, 0 },
    { 0, 0, 5, 1, 0, 0, 992, 0, 0, 0 },
    { 0, 1, 5, 6, 0, 0, 992, 0, 992, 0 },
    { 0, 1, 6, 2, 0, 0, 992, 0, 0, 0 },
    { 0, 2, 6, 7, 0, 0, 992, 0, 992, 0 },
    { 0, 2, 7, 3, 0, 0, 992, 0, 0, 0 },
    { 0, 3, 7, 4, 0, 0, 992, 0, 992, 0 },
    { 0, 3, 4, 0, 0, 0, 992, 0, 0, 0 },
};

O71TextureTTriplet o71SecondaryTextureT[8] = {
    { 0, 0, 1024 },
    { 0, 1024, 1024 },
    { 0, 0, 1024 },
    { 0, 1024, 1024 },
    { 0, 0, 1024 },
    { 0, 1024, 1024 },
    { 0, 0, 1024 },
    { 0, 1024, 1024 },
};
