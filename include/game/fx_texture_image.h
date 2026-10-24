#ifndef GAME_FX_TEXTURE_IMAGE_H
#define GAME_FX_TEXTURE_IMAGE_H

#include "PR/ultratypes.h"

/* Tier B: the resident FX code reads these dimensions, and the procedural
 * callback accesses 16-bit image data at byte 0x20. This shared N64 view
 * gives the actual caller and callback one compatible resource type.
 * The variable-length payload does not establish its allocation bounds.
 * Original header fields and declaration spelling remain unknown. */
typedef struct FxTextureImage {
    u8 pad00[6];
    u16 width;
    u16 height;
    u8 pad0A[0x16];
    u16 pixels[];
} FxTextureImage;

#endif
