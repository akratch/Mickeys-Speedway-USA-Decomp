#include "PR/ultratypes.h"

extern s32 gOverlay33BufferIndex;
extern u8 D_8[][0xC00];
extern void *gOverlay33DisplayLists[];
extern void *gOverlay33BufferRefs[];
extern void *gOverlay33ActiveBuffer;
extern s32 gOverlay33Ready;

extern void overlay33BuildDisplayListReloc(void);
extern s32 rcpFast3d(u64 *dataStart, u64 *dataEnd, s32 taskType,
                     void *framebuffer);

/* Matched 2026-09-17, lane w7-o033. 39/39 words, frame 0x18, 18 relocs.
 * Non-volatile `g ^= 1` then `refs[g]` keeps the buffer-index address as one
 * coloured web through both regions and forwards the xor into the subscript.
 * `volatile` on the index forced a post-store reload. The build-list call is
 * a same-overlay SYMBOL reloc, so it goes through the Reloc placeholder. */
void overlay33PresentAndSwap(void) {
    s32 index;

    overlay33BuildDisplayListReloc();
    index = gOverlay33BufferIndex;
    rcpFast3d((u64 *)&D_8[index][0],
              (u64 *)gOverlay33DisplayLists[index], 4,
              gOverlay33BufferRefs[index]);
    gOverlay33BufferIndex ^= 1;
    gOverlay33ActiveBuffer = gOverlay33BufferRefs[gOverlay33BufferIndex];
    gOverlay33Ready = 1;
}
