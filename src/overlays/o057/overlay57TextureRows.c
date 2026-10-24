#include "overlays/overlay_057.h"

/* Tier B: initialization writes ten first-texture pointers. The renderer's
 * real 16-byte node schema supplies frame offset and signed coordinates.
 * Tier D: the ten-record extent and original declaration are inferred from
 * that loop and the physical row layout; the following 16 bytes stay open.
 * Runtime bank[7] supplies the shared texture. No active draw use of this
 * particular node group has been established. */
RcpTextureNode o57TextureRows[10] = {
    { NULL, NULL, 0 << 16, 0, 0 },
    { NULL, NULL, 1 << 16, 32, 0 },
    { NULL, NULL, 2 << 16, 64, 0 },
    { NULL, NULL, 3 << 16, 96, 0 },
    { NULL, NULL, 4 << 16, 128, 0 },
    { NULL, NULL, 5 << 16, 0, 60 },
    { NULL, NULL, 6 << 16, 32, 60 },
    { NULL, NULL, 7 << 16, 64, 60 },
    { NULL, NULL, 8 << 16, 96, 60 },
    { NULL, NULL, 9 << 16, 128, 60 },
};
