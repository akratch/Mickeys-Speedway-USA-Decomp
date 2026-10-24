#include "PR/ultratypes.h"
#include "overlays/o045/resource_descriptor.h"

typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

/* Tier B: the real resource-group load/free walks terminate at -1. The two
 * texture groups use the resident renderer's node fields and null-texture
 * sentinels, with independent signed animation counters and float times.
 * Descriptor handles and state/error/count words have actual typed writers.
 * Tier D: original names/declarations/qualifiers remain inferred. In
 * particular, the initial packed offsets -12 are retained exactly; they are
 * overwritten before the menu draws. The old caller's signed frame and
 * oversized Sprite views remain separate source/native type debt. */
s16 o61ResourceIds[4] = {3, 4, 5, -1};
RcpTextureNode o61ConfirmSprite[2] = {
    {0, 0, -12, -12, 0}, {0, 0, 0, 0, 0},
};
s32 o61ConfirmAnimation = 13;
f32 o61ConfirmTime = 0.0f;
RcpTextureNode o61CancelSprite[2] = {
    {0, 0, -12, -12, 0}, {0, 0, 0, 0, 0},
};
s32 o61CancelAnimation = 13;
f32 o61CancelTime = 0.0f;
Overlay45ResourceDescriptor *o61TitleDescriptor = 0;
Overlay45ResourceDescriptor *o61LoadDescriptor = 0;
Overlay45ResourceDescriptor *o61SaveDescriptor = 0;
Overlay45ResourceDescriptor *o61EraseDescriptor = 0;
Overlay45ResourceDescriptor *o61DoneDescriptor = 0;
Overlay45ResourceDescriptor *o61NoPakDescriptor = 0;
Overlay45ResourceDescriptor *o61PakErrorDescriptor = 0;
Overlay45ResourceDescriptor *o61PakFullDescriptor = 0;
Overlay45ResourceDescriptor *o61YesDescriptor = 0;
Overlay45ResourceDescriptor *o61NoDescriptor = 0;
Overlay45ResourceDescriptor *o61RetryDescriptor = 0;
Overlay45ResourceDescriptor *o61AbortDescriptor = 0;
Overlay45ResourceDescriptor *o61SaveToSgDescriptor = 0;
s32 o61RepeatX = 0;
s32 o61RepeatY = 0;
u32 o61FreeBytes = 0;
s32 o61EntryCount = 0;
s32 o61SelectedEntry = 0;
s32 o61TopEntry = 0;
s32 o61State = 0;
s32 o61Error = 0;
s32 o61ErrorNextState = 0;
s32 o61ErrorBackState = 0;
