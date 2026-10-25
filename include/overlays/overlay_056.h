#ifndef OVERLAY_056_H
#define OVERLAY_056_H

#include "PR/ultratypes.h"

typedef struct Overlay56Context {
    u8 pad0[0x12C];
    s16 resourceId;
} Overlay56Context;

extern void *o56MinimapResource_o056Reloc;
extern s16 o56MinimapFadeState_o056Reloc;
extern u8 gOverlay56Mode;
extern u32 gOverlay56Colors[];

void overlay56GetDimensionsReloc(u32 *width, u32 *height);
Overlay56Context *overlay56GetContextReloc(void);
void *overlay56LoadResourceReloc(s16 resourceId);
void overlay56ReleaseResourceReloc(void *resource);

void overlay56OffsetCoordinates(u32 *x, u32 *y);
void overlay56CenterCoordinates(s32 *x, s32 *y);
void overlay56SplitTime(s32 value, s32 *minutes, s32 *seconds,
                        s32 *centiseconds);
void overlay56SetMode(s32 mode);
void overlay56LoadResource(void);
void overlay56ReleaseResource(void);
void overlay56UnpackColor(s32 index, s32 *red, s32 *green, s32 *blue);

#endif
