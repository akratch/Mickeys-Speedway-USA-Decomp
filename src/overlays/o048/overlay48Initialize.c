#include "ultra64.h"

extern u8 gOverlay48State;
extern void overlay48ConfigureReloc();
extern void mainChangeCameras(s32 mode);

/* DKR v77/v80 and JFG have no exact matching three-call setup tuple. */
void overlay48Initialize(void) {
    overlay48ConfigureReloc(0x1C, 0, 0, 0xD, 1, 0);
    mainChangeCameras(1);
    overlay48ConfigureReloc(6, 6, 0, 2, 0, 0);
    gOverlay48State = 0;
}
