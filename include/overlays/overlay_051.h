#ifndef OVERLAY_051_H
#define OVERLAY_051_H

#include "PR/ultratypes.h"
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

extern void *D_800D31C8[];
extern u8 gOverlay51Resource0[];
extern u8 gOverlay51Resource18[];
extern Overlay51Glyph gOverlay51TimeGlyphs[10];
extern Overlay51Glyph gOverlay51ClockGlyphs[2];
extern s8 gOverlay51BlinkCounter;
extern s8 gOverlay51Item;
extern s32 gOverlay51ItemAlpha;
extern s32 gOverlay51TransitionDone;
extern f32 gOverlay51HudHeight;
extern s16 gOverlay51Handle;

s32 overlay51CreateReloc();
void overlay51PatchIndices();
void freeFrontEndList(void *resource);
void overlay56ReleaseResource(void);
void freeFrontEndItem(s32 index);

void overlay51Initialize(void);
void overlay51PatchIndices(OverlayPatchIndexEntry *entry);
void overlay51ReleaseState(void);

#endif
