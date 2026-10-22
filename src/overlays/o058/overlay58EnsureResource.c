#include "PR/ultratypes.h"

typedef struct Overlay58Resource {
    u8 pad0[0x12];
    u8 count;
} Overlay58Resource;

extern void *D_8007A1FC;
extern s16 gOverlay58ResourceId;
extern Overlay58Resource D_800D3128;
extern s32 gOverlay58ResourceState;
extern s32 gOverlay58ResourceReady;
extern void *overlay68CheckKind(s16 id);

/* Pinned DKR v77/v80 and JFG scans found no matching donor. */
void overlay58EnsureResource(void) {
    if (D_8007A1FC == NULL) {
        D_8007A1FC = overlay68CheckKind(gOverlay58ResourceId);
        if (D_8007A1FC != NULL) {
            gOverlay58ResourceState = 2;
            D_800D3128.count++;
            gOverlay58ResourceReady = 1;
        } else {
            gOverlay58ResourceState = 0;
        }
    } else {
        gOverlay58ResourceState = 1;
    }
}
