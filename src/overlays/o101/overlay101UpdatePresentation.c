#include "PR/ultratypes.h"

extern s32 gOverlay101PresentationActive;
extern s32 gOverlay101PresentationDone;
extern s32 gOverlay101PresentationTimer;
extern s32 gOverlay101InputMode;
extern s32 gOverlay101GlobalX;
extern s32 gOverlay101GlobalY;
extern void *gOverlay101Handle1D4;

extern void overlay101DispatchActive(s32 value);
extern void overlay101DispatchEvents(s32 value);
extern void overlay101UpdateChains(s32 value);
extern void overlay101DrawSlots(void *context, void *data,
                                            void *vertices);
extern void func_8002FB34(void *context, void **handle,
                                              f32 x, f32 y, f32 scaleX,
                                              f32 scaleY, s32 layer,
                                              s32 mode);
extern void overlay101Initialize(void);
extern s32 overlay101ReadInputReloc(s32 controller);

void overlay101UpdatePresentation(void *context, void *data, void *vertices,
                                  s32 step) {
    s32 state;

    if (gOverlay101PresentationActive != 0) {
        overlay101DispatchActive(step);
        overlay101DispatchEvents(step);
        overlay101UpdateChains(step);
        overlay101DrawSlots(context, data, vertices);
        func_8002FB34(
            context, &gOverlay101Handle1D4, (f32)gOverlay101GlobalX,
            (f32)gOverlay101GlobalY, 1.0f, 1.0f, -2, 3);
        state = gOverlay101PresentationDone;
    } else {
        state = gOverlay101PresentationDone;
        if (state == 0) {
            gOverlay101PresentationTimer += step;
            if (gOverlay101PresentationTimer >= 0xD3) {
                overlay101Initialize();
                gOverlay101PresentationTimer = 0;
                state = gOverlay101PresentationDone;
            }
        }
    }

    if (state == 0) {
        if (gOverlay101InputMode != 0) {
            if (overlay101ReadInputReloc(0) & 0x9000) {
                overlay101Initialize();
                gOverlay101PresentationDone = 1;
            }
        } else if ((overlay101ReadInputReloc(0) & 0x820) == 0x820) {
            overlay101Initialize();
            gOverlay101PresentationDone = 1;
        }
    }
}
