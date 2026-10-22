#include "overlays/overlay_099.h"

typedef struct Overlay99Segment {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    u8 pad10[0x1A];
    s8 headingOffset;
    u8 pad2B;
    Overlay99RenderState *object;
} Overlay99Segment;

extern s32 gOverlay99SegmentCount;
extern Overlay99Segment gOverlay99Segments[];
extern void *D_800D2FAC;
extern void viGetCurrentSize(s32 *width, s32 *height);
extern void rcpClearZBuffer(Gfx **displayList, u32 width,
                                       u32 height, s32 x1, s32 y1, s32 x2,
                                       s32 y2);
extern void texDPInit(Gfx **displayList);
extern s16 Arctanf(f32 x, f32 z);
extern f32 sqrtf(f32 squared);
extern void func_80009E78(Gfx **displayList, Mtx **matrices,
                                       void *vertices,
                                       Overlay99RenderState *object);

/* Overlay 99 +0xBA4..+0xDDC, 142 words with no target padding. The shipped
 * relocation tables authenticate viGetCurrentSize, rcpClearZBuffer,
 * texDPInit, Arctanf, sqrtf, func_80009E78, and the local sorted-entry
 * renderer. */
void overlay99RenderSegments(Gfx **displayList, Mtx **matrices, void *vertices,
                             f32 scale) {
    f32 dx;
    f32 dz;
    s32 width;
    s32 height;
    f32 length;
    Overlay99Segment *segment;
    Overlay99RenderState *object;
    Gfx *command;
    s32 initialized;
    s32 i;

    i = 0;
    segment = gOverlay99Segments;
    initialized = 0;
    if (gOverlay99SegmentCount > 0) {
        do {
            object = segment->object;
            if (object != 0) {
                if (initialized == 0) {
                    viGetCurrentSize(&width, &height);
                    initialized = 1;
                    command = (*displayList)++;
                    command->words.w0 = 0xBC000806;
                    command->words.w1 = (u32)D_800D2FAC + 0x80000000;
                    rcpClearZBuffer(displayList, width, height, 0,
                                               0, width, height);
                    texDPInit(displayList);
                }

                dx = segment->x1 - segment->x0;
                dz = segment->z1 - segment->z0;
                object->rotation0 = 0;
                object->rotation1 = 0;
                object->heading = Arctanf(-dx, dz);
                dx *= scale;
                dz *= scale;
                object->x = segment->x0 + dx;
                object->y = segment->z0 + dz;
                object->z = 5.0f;
                length = sqrtf((dx * dx) + (dz * dz));
                if (segment->headingOffset != 0) {
                    object->heading += (s32)((f32)segment->headingOffset *
                                             65536.0f * scale);
                }
                object->flags = (s16)object->flags & ~0x400;
                func_80009E78(displayList, matrices, vertices,
                                           object);
                object->flags = (s16)object->flags | 0x400;
                overlay99RenderSortedEntries(displayList, matrices, vertices,
                                             object, length);
            }
            i++;
            segment++;
        } while (i < gOverlay99SegmentCount);
    }
}
