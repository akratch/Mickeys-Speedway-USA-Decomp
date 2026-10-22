#include "PR/ultratypes.h"

typedef struct Overlay57ModeValue {
    s32 value;
} Overlay57ModeValue;

typedef struct Overlay57Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay57Position;

extern s32 gOverlay57ModeFlag;
extern f32 gOverlay57DistanceLimit;
extern Overlay57Position *gOverlay57Reference;
extern s32 gOverlay57Timer;
extern s32 gOverlay57DistanceState;
extern void camSetNo(s32 value);
extern Overlay57Position *camGetPtr(void);
extern void func_800498FC(s32, s32, s32, s32, s32, s32, s32);
extern void *overlay84GetActive(void);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void overlay84Mark(void);

/* DKR v77/v80 and JFG contain no donor for this distance/state transition. */
void overlay57CheckDistance(Overlay57ModeValue value) {
    Overlay57Position *position;
    Overlay57Position *reference;
    f32 dx;
    f32 dy;
    f32 dz;

    gOverlay57ModeFlag = 0;
    camSetNo(0);
    position = camGetPtr();
    reference = gOverlay57Reference;
    if (reference != NULL) {
        dx = reference->x - position->x;
        dy = reference->y - position->y;
        dz = reference->z - position->z;
        if (dx * dx + dy * dy + dz * dz < gOverlay57DistanceLimit) {
            func_800498FC(4, 0x3EAE147B, 0xBF800000, 0, 0, 0, 0);
        }
    }
    if (overlay84GetActive() == NULL) {
        gOverlay57Timer = 0x11;
        mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    }
}
