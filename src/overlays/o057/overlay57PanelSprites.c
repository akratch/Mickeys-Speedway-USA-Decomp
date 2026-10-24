#include "overlays/overlay_057.h"

/* Tier B: the scaled texture renderer consumes 16-byte texture nodes and
 * stops at the next null texture. Initialization fills the first pointer;
 * the second complete node is each list's genuine terminator. */
O57MiddleTextureNode o57MirroredMenuSprites[2] = {
    { NULL, NULL, 0, 30, 30 },
    { NULL, NULL, 0, 0, 0 },
};

O57MiddleTextureNode o57RightPanelSprites[2] = {
    { NULL, NULL, 0, 380, 190 },
    { NULL, NULL, 0, 0, 0 },
};

O57MiddleTextureNode o57LeftPanelSprites[2] = {
    { NULL, NULL, 0, -70, 190 },
    { NULL, NULL, 0, 0, 0 },
};

O57MiddleTextureNode o57CaptionSprites[2] = {
    { NULL, NULL, 0, 0, 0 },
    { NULL, NULL, 0, 0, 0 },
};
