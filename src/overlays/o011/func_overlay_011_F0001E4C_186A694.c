#include "PR/ultratypes.h"

typedef struct O11StatusSlot {
    u8 mode;
    u8 pad01[7];
    s32 field8;
    u8 pad0C[0x1C];
} O11StatusSlot;

typedef struct O11ObjectSub {
    u8 pad0;
    s8 value1;
} O11ObjectSub;

typedef struct O11Object {
    u8 pad00[0x64];
    O11ObjectSub *sub64;
} O11Object;

extern s8 D_0[];
extern s8 D_0_reload_success[];
extern s8 D_0_reload_failure[];
extern u8 D_INPUT[];
extern u8 D_menuBase[];
extern s32 D_1BC;
extern void *D_1CC[5];
extern s32 D_state;
extern u8 D_stateFlag;
extern s8 D_cfgA;
extern s8 D_cfgB;

#define O11_SELECTION (*(s16 *)(D_menuBase + 0x1B8))
#define O11_ARGUMENT (*(s32 *)(D_INPUT + 0x1C4))
#define O11_COUNTER (*(s32 *)(D_menuBase + 0x204))

extern O11StatusSlot *func_overlay_011_F0000000_1868848(void);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F0002A74_186B2BC(void);
extern void func_overlay_011_F0002BF4_186B43C(void);
extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
extern O11Object *func_80005820(s32 controller);
extern s32 func_8002675C(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void func_80028528(s32 arg0);
extern void func_800290AC(s32 arg0);
extern void func_800291D8(s32 arg0);
extern void func_800006BC(f32 arg0, s32 arg1);
extern void func_overlay_045_F0001BF4_188E04C(void *handle, s32 value);
extern void func_overlay_066_F0000000(void *arg0);

/* Workbench: size 0, frame 0x40, 8=8 slots, 63 masked / 68 raw, first +0xD8.
 * Remaining 2+2 inserts: occupancy beq vs addiu 4, addiu 2 schedule. */
#ifdef NON_MATCHING
void func_overlay_011_F0001E4C_186A694(s32 updateRate) {
    s32 index;
    s8 direction;
    O11ObjectSub *sub;
    s16 value;
    O11StatusSlot *status;
    s32 action;
    void **handle;
    s32 state;
    s32 transition;
    volatile s32 *menuInput;

    status = func_overlay_011_F0000000_1868848();
    direction = D_0[O11_ARGUMENT];
    if (direction < -32) {
        if (D_1BC < 5) {
            D_1BC++;
            func_80000F94(0x32C, 0);
            direction = D_0_reload_success[O11_ARGUMENT];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_0_reload_failure[O11_ARGUMENT];
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

    for (handle = D_1CC, index = 1; index < 6; handle++, index++) {
        value = (index == D_1BC) ? O11_SELECTION : 0;
        func_overlay_045_F0001BF4_188E04C(*handle, value);
    }

    menuInput = (volatile s32 *)(D_menuBase + 0x1C4);
    if ((func_8002554C(O11_ARGUMENT) & 0x8000) || *menuInput != 0) {
        switch (D_1BC) {
        case 1:
            func_overlay_066_F0000000(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_overlay_011_F0002BF4_186B43C();
            O11_COUNTER = 1;
            break;
        case 2:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == 1) {
                state = D_state;
                if (((state == 2) || (state == 3)) && (D_stateFlag != 0)) {
                    D_cfgB = 4 - state;
                    D_cfgA = 4;
                } else {
                    D_cfgB = 0;
                    D_cfgA = state;
                }
                sub = func_80005820(O11_ARGUMENT)->sub64;
                O11_COUNTER = 1;
                /* || of consecutive 5/6 range-folds; volatile keeps both tests.
                 * index=2 plus empty-if/OR-zero (L109) keeps the 0x28 scale
                 * without spilling action. */
                if ((*(volatile u8 *)&status->mode == 5) ||
                    (*(volatile u8 *)&status->mode == 6)) {
                    status[0].field8 = 0;
                    status[1].field8 = 0;
                    index = 2;
                    if (index) {
                    }
                    index |= 0;
                    status[index + 1].field8 = 0;
                    status[index + 2].field8 = 0;
                    status[index + 3].field8 = 0;
                    status[index].field8 = 0;
                }
                transition = func_8002675C();
                func_80028374(transition, sub->value1, 0, 5, 1, 0);
            }
            break;
        case 3:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == 1) {
                func_80028528(1);
                func_80028374(0x1D, 0, 0, 0xB, 1, 0);
                O11_COUNTER = 1;
            }
            break;
        case 4:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == 1) {
                O11_COUNTER = 1;
                func_80028528(1);
                if (status->mode == 5) {
                    func_80028374(0xC, 0, 0, 0x12, 1, 0);
                } else {
                    func_80028374(0xC, 0, 0, 0x11, 1, 0);
                }
            }
            break;
        case 5:
            action = *menuInput;
            if (action == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (action == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (action == 1) {
                O11_COUNTER = 1;
                func_overlay_011_F0002A74_186B2BC();
                func_80028528(1);
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
            }
            break;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o011/func_overlay_011_F0001E4C_186A694/func_overlay_011_F0001E4C_186A694.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_011_F0001E4C_186A694:start
 * symbol: func_overlay_011_F0001E4C_186A694
 * score: 63/295 words
 * frame: 0x40
 * relocations: 87
 * first-mismatch: +0xD8
 * summary: Size 0, 8=8 slots, masked 63. Occupancy beq vs addiu 4; addiu 2 scheduled early. Colour still L155-shadowed by 2+2 inserts.
 * PLATEAU-HANDOFF:func_overlay_011_F0001E4C_186A694:end
 */
