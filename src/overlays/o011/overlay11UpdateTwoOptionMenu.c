#include "PR/ultratypes.h"

extern s8 D_0[];
extern s8 D_0_reload_success[];
extern s8 D_0_reload_failure[];
extern s8 D_INPUT[];
extern s8 D_menuBase[];
extern s8 D_menuCounterBase[];
extern s8 D_menuInputBase[];
extern s32 D_1BC;
extern void *D_1CC[2];

#define OVERLAY11_SELECTION (*(s16 *)(D_menuBase + 0x1B8))
#define OVERLAY11_ARGUMENT (*(s32 *)(D_INPUT + 0x1C4))
#define OVERLAY11_COUNTER (*(s32 *)(D_menuCounterBase + 0x204))
/* The menu input word in initialized data, distinct from the argument word
   OVERLAY11_ARGUMENT reads at the same offset past the BSS base. */
#define OVERLAY11_MENU_INPUT (*(s32 *)(D_menuInputBase + 0x1C4))

extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void func_80028528(s32 arg0);
extern void func_800290AC(s32 arg0);
extern void func_800291D8(s32 arg0);
extern void func_800006BC(f32 arg0, s32 arg1);
extern void func_overlay_045_F0001BF4_188E04C(void *handle, s32 value);
extern void func_overlay_066_F0000000(void *arg0);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F00029AC_186B1F4(void);
extern void func_overlay_011_F0002BF4_186B43C(void);

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 11 as none. */
void overlay11UpdateTwoOptionMenu(s32 updateRate) {
    s32 index;
    s8 direction;
    s16 value;
    void **handle;

    direction = D_0[OVERLAY11_ARGUMENT];
    if (direction < -32) {
        if (D_1BC < 2) {
            D_1BC++;
            func_80000F94(0x32C, 0);
            direction = D_0_reload_success[OVERLAY11_ARGUMENT];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_0_reload_failure[OVERLAY11_ARGUMENT];
        }
    }
    if (direction >= 33) {
        if (D_1BC >= 2) {
            D_1BC--;
            func_80000F94(0x32C, 0);
        } else {
            func_80000F94(0x32D, 0);
        }
    }

    handle = D_1CC;
    index = 1;
    do {
        value = (index == D_1BC) ? OVERLAY11_SELECTION : 0;
        func_overlay_045_F0001BF4_188E04C(*handle, value);
        handle++;
        index++;
    } while (index != 3);

    if ((func_8002554C(OVERLAY11_ARGUMENT) & 0x8000) ||
        OVERLAY11_MENU_INPUT != 0) {
        switch (D_1BC) {
        case 1:
            func_overlay_066_F0000000(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_overlay_011_F0002BF4_186B43C();
            OVERLAY11_COUNTER = 1;
            break;
        case 2:
            if (OVERLAY11_MENU_INPUT == 0) {
                func_overlay_011_F0001058_18698A0(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == -1) {
                func_overlay_011_F0001130_1869978(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == 1) {
                func_80028528(1);
                func_overlay_011_F00029AC_186B1F4();
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                OVERLAY11_COUNTER = 1;
            }
            break;
        }
    }
}
