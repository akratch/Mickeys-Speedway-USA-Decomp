#include "PR/ultratypes.h"

extern s32 gOverlay101PresentationActive;
extern s32 gOverlay101PresentationDone;
extern s32 gOverlay101PresentationTimer;
extern s32 gOverlay101InputMode;
extern s32 gOverlay101GlobalX;
extern s32 gOverlay101GlobalY;
extern void *gOverlay101Handle1D4;

extern void overlay101StopFirstReloc(s32 value);
extern void overlay101StopSecondReloc(s32 value);
extern void overlay101StopThirdReloc(s32 value);
extern void overlay101DrawPresentationReloc(void *context, void *data,
                                            void *vertices);
extern void overlay101CreatePresentationReloc(void *context, void **handle,
                                              f32 x, f32 y, f32 scaleX,
                                              f32 scaleY, s32 layer,
                                              s32 mode);
extern void overlay101AdvancePresentationReloc(void);
extern s32 overlay101ReadInputReloc(s32 controller);
extern void overlay101FinishPresentationReloc(void);

void overlay101UpdatePresentation(void *context, void *data, void *vertices,
                                  s32 step) {
    s32 state;

    if (gOverlay101PresentationActive != 0) {
        overlay101StopFirstReloc(step);
        overlay101StopSecondReloc(step);
        overlay101StopThirdReloc(step);
        overlay101DrawPresentationReloc(context, data, vertices);
        overlay101CreatePresentationReloc(
            context, &gOverlay101Handle1D4, (f32)gOverlay101GlobalX,
            (f32)gOverlay101GlobalY, 1.0f, 1.0f, -2, 3);
        state = gOverlay101PresentationDone;
    } else {
        state = gOverlay101PresentationDone;
        if (state == 0) {
            gOverlay101PresentationTimer += step;
            if (gOverlay101PresentationTimer >= 0xD3) {
                overlay101AdvancePresentationReloc();
                gOverlay101PresentationTimer = 0;
                state = gOverlay101PresentationDone;
            }
        }
    }

    if (state == 0) {
        if (gOverlay101InputMode != 0) {
            if (overlay101ReadInputReloc(0) & 0x9000) {
                overlay101FinishPresentationReloc();
                gOverlay101PresentationDone = 1;
            }
        } else if ((overlay101ReadInputReloc(0) & 0x820) == 0x820) {
            overlay101FinishPresentationReloc();
            gOverlay101PresentationDone = 1;
        }
    }
}
