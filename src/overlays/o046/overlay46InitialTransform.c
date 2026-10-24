#include "overlays/overlay_046.h"

/* Tier B: the initializer stores the created resource here and reads the
 * three float values into its emitter state. Read-only use does not establish
 * original const qualification; all definitions remain mutable. */
Overlay46Resource *gOverlay46InitialAuxiliaryResource = 0;
f32 gOverlay46EmitterInitialX = 578.0f;
f32 gOverlay46EmitterInitialY = 17.0f;
f32 gOverlay46EmitterInitialSpeed = 10.7f;
