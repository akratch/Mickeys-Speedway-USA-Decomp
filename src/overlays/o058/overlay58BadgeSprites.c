#include "overlays/overlay_058.h"

/* Tier B: the result renderer reads 16-byte texture nodes until the next
 * null texture. Initialization supplies the first resource pointer. */
RcpTextureNode o58ResultBadgeSprites[2] = {
    { NULL, NULL, 0, 0, 0 },
    { NULL, NULL, 0, 0, 0 },
};
