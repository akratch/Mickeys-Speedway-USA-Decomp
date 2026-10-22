#include "PR/ultratypes.h"

typedef struct Overlay57Transition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} Overlay57Transition;

extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;
extern s32 gOverlay57Delay;
extern Overlay57Transition gOverlay57Transition;

extern s32 overlay84GetActive(void);
extern void func_overlay_057_F0001020_18A4C18(s32 updateRate);
extern void overlay57UpdateNode(void);

/*
 * Overlay 57 +0x3048. Pinned DKR v77/v80 and JFG object/source checks found
 * no exact donor or semantic match for this three-field easing transition.
 */
void overlay57UpdateTransition(s32 updateRate) {
    s32 i;

    gOverlay57ModeFlag = 1;
    for (i = 0; i < updateRate; i++) {
        gOverlay57Transition.horizontal +=
            (0x17C - gOverlay57Transition.horizontal) >> 2;
        gOverlay57Transition.vertical +=
            (0xBE - gOverlay57Transition.vertical) >> 2;
        gOverlay57Delay += (0xA00 - gOverlay57Delay) >> 3;
    }
    if (overlay84GetActive() == 2) {
        gOverlay57Timer = 1;
        gOverlay57State = 1;
    }
    func_overlay_057_F0001020_18A4C18(updateRate);
    overlay57UpdateNode();
}
