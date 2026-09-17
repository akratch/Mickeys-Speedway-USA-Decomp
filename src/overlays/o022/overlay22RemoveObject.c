#include "PR/ultratypes.h"

typedef struct Overlay22ModelState {
    u8 pad00[6];
    u16 flags06;
} Overlay22ModelState;

typedef struct Overlay22Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x30];
    Overlay22ModelState *model48;
    u8 pad4C[0x34];
    u32 flags80;
} Overlay22Object;

Overlay22Object *gOverlay22Nodes[12] = { 0 };
volatile s32 D_30 = 0;
static u8 overlay22DataPad34[0xC] = { 0 };
static f32 overlay22Constants[5] = { 14.4F, 14.4F, 0.8F, 0.03F, 0.707F };
static u8 overlay22DataTail[0xC] = { 0 };

extern void partUpdateTriggers(Overlay22Object *object, s32 mode);
extern void func_80002FE0(u16 soundId, f32 x, f32 y, f32 z, u8 priority,
                          void **handle);
extern void func_80006EA0(void *object);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
#ifdef NON_MATCHING
void func_overlay_022_F0000D30_1878E38(Overlay22Object *object, s32 flags) {
    s32 i;
    s32 found;
    s32 count;

    object->model48->flags06 &= ~1;
    count = D_30;
    found = -1;
    for (i = 0; i < count; i++) {
        if (gOverlay22Nodes[i] == object) {
            found = i;
            i = count;
        }
    }

    if (found != -1) {
        while (found < count - 1) {
            gOverlay22Nodes[found] = gOverlay22Nodes[found + 1];
            found++;
        }
        gOverlay22Nodes[count] = 0;
        D_30 = count - 1;
    }

    if (flags & 1) {
        object->flags80 |= 2;
        partUpdateTriggers(object, 1);
    }
    if (flags & 2) {
        func_80002FE0(0x22B, object->x, object->y, object->z, 4, 0);
    }
    if (flags & 4) {
        func_80002FE0(0x279, object->x, object->y, object->z, 4, 0);
    }
    func_80006EA0(object);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o022/overlay22RemoveObject/func_overlay_022_F0000D30_1878E38.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_022_F0000D30_1878E38:start
 * symbol: func_overlay_022_F0000D30_1878E38
 * score: 40/91 words
 * frame: 0x28
 * relocations: 12
 * first-mismatch: +0x10
 * summary: L160 index compact without current/end/last is 40/91 register-only at delta 0; identity-gated web 15 still takes v0 and force w15=c2 scores 34 not 0.
 * PLATEAU-HANDOFF:func_overlay_022_F0000D30_1878E38:end
 */
