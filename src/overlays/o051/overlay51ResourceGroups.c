#include "overlays/overlay_051.h"

/* Tier B: the resident load/free helpers walk signed-halfword asset IDs
 * through the -1 terminator. The setup helper reads the second list using
 * the same sentinel contract; these are mutable front-end lists. */
s16 gOverlay51Resource0[12] = {
    18, 19, 20, 21, 22, 23, 24, 25, 26, 4, 80, -1,
};

s16 gOverlay51Resource18[2] = {4, -1};
