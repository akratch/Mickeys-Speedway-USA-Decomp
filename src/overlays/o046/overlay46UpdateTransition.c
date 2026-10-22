#include "PR/ultratypes.h"

extern u8 gOverlay46DisplayState[];
extern u8 gOverlay46DisplayOutput[];
extern s32 gOverlay46Mode58;
extern s32 gOverlay46Timer5C;
extern s16 gOverlay46Selection;
extern s32 gOverlay46FadeOutput;

extern void camSetView(void *state, s32 mode);
extern void func_80022B94(void *state, void *output);
extern s32 overlay41IsUnitScale(s32 index);
extern void func_80028D30(s32 index);
extern void frontDrawObj(s32 index);
extern void func_overlay_046_F0001228_188F620(s32 updateRate);

/* Pinned DKR v77/v80 and JFG scans found no donor inside this owner. */
void overlay46UpdateTransition(s32 updateRate) {
    s32 finished;

    finished = 0;
    camSetView(gOverlay46DisplayState, 0);
    func_80022B94(gOverlay46DisplayState, gOverlay46DisplayOutput);

    switch (gOverlay46Mode58) {
        case 1:
            if (gOverlay46Timer5C > 0) {
                gOverlay46Timer5C -= updateRate;
            }
            if (overlay41IsUnitScale(0) != 0) {
                gOverlay46Mode58 = 6;
                gOverlay46Timer5C = 270;
            }
            break;
        case 6:
            gOverlay46Timer5C -= updateRate;
            if (gOverlay46Timer5C <= 0) {
                gOverlay46Mode58 = 0;
                func_80028D30(0);
            }
            finished = 1;
            break;
        case 0:
            finished = 1;
            break;
    }

    if (finished != 0) {
        gOverlay46Selection += updateRate * 2;
        if (gOverlay46Selection >= 256) {
            gOverlay46Selection = 255;
        }
        gOverlay46FadeOutput = gOverlay46Selection;
        frontDrawObj(5);
        gOverlay46FadeOutput = 255;
    }

    func_overlay_046_F0001228_188F620(updateRate);
}
