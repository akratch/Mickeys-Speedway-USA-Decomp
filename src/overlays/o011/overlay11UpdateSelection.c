#include "PR/ultratypes.h"

extern s8 D_800D3190[];
extern s16 gOverlay11Selection;
extern s32 gOverlay11Mode;
extern s32 gOverlay11Argument;
extern void *gOverlay11Handle2;
extern void *gOverlay11Handle3;

extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
extern void func_overlay_045_F0001BF4_188E04C(void *handle, s32 value);

/* DKR v77/v80 and JFG contain no matching state/input topology. */
void overlay11UpdateSelection(s32 updateRate) {
    s8 direction;
    s32 mode;

    direction = D_800D3190[gOverlay11Argument];
    if (direction < -32) {
        if (gOverlay11Mode == 3) {
            gOverlay11Mode--;
            func_80000F94(0x32C, 0);
            direction = D_800D3190[gOverlay11Argument];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_800D3190[gOverlay11Argument];
        }
    }
    if (direction >= 33) {
        if (gOverlay11Mode == 2) {
            gOverlay11Mode++;
            func_80000F94(0x32C, 0);
        } else {
            func_80000F94(0x32D, 0);
        }
    }

    mode = gOverlay11Mode;
    if (mode == 2) {
        func_overlay_045_F0001BF4_188E04C(gOverlay11Handle2,
                                          gOverlay11Selection);
        func_overlay_045_F0001BF4_188E04C(gOverlay11Handle3, 0);
    } else if (mode == 3) {
        func_overlay_045_F0001BF4_188E04C(gOverlay11Handle2, 0);
        func_overlay_045_F0001BF4_188E04C(gOverlay11Handle3,
                                          gOverlay11Selection);
    }

    if (func_8002554C(gOverlay11Argument) & 0x4000) {
        gOverlay11Mode = 0;
        gOverlay11Argument = -1;
        return;
    }
    if (func_8002554C(gOverlay11Argument) & 0x8000) {
        mode = gOverlay11Mode;
        if (mode == 2) {
            gOverlay11Mode = 0;
            gOverlay11Argument = 1;
            return;
        }
        if (mode == 3) {
            gOverlay11Mode = 0;
            gOverlay11Argument = -1;
        }
    }
}
