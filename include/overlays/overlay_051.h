#ifndef OVERLAY_051_H
#define OVERLAY_051_H

#include "PR/ultratypes.h"
#include "game/menu_resources.h"
#include "overlays/patch_indices.h"

/* One 0x10-byte HUD glyph row entry; the texture and alternate fields hold
 * front-end item indices until overlay51PatchIndices rewrites them. */
typedef struct Overlay51Glyph {
    void *texture;
    void *alternate;
    s32 glyph;
    s16 x;
    s16 y;
} Overlay51Glyph;

extern s16 gOverlay51Resource0_o051Reloc[12];
extern s16 gOverlay51Resource18_o051Reloc[2];
extern Overlay51Glyph gOverlay51TimeGlyphs[10];
extern Overlay51Glyph gOverlay51ClockGlyphs[2];
extern s8 gOverlay51BlinkCounter;
extern s8 gOverlay51Item;
extern s32 gOverlay51ItemAlpha;
extern s32 gOverlay51TransitionDone;
extern f32 gOverlay51HudHeight;
extern s16 gOverlay51Handle;

void loadFrontEndList_o051Reloc(s16 *assetGroup);
void setupFrontEndList_o051Reloc(s16 *objectGroup);
void amTunePlay_o051Reloc(u8 sequenceId);
s32 runlinkDownloadCode_o051Reloc(s32 overlayIndex);
void overlay56LoadResource_o051Reloc(void);
s32 levelGetNumber_o051Reloc(void);
void freeFrontEndList(s16 *resource);
void overlay56ReleaseResource(void);
void freeFrontEndItem(s32 index);

void overlay51Initialize(void);
void overlay51ReleaseState(void);

#endif
