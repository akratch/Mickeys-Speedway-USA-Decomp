#include "PR/ultratypes.h"

typedef struct O11Status {
    u8 value0;
    u8 value1;
    u8 value2;
    u8 mode;
} O11Status;

typedef struct O11ObjectSub {
    u8 pad0;
    s8 value1;
} O11ObjectSub;

typedef struct O11Object {
    u8 pad00[0x64];
    O11ObjectSub *sub64;
} O11Object;

extern s8 D_800D3194[];
extern u8 D_menuBase[];
extern s16 D_1B8;
extern s32 D_1BC;
extern s32 D_1C4;
extern void *D_1CC[3];
extern s32 D_204;

extern s8 D_8007BEF8;
extern s8 D_8007BEFC;
extern s8 D_8007BF0C;
extern u16 D_8007BF1C;
extern s32 D_8007BF44;
extern s16 D_table[][4];
extern s32 D_paramA;
extern s32 D_paramB;
extern s32 D_paramC;
extern s32 D_paramD;
extern s32 D_8007C1A0;

extern O11Status *func_80028F54(void);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F0002948_186B190(void);
extern void func_overlay_011_F0002BF4_186B43C(void);
extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
extern O11Object *func_80005820(s32 controller);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void func_80028528(s32 arg0);
extern void func_8003A754(void);
extern void func_800290AC(s32 arg0);
extern void func_800291D8(s32 arg0);
extern void func_800006BC(f32 arg0, s32 arg1);
extern void func_overlay_045_F0001BF4_188E04C(void *handle, s32 value);
extern void overlay66Select(s32 selection);

/*
 * The match is two facts, and each is a regression without the other.
 *
 * 1. The `+0x138` spill pair. `as1` decides a call's delay-slot filler by
 *    memory disambiguation against `$sp`. Without a fact for the argument
 *    load's base register, both caller-save spill stores hang off that load,
 *    scheduling it releases them together, they enter the ready list in
 *    reverse emission order and the later one wins -- which is the inversion
 *    this function showed for three passes. `uopt` emits the fact,
 *    `.noalias $3,$sp`, at an *indexed* reference whose base it knows is an
 *    array address, and not for a walking pointer it merely copied. Routing
 *    the offset through a pointer variable -- `handle = D_1CC - 1` indexed by
 *    the counter -- is what gets the fact at displacement zero, where every
 *    other pointer-arithmetic spelling folds to base-plus-four with a minus-
 *    four displacement.
 *
 * 2. The preheader order. The indexed form makes `uopt` create a strength-
 *    reduced induction pointer, and it appends that pointer's preheader
 *    initialisation after every user preheader statement, so a separate
 *    `index = 1;` statement puts the counter's `li` first where the target
 *    materialises the address first. `as1` re-schedules that block anyway --
 *    the `lui` moves above the `li` in both -- and it breaks the remaining tie
 *    by source line, not by emission index. Putting the counter's
 *    initialisation in the `for` header gives it the loop statement's own
 *    line, the same line the induction initialiser carries, and the tie then
 *    falls to the address. Emission order is unchanged; only the `.loc` is.
 *
 * The declaration list is load-bearing for the four live stack homes. The
 * frame is `align8(28 + declared block + 4 * pooled temporaries)` and the
 * pooled induction pointer takes cell one, so the seven declared words run
 * `index` at 68 down to `finish` at 44 in declaration order, with the pointer
 * at 36 and frame `0x48`. Reordering the declarations or reintroducing the
 * `selection`, `action`, `object` and `value` locals that were folded away
 * moves `status` and `finish` and costs sixteen words.
 *
 * Pinned exact-overlay DKR v77/v80 and JFG scans found no exact donor.
 */
void overlay11UpdateMenu(s32 updateRate) {
    s32 index;
    s8 direction;
    O11ObjectSub *sub;
    volatile s32 *menuInput;
    void **handle;
    O11Status *status;
    s32 finish;

    status = func_80028F54();
    direction = D_800D3194[D_1C4];
    if (direction < -32) {
        if (((status->mode >= 2) && (D_1BC < 3)) ||
            ((status->mode == 1) && (D_1BC < 2))) {
            D_1BC++;
            func_80000F94(0x32C, 0);
            direction = D_800D3194[D_1C4];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_800D3194[D_1C4];
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

    handle = D_1CC - 1;
    for (index = 1; index != 4; index++) {
        func_overlay_045_F0001BF4_188E04C(handle[index],
                                          (index == D_1BC) ? D_1B8 : 0);
    }

    menuInput = (s32 *)(D_menuBase + 0x1C4);
    if ((func_8002554C(D_1C4) & 0x8000) || *menuInput != 0) {
        finish = 0;
        index = D_1BC;
        switch (index) {
        case 1:
            overlay66Select(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_overlay_011_F0002BF4_186B43C();
            D_204 = 1;
            break;
        case 2:
            index = *menuInput;
            if (index == 0) {
                func_overlay_011_F0001058_18698A0(4);
            } else if (index == -1) {
                func_overlay_011_F0001130_1869978(4);
            } else if (index == 1) {
                D_8007BEF8 = 6;
                D_8007BEFC = 5;
                D_8007BF0C = 0;
                sub = func_80005820(D_1C4)->sub64;
                if ((status->value1 != 0) && !(D_8007BF1C & 0x100)) {
                    status->mode--;
                }
                if (status->mode == 0) {
                    finish = 1;
                } else {
                    if (D_8007BF44 > 0) {
                        D_paramA = D_table[status->value2][status->value1];
                        D_paramB = sub->value1;
                        D_paramC = 5;
                        D_paramD = 0;
                        func_80028374(0x12, 0, 0, 0xF, 1, 0);
                        func_80028528(1);
                    } else {
                        func_80028374(D_table[status->value2][status->value1],
                                     sub->value1, 0, 5, 1, 0);
                    }
                    D_204 = 1;
                }
            }
            break;
        case 3:
            index = *menuInput;
            if (index == 0) {
                func_overlay_011_F0001058_18698A0(4);
            } else if (index == -1) {
                func_overlay_011_F0001130_1869978(4);
            } else if (index == 1) {
                if (D_8007C1A0 == 1) {
                    func_8003A754();
                }
                func_overlay_011_F0002948_186B190();
                finish = 1;
            }
            break;
        }

        if (finish != 0) {
            if (D_8007BF44 > 0) {
                sub = func_80005820(D_1C4)->sub64;
                D_paramA = 0xC;
                D_paramB = sub->value1;
                D_paramC = 0xC;
                D_paramD = 1;
                func_80028374(0x12, 0, 0, 0xF, 1, 0);
                func_80028528(1);
            } else {
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
            }
            D_204 = 1;
        }
    }
}
