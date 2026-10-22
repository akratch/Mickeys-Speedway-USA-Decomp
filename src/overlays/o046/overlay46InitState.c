#include "PR/ultratypes.h"

extern s32 gOverlay46Enabled;
extern s32 gOverlay46Limit;
extern u8 gOverlay46StateA[];
extern u8 gOverlay46StateB[];
extern s16 gOverlay46Selection;
extern void overlay46InitResourceReloc(void *state);
extern void overlay46InitializeBuffers(void);

/* DKR v77/v80 and JFG checks found no exact donor for this state initializer. */
void overlay46InitState(void) {
    gOverlay46Enabled = 1;
    gOverlay46Limit = 0x5A;
    overlay46InitResourceReloc(gOverlay46StateA);
    overlay46InitResourceReloc(gOverlay46StateB);
    overlay46InitializeBuffers();
    gOverlay46Selection = 0;
}
