#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Transform {
    s16 angle;
    u8 pad2[0x82];
    s32 mode;
    s32 index;
} Overlay97Transform;

typedef struct Overlay97TransformInit {
    u8 pad0[0xB];
    u8 index;
    u8 angle;
    s8 mode;
} Overlay97TransformInit;

extern s32 gOverlay97TransformContext;
extern void SetLinkSlot(s32 context, s32, s32,
                                         Overlay97Transform *transform);

void overlay97InitTransform(Overlay97Transform *transform,
                            Overlay97TransformInit *init) {
    SetLinkSlot(gOverlay97TransformContext, 2, 2, transform);
    transform->mode = init->mode;
    transform->index = init->index;
    transform->angle = (s16)((((s32)init->angle) & 0xFF) << 10);
}
