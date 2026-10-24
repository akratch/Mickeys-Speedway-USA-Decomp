#include "game/fx.h"
#include "overlays/overlay_064.h"

/* Tier B: the resident FX update/initialization paths index four callbacks.
 * The first is the procedural texture export; the remaining slots are empty.
 * The resident rename step preserves this source identity while ordinary ELF
 * linking initializes the slot to the loader's dangling-jump trap. */
FxTextureCallback D_8007D47C[4] = {
    func_overlay_064_F0000000_18C3B28, 0, 0, 0
};
