#include "overlays/overlay_060.h"

/* Tier B: loadFrontEndList/freeFrontEndList consume signed resource IDs until
 * -1. The two resident texture renderers consume 16-byte nodes through the
 * next null texture; initialization fills each first texture pointer.
 * Animation state/frame and the selected actor are mutable CPU state.
 * Tier D: names and original declarations are inferred. The two-node lists
 * retain complete terminal records; no meaning is assigned to unread fields.
 * The compiler supplies the asset-list and final alignment bytes. */
s16 o60MenuAssetList[] = {121, 3, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 74, 123, -1};
RcpTextureNode o60MenuPanelSprites[2] = {{NULL, NULL, 0, 0, 0}, {NULL, NULL, 0, 0, 0}};
RcpTextureNode o60AnimatedMenuSprites[2] = {{NULL, NULL, 0, -12, -12}, {NULL, NULL, 0, 0, 0}};
RcpTextureNode o60RecordCharacterSprites[2] = {{NULL, NULL, 0, -12, -12}, {NULL, NULL, 0, 0, 0}};
RcpTextureNode o60RecordBadgeSprites[2] = {{NULL, NULL, 0, -16, -16}, {NULL, NULL, 0, 0, 0}};
s32 o60TextureAnimationState = 13;
f32 o60TextureAnimationFrame = 0.0f;
ControlActor *o60SelectedMenuActor = NULL;
