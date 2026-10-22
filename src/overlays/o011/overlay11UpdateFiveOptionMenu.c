#include "PR/ultratypes.h"

typedef struct O11Sub {
    u8 pad0;
    s8 value1;
} O11Sub;

typedef struct O11Object {
    u8 pad00[0x64];
    O11Sub *sub64;
} O11Object;

extern s8 D_0[];
extern s8 D_0_reload_success[];
extern s8 D_0_reload_failure[];
extern u8 D_menuBase[];
extern s16 D_1B8;
extern s32 D_1BC;
extern s32 D_1C4;
extern void *D_1CC[5];
extern s32 D_204;
extern s8 D_8007BEF8;
extern s8 D_8007BEFC;
extern s8 D_8007BF0C;
extern s32 D_lastMode;

extern void func_80000F94(s32, void *);
extern u32 func_8002554C(s32);
extern O11Object *func_80005820(s32);
extern s32 func_8002675C(void);
extern void func_80028374(s32, s32, s32, s32, s32, s32);
extern void func_800290AC(s32);
extern void func_800291D8(s32);
extern void func_800006BC(f32, s32);
extern void func_overlay_045_F0001BF4_188E04C(void *, s32);
extern void func_overlay_066_F0000000(void *);
extern void func_overlay_011_F0001058_18698A0(s32);
extern void func_overlay_011_F0001130_1869978(s32);
extern void func_overlay_011_F0002A10_186B258(void);
extern void func_overlay_011_F0002BF4_186B43C(void);

void overlay11UpdateFiveOptionMenu(s32 updateRate) {
    s32 index;
    s8 direction;
    s16 value;
    void **handle;
    void *action;
    O11Object *object;
    O11Sub *sub;
    s32 transition;
    void * volatile *menuInput;

    direction = D_0[D_1C4];
    if (direction < -32) {
        if (D_1BC < 5) {
            D_1BC++;
            func_80000F94(0x32C, 0);
            direction = D_0_reload_success[D_1C4];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_0_reload_failure[D_1C4];
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
        value = (index == D_1BC) ? D_1B8 : 0;
        func_overlay_045_F0001BF4_188E04C(*handle, value);
        handle++;
        index++;
    } while (index != 6);

    menuInput = (void * volatile *)(D_menuBase + 0x1C4);
    if ((func_8002554C(D_1C4) & 0x8000) || *menuInput != 0) {
        switch (D_1BC) {
        case 1:
            func_overlay_066_F0000000(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_overlay_011_F0002BF4_186B43C();
            D_204 = 1;
            break;
        case 2:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == (void *)-1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == (void *)1) {
                D_8007BEF8 = 1;
                D_8007BEFC = 0;
                D_8007BF0C = 0;
                object = func_80005820(D_1C4);
                sub = object->sub64;
                transition = func_8002675C();
                func_80028374(transition, sub->value1, 0, 5, 1, 0);
                D_204 = 1;
            }
            break;
        case 3:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == (void *)-1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == (void *)1) {
                func_80028374(0x1D, 0, 0, 0xB, 1, 0);
                D_204 = 1;
            }
            break;
        case 4:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == (void *)-1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == (void *)1) {
                func_80028374(0xC, 0, 0, 0x11, 1, 0);
                D_204 = 1;
            }
            break;
        case 5:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == (void *)-1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == (void *)1) {
                func_overlay_011_F0002A10_186B258();
                D_lastMode = 5;
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                D_204 = 1;
            }
            break;
        }
    }
}
