#include "PR/ultratypes.h"

/* Tier B: actual load/free asset-list readers stop at -1. The RCP reader
 * uses 16-byte texture nodes and stops at a null texture; the first node's
 * texture is installed by the module initializer. Tier D: bounded storage
 * views and names, not recovered original declarations or closure of the
 * legacy caller prototypes and aliases. */
#include "game/texture_header.h"
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

s16 gOverlay46ParticleAssets[] = {
    101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, -1,
};
RcpTextureNode gOverlay46CopyrightNodes[2] = {
    { 0, 0, 0, -40, -16 },
    { 0, 0, 0, 0, 0 },
};
void *gOverlay46InitialResourceA = 0;
void *gOverlay46InitialResourceB = 0;
void *gOverlay46InitialAnimation = 0;
s32 gOverlay46InitialMode = 0;
s32 gOverlay46InitialTimer = 0;
