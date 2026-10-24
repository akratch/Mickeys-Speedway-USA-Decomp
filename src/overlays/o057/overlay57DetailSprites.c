#include "overlays/overlay_057.h"

/* Tier B: the unscaled badge renderer and scaled status-value renderer use
 * the same 16-byte node schema. Initialization supplies each first texture,
 * while the following null node terminates the list. */
O57MiddleTextureNode o57BadgeSprites[2] = {
    { NULL, NULL, 0, 0, 0 },
    { NULL, NULL, 0, 0, 0 },
};

O57MiddleTextureNode o57StatusValueSprites[2] = {
    { NULL, NULL, 0, -8, -8 },
    { NULL, NULL, 0, 0, 0 },
};
