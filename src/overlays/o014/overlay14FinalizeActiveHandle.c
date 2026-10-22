#include "PR/ultratypes.h"

typedef struct Overlay14Object {
    s16 id;
    u8 pad2[10];
    f32 x;
    f32 y;
    f32 z;
} Overlay14Object;

extern s32 gOverlay14ActiveHandle;
extern Overlay14Object *func_80005820(s32 index);
extern void controlSetPlayerSetup(s16 x, s16 y, s16 z, s16 angle);
extern s32 mainGetNextCharacter(void);
extern u8 frontGetMode(void);
extern void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup,
                            s32 nextCamera, s32 fadeOut, s32 flags);

/* Pinned DKR v77/v80 and JFG scans found no exact finalization donor. */
void overlay14FinalizeActiveHandle(void) {
    Overlay14Object *object;

    if (gOverlay14ActiveHandle != -1) {
        object = func_80005820(0);
        if (object != 0) {
            controlSetPlayerSetup((s16)(s32)object->x,
                          (s16)(s32)(object->y + 2.0f),
                          (s16)(s32)object->z, object->id);
        }
        mainChangeLevel(gOverlay14ActiveHandle, mainGetNextCharacter(), 0,
                        frontGetMode(), 1, 0);
        gOverlay14ActiveHandle = -1;
    }
}
