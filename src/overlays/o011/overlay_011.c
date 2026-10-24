#include "overlays/overlay_011.h"

/*
 * Overlay 11: the front-end menus. One translation unit; only
 * overlay11Initialize, the menu background draw and
 * overlay11ReleaseCurrentGroup are exported, and every other function is
 * static.
 */

static void overlay11CreateHandles(void);
static void overlay11InitializeFour(void);
static void overlay11InitializeSixB(void);
static void overlay11InitializeThreeA(void);
static void overlay11InitializeSixA(void);
static void overlay11InitializeSixC(void);
static void overlay11InitializeThreeB(void);
static void overlay11EnableHandles(s32 count);
static void overlay11DisableHandles(s32 count);
static void overlay11UpdateSelection(s32 updateRate);
static void overlay11UpdateMenu(s32 updateRate);
static void overlay11UpdateTwoOptionMenu(s32 updateRate);
static void overlay11UpdateFiveOptionMenu(s32 updateRate);
static void func_overlay_011_F0001E4C_186A694(s32 updateRate);
static void func_overlay_011_F00022E8_186AB30(s32 updateRate);
static void overlay11UpdateModeSix(s32 updateRate);
static void overlay11ReleaseGroup4(void);
static void overlay11ReleaseGroup3A(void);
static void overlay11ReleaseGroup6A(void);
static void overlay11ReleaseGroup6B(void);
static void overlay11ReleaseGroup6C(void);
static void overlay11ReleaseGroup3B(void);
static void overlay11ReleaseHandles(void);

/* DKR v77/v80 and JFG contain no matching Overlay 11 initializer donor. */
void overlay11Initialize(s32 argument) {
    Overlay11Status *status;

    func_800290AC(1);
    amTuneSetFade(0.5f, 0);
    gOverlay11Argument = argument;
    gOverlay11Timer = 30;
    gOverlay11Counter = 0;
    gOverlay11Selection = 0;
    overlay66Select(1);
    status = func_80028F54();
    if (gOverlay11Initialized == 0) {
        fontUseFont(3);
        func_8004B0B8(255, 255, 255, 0, 255);
        switch (status->mode) {
        case 0:
            overlay11InitializeFour();
            break;
        case 1:
            overlay11InitializeSixB();
            break;
        case 3:
            overlay11InitializeThreeA();
            break;
        case 4:
        case 5:
            if (D_8007C1A0 == 1) {
                overlay11InitializeSixC();
            } else {
                overlay11InitializeSixA();
            }
            break;
        case 2:
            overlay11InitializeThreeB();
            break;
        }
        gOverlay11Argument = 0;
        overlay11CreateHandles();
        gOverlay11Initialized = 1;
    }
}

/* Matched 2026-10-02 from 534 masked words. What it took, in the order it
 * was found:
 * - the vertex writer without its `do { } while (0)` wrapper (above), and
 *   the seven rows of a column as ONE loop from the top row: IDO unrolls it
 *   by four after a three-row remainder, which is the shipped shape (the
 *   main pass indexes from a constant 3);
 * - every packet as `pointer = list++` then two stores; the two header
 *   packets and the column packets each have their own pointer local (one
 *   shared local is one web and loses v0 to the row y);
 * - x, the top y and the row y are plain s32 locals, and the row y is
 *   reset at the TOP of the column body, before the packets: that
 *   assignment is the dead copy the shipped column loop keeps;
 * - the row is indexed `alpha[row + block * 6]` (row first), the address
 *   byte is masked with 0xFF before the shift, and the switch lists mode 2
 *   last;
 * - ten scalar locals in this order put alpha, width, height and the two
 *   spill cells at the shipped homes (`unused` is the tenth);
 * - L59: the second header packet's two stores share a line, and so do the
 *   column packet's pointer step and first word.
 * The two 0.05f literals are spelled differently because the shipped pool
 * holds two entries (L103). */
void func_overlay_011_F0000150_1868998(Overlay11Gfx **displayList, void **matrix,
                                        Overlay11Vertex **vertices,
                                        s32 updateRate) {
    s32 row;
    s32 column;
    s32 block;
    Overlay11Gfx *command;
    s32 x;
    u8 alpha[13][17];
    s32 top;
    u32 width;
    u32 height;
    s32 y;
    Overlay11Gfx *scissor;
    Overlay11Gfx *combine;
    s32 unused;

    if (func_800290A0_o011Reloc() != 0) {
        if (gOverlay11Fade == 0.0f) {
            for (row = 0; row < 13; row++) {
                for (column = 0; column < 17; column++) {
                    gOverlay11GridPhases[row][column] =
                        mathRnd_o011Reloc(-0x8000, 0x7FFF);
                }
            }
        }
        gOverlay11Fade += 0.05f * (f32)updateRate;
        if (gOverlay11Fade > 1.0f) {
            gOverlay11Fade = 1.0f;
        }
    } else {
        gOverlay11Fade -= .05f * (f32)updateRate;
        if (gOverlay11Fade <= 0.0f) {
            gOverlay11Fade = 0.0f;
            return;
        }
    }

    D_800D3140 = *displayList;
    D_800D3144 = *matrix;
    D_800D3148 = *vertices;

    viGetCurrentSize_o011Reloc(&width, &height);
    scissor = D_800D3140++;
    scissor->w0 = 0xED000000;
    scissor->w1 =
        ((((s32)((width - 1) * 4.0f)) & 0xFFF) << 12) |
        (((s32)((height - 1) * 4.0f)) & 0xFFF);
    camStandardOrtho_o011Reloc(&D_800D3140, &D_800D3144);
    texDPTextureX_o011Reloc(&D_800D3140, 0, 4, 0);
    combine = D_800D3140++;
    combine->w0 = 0xFCFFFFFF; combine->w1 = 0xFFFE793C;

    for (row = 0; row < 13; row++) {
        for (column = 0; column < 17; column++) {
            gOverlay11GridPhases[row][column] += updateRate << 8;
            alpha[row][column] =
                (u8)((func_8002A8C0_o011Reloc(gOverlay11GridPhases[row][column]) * 32.0f +
                      96.0f) *
                     gOverlay11Fade);
        }
    }

    for (block = 0; block < 2; block++) {
        x = -160;
        top = 120 - block * 120;
        for (column = 0; column < 17; column++) {
            y = top;
            command = D_800D3140++; command->w0 = 0x04000000 | ((((((u32)D_800D3148 + 0x80000000) & 6) | 0x38) & 0xFF) << 16) | ((((column % 2 * 7) << 9) | 0x4E) & 0xFFFF);
            command->w1 = (u32)D_800D3148 + 0x80000000;
            if (column != 0) {
                command = D_800D3140++;
                command->w0 = 0x05B100C0;
                command->w1 =
                    (u32)(gOverlay11GridTriangles + (column % 2 * 0xC0)) + 0x80000000;
            }
            for (row = 0; row < 7; row++) {
                O11_WRITE_VERTEX(x, y, alpha[row + block * 6][column]);
                y -= 20;
            }
            x += 20;
        }
    }

    if ((gOverlay11InputDelay == 0) && (gOverlay11OptionDone == 0)) {
        if ((gOverlay11Mode == 0) && (gOverlay11OptionAction == 0) &&
            (joyGetPressed_o011Reloc(gOverlay11PadIndex) & 0x5000)) {
            overlay66Select(0);
            func_800290AC_o011Reloc(0);
            func_800291D8_o011Reloc(0x1E);
            amTuneSetFadeScaled_o011Reloc(0.5f, 0x7F);
            overlay11ReleaseCurrentGroup_o011Reloc();
            gOverlay11OptionDone = 1;
        } else {
            gOverlay11OptionHighlight += gOverlay11HighlightStep * updateRate;
            if (gOverlay11OptionHighlight >= 0x100) {
                gOverlay11OptionHighlight = 0x1FF - gOverlay11OptionHighlight;
                gOverlay11HighlightStep = -gOverlay11HighlightStep;
            } else if (gOverlay11OptionHighlight < 0) {
                gOverlay11OptionHighlight = -gOverlay11OptionHighlight;
                gOverlay11HighlightStep = -gOverlay11HighlightStep;
            }
            if (gOverlay11Mode != 0) {
                overlay11UpdateSelection(updateRate);
            } else {
                switch (func_80028F54_o011Reloc()->mode) {
                case 0:
                    overlay11UpdateMenu(updateRate);
                    break;
                case 1:
                    overlay11UpdateFiveOptionMenu(updateRate);
                    break;
                case 3:
                    overlay11UpdateTwoOptionMenu(updateRate);
                    break;
                case 4:
                case 5:
                    if (D_8007C1A0 == 1) {
                        func_overlay_011_F00022E8_186AB30(updateRate);
                    } else {
                        func_overlay_011_F0001E4C_186A694(updateRate);
                    }
                    break;
                case 2:
                    overlay11UpdateModeSix(updateRate);
                    break;
                }
            }
        }
    }

    if (gOverlay11InputDelay != 0) {
        gOverlay11InputDelay -= updateRate;
        if (gOverlay11InputDelay < 0) {
            gOverlay11InputDelay = 0;
        }
    }

    *displayList = D_800D3140;
    *matrix = D_800D3144;
    *vertices = D_800D3148;
}

#undef O11_WRITE_VERTEX

/* DKR v77/v80 contain only generic fixed-resource creation sequences. */
static void overlay11CreateHandles(void) {
    gOverlay11Handles[0] = overlay45CreateDescriptor(D_8007C0B8->resource2D0,
                                                160, 40, 4);
    gOverlay11Handles[1] = overlay45CreateDescriptor(gOverlay11DirectResource,
                                                160, 120, 4);
    gOverlay11Handles[2] = overlay45CreateDescriptor(D_8007C0B8->resource58,
                                                120, 120, 4);
    gOverlay11Handles[3] = overlay45CreateDescriptor(D_8007C0B8->resource5C,
                                                200, 120, 4);
    overlay45SetMode(gOverlay11Handles[0], 0);
    overlay45SetMode(gOverlay11Handles[1], 0);
    overlay45SetMode(gOverlay11Handles[2], 0);
    overlay45SetMode(gOverlay11Handles[3], 0);
    gOverlay11Mode = 3;
}

/* DKR v77/v80 and JFG contain no matching Overlay 11 initializer donor. */
static void overlay11InitializeFour(void) {
    char buffer[80];
    Overlay11Status *status;

    status = func_80028F54();
    if (status->value3 == 2) {
        sprintf(buffer, gOverlay11Format, D_8007C0B8->value190);
    } else {
        sprintf(buffer, D_8007C0B8->format194, status->value3 - 1);
    }

    gOverlay11Created[0] = overlay45CreateDescriptor(
        D_8007C0B8->resource198, 160, 40, 4);
    gOverlay11Created[1] = overlay45CreateDescriptor(
        D_8007C0B8->resource19C, 160, 100, 4);

    if (status->value3 == 1) {
        gOverlay11Created[2] = overlay45CreateDescriptor(
            D_8007C0B8->resource50, 160, 135, 4);
        gOverlay11Created[3] = 0;
    } else {
        if (D_8007BF1C & 0x100) {
            gOverlay11Created[2] = overlay45CreateDescriptor(
                D_8007C0B8->resource1A8, 160, 135, 4);
        } else if (status->value1 == 0) {
            gOverlay11Created[2] = overlay45CreateDescriptor(
                D_8007C0B8->resource1A0, 160, 135, 4);
        } else {
            gOverlay11Created[2] =
                overlay45CreateDescriptor(buffer, 160, 135, 4);
        }
        gOverlay11Created[3] = overlay45CreateDescriptor(
            D_8007C0B8->resource50, 160, 170, 4);
    }
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
static void overlay11InitializeSixB(void) {
    gOverlay11Created[0] = overlay45CreateDescriptor(D_8007C0B8->resource1A4, 160, 40, 4);
    gOverlay11Created[1] = overlay45CreateDescriptor(D_8007C0B8->resource19C, 160, 80, 4);
    gOverlay11Created[2] = overlay45CreateDescriptor(D_8007C0B8->resource1A8, 160, 110, 4);
    gOverlay11Created[3] = overlay45CreateDescriptor(D_8007C0B8->resource94, 160, 140, 4);
    gOverlay11Created[4] = overlay45CreateDescriptor(D_8007C0B8->resource98, 160, 170, 4);
    gOverlay11Created[5] = overlay45CreateDescriptor(D_8007C0B8->resource50, 160, 200, 4);
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
static void overlay11InitializeThreeA(void) {
    gOverlay11Created0 = overlay45CreateDescriptor(D_8007C0B8->resource1AC, 160, 40, 4);
    gOverlay11Created1 = overlay45CreateDescriptor(D_8007C0B8->resource19C, 160, 110, 4);
    gOverlay11Created2 = overlay45CreateDescriptor(D_8007C0B8->resource50, 160, 140, 4);
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
static void overlay11InitializeSixA(void) {
    gOverlay11Created[0] = overlay45CreateDescriptor(
        D_8007C0B8->resource1AC, 160, 40, 4);
    gOverlay11Created[1] = overlay45CreateDescriptor(
        D_8007C0B8->resource19C, 160, 80, 4);
    gOverlay11Created[2] = overlay45CreateDescriptor(
        D_8007C0B8->resource1A8, 160, 110, 4);
    gOverlay11Created[3] = overlay45CreateDescriptor(
        D_8007C0B8->resource94, 160, 140, 4);
    gOverlay11Created[4] = overlay45CreateDescriptor(
        D_8007C0B8->resource98, 160, 170, 4);
    gOverlay11Created[5] = overlay45CreateDescriptor(
        D_8007C0B8->resource50, 160, 200, 4);
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
static void overlay11InitializeSixC(void) {
    gOverlay11Created[0] = overlay45CreateDescriptor(D_8007C0B8->resource284, 160, 40, 4);
    gOverlay11Created[1] = overlay45CreateDescriptor(D_8007C0B8->resource19C, 160, 80, 4);
    gOverlay11Created[2] = overlay45CreateDescriptor(D_8007C0B8->resource1A8, 160, 110, 4);
    gOverlay11Created[3] = overlay45CreateDescriptor(D_8007C0B8->resource94, 160, 140, 4);
    gOverlay11Created[4] = overlay45CreateDescriptor(D_8007C0B8->resource98, 160, 170, 4);
    gOverlay11Created[5] = overlay45CreateDescriptor(D_8007C0B8->resource50, 160, 200, 4);
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
static void overlay11InitializeThreeB(void) {
    gOverlay11Created0 = overlay45CreateDescriptor(D_8007C0B8->resource1C4, 160, 40, 4);
    gOverlay11Created1 = overlay45CreateDescriptor(D_8007C0B8->resource19C, 160, 110, 4);
    gOverlay11Created2 = overlay45CreateDescriptor(D_8007C0B8->resource50, 160, 140, 4);
    gOverlay11CreatedActive = 1;
}

/* DKR v77/v80 contain only generic fixed-handle update loops. */
static void overlay11EnableHandles(s32 count) {
    s32 i;

    gOverlay11ModeStart = 1;
    overlay45SetMode(gOverlay11Handles[0], 0xFF);
    overlay45SetMode(gOverlay11Handles[1], 0xFF);
    overlay45SetMode(gOverlay11Handles[3], 0xFF);
    overlay45SetMode(gOverlay11Handles[2], 0xFF);
    overlay45SetField22(gOverlay11Handles[0], 0);
    overlay45SetField22(gOverlay11Handles[1], 0);
    overlay45SetField22(gOverlay11Handles[3], 0);
    overlay45SetField22(gOverlay11Handles[2], 0);
    gOverlay11ModeFinish = 3;
    for (i = 0; i < count; i++) {
        overlay45SetMode(gOverlay11Created[i], 0);
    }
}

/* DKR v77/v80 contain only generic fixed-handle update loops. */
static void overlay11DisableHandles(s32 count) {
    s32 i;

    gOverlay11Flag = 0;
    gOverlay11Mode = 0;
    overlay45SetMode(gOverlay11Handles[0], 0);
    overlay45SetMode(gOverlay11Handles[1], 0);
    overlay45SetMode(gOverlay11Handles[3], 0);
    overlay45SetMode(gOverlay11Handles[2], 0);
    for (i = 0; i < count; i++) {
        overlay45SetMode(gOverlay11Created[i], 0xFF);
    }
}

/* DKR v77/v80 and JFG contain no matching state/input topology. */
static void overlay11UpdateSelection(s32 updateRate) {
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
        func_overlay_045_F0001BF4_188E04C_o011Reloc(gOverlay11Handle2,
                                          gOverlay11Selection);
        func_overlay_045_F0001BF4_188E04C_o011Reloc(gOverlay11Handle3, 0);
    } else if (mode == 3) {
        func_overlay_045_F0001BF4_188E04C_o011Reloc(gOverlay11Handle2, 0);
        func_overlay_045_F0001BF4_188E04C_o011Reloc(gOverlay11Handle3,
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
static void overlay11UpdateMenu(s32 updateRate) {
    s32 index;
    s8 direction;
    Overlay11ObjectSub *sub;
    volatile s32 *menuInput;
    void **handle;
    Overlay11Status *status;
    s32 finish;

    status = func_80028F54();
    direction = D_800D3194[D_1C4];
    if (direction < -32) {
        if (((status->value3 >= 2) && (D_1BC < 3)) ||
            ((status->value3 == 1) && (D_1BC < 2))) {
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
        func_overlay_045_F0001BF4_188E04C_o011Reloc(handle[index],
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
            overlay11ReleaseCurrentGroup_o011Reloc();
            D_204 = 1;
            break;
        case 2:
            index = *menuInput;
            if (index == 0) {
                overlay11EnableHandles(4);
            } else if (index == -1) {
                overlay11DisableHandles(4);
            } else if (index == 1) {
                D_8007BEF8.s = 6;
                D_8007BEFC.s = 5;
                D_8007BF0C = 0;
                sub = func_80005820(D_1C4)->sub64;
                if ((status->value1 != 0) && !(D_8007BF1C & 0x100)) {
                    status->value3--;
                }
                if (status->value3 == 0) {
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
                overlay11EnableHandles(4);
            } else if (index == -1) {
                overlay11DisableHandles(4);
            } else if (index == 1) {
                if (D_8007C1A0 == 1) {
                    func_8003A754();
                }
                overlay11ReleaseGroup4();
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

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 11 as none. */
static void overlay11UpdateTwoOptionMenu(s32 updateRate) {
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
        func_overlay_045_F0001BF4_188E04C_o011Reloc(*handle, value);
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
            overlay11ReleaseCurrentGroup_o011Reloc();
            OVERLAY11_COUNTER = 1;
            break;
        case 2:
            if (OVERLAY11_MENU_INPUT == 0) {
                overlay11EnableHandles(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == -1) {
                overlay11DisableHandles(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == 1) {
                mainChangeCameras(1);
                overlay11ReleaseGroup3A();
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                OVERLAY11_COUNTER = 1;
            }
            break;
        }
    }
}

static void overlay11UpdateFiveOptionMenu(s32 updateRate) {
    s32 index;
    s8 direction;
    s16 value;
    void **handle;
    void *action;
    Overlay11Object *object;
    Overlay11ObjectSub *sub;
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
        func_overlay_045_F0001BF4_188E04C_o011Reloc(*handle, value);
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
            overlay11ReleaseCurrentGroup_o011Reloc();
            D_204 = 1;
            break;
        case 2:
            action = *menuInput;
            if (action == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (action == (void *)-1) {
                overlay11DisableHandles(6);
                return;
            }
            if (action == (void *)1) {
                D_8007BEF8.s = 1;
                D_8007BEFC.s = 0;
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
                overlay11EnableHandles(6);
                return;
            }
            if (action == (void *)-1) {
                overlay11DisableHandles(6);
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
                overlay11EnableHandles(6);
                return;
            }
            if (action == (void *)-1) {
                overlay11DisableHandles(6);
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
                overlay11EnableHandles(6);
                return;
            }
            if (action == (void *)-1) {
                overlay11DisableHandles(6);
                return;
            }
            if (action == (void *)1) {
                overlay11ReleaseGroup6A();
                D_lastMode = 5;
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                D_204 = 1;
            }
            break;
        }
    }
}

/* Matched 2026-10-01 by writing each object the relocation records name
 * instead of offsets from one menu base:
 * - the option handles are indexed by the one-based option number, with no
 *   declared walking pointer; the compiler's own cursor takes the temporary
 *   home and is saved before the index;
 * - the action is read from its object at every test, with no local and no
 *   volatile pointer;
 * - the two status modes are an ordinary `||`, and the six status fields are
 *   cleared by a plain loop, which the compiler unrolls as shipped;
 * - the two configuration bytes are unsigned, so the two 4s stay separate;
 * - five locals, which puts the cursor's temporary at the shipped home. */
static void func_overlay_011_F0001E4C_186A694(s32 updateRate) {
    s32 index;
    s8 direction;
    Overlay11ObjectSub *sub;
    s16 value;
    Overlay11Status *status;

    status = func_80028F54();
    direction = D_800D3194[gOverlay11PadIndex];
    if (direction < -32) {
        if (gOverlay11OptionIndex < 5) {
            gOverlay11OptionIndex++;
            amSndPlay(0x32C, 0);
            direction = D_800D3194[gOverlay11PadIndex];
        } else {
            amSndPlay(0x32D, 0);
            direction = D_800D3194[gOverlay11PadIndex];
        }
    }
    if (direction >= 33) {
        if (gOverlay11OptionIndex >= 2) {
            gOverlay11OptionIndex--;
            amSndPlay(0x32C, 0);
        } else {
            amSndPlay(0x32D, 0);
        }
    }

    for (index = 1; index < 6; index++) {
        value = (index == gOverlay11OptionIndex) ? gOverlay11OptionHighlight : 0;
        overlay45SetField22(gOverlay11OptionHandles[index], value);
    }

    if ((joyGetPressed(gOverlay11PadIndex) & 0x8000) || (gOverlay11OptionAction != 0)) {
        switch (gOverlay11OptionIndex) {
        case 1:
            overlay66Select(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            amTuneSetFadeScaled(0.5f, 0x7F);
            overlay11ReleaseCurrentGroup_o011Reloc();
            gOverlay11OptionDone = 1;
            break;
        case 2:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                if (((D_8007C1A0 == 2) || (D_8007C1A0 == 3)) && (gOverlay11GameStateFlagReloc != 0)) {
                    D_8007BEFC.u = 4 - D_8007C1A0;
                    D_8007BEF8.u = 4;
                } else {
                    D_8007BEFC.u = 0;
                    D_8007BEF8.u = D_8007C1A0;
                }
                sub = func_80005820(gOverlay11PadIndex)->sub64;
                gOverlay11OptionDone = 1;
                if ((status->mode == 5) || (status->mode == 6)) {
                    for (index = 0; index < 6; index++) {
                        status[index].field8 = 0;
                    }
                }
                mainChangeLevel(levelGetNumber(), sub->value1, 0, 5, 1, 0);
            }
            break;
        case 3:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                mainChangeCameras(1);
                mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        case 4:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                gOverlay11OptionDone = 1;
                mainChangeCameras(1);
                if (status->mode == 5) {
                    mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                } else {
                    mainChangeLevel(0xC, 0, 0, 0x11, 1, 0);
                }
            }
            break;
        case 5:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                gOverlay11OptionDone = 1;
                overlay11ReleaseGroup6B();
                mainChangeCameras(1);
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
            }
            break;
        }
    }
}

/* The sibling of func_overlay_011_F0001E4C_186A694 (the same option-menu
 * update for another menu), written as its copy and matched as the first draft of it
 * (227 masked words on the inherited candidate). Same objects, same loop
 * forms and the same five locals; only the case bodies differ: option 2
 * fixes the configuration bytes and clears the status fields
 * unconditionally, options 3 and 4 change level before setting the done
 * flag, and option 5 releases group 6C and sets the resident next-mode word
 * (resident bss 0x800D3048, reached through its relocation record). */
static void func_overlay_011_F00022E8_186AB30(s32 updateRate) {
    s32 index;
    s8 direction;
    Overlay11ObjectSub *sub;
    s16 value;
    Overlay11Status *status;

    status = func_80028F54();
    direction = D_800D3194[gOverlay11PadIndex];
    if (direction < -32) {
        if (gOverlay11OptionIndex < 5) {
            gOverlay11OptionIndex++;
            amSndPlay(0x32C, 0);
            direction = D_800D3194[gOverlay11PadIndex];
        } else {
            amSndPlay(0x32D, 0);
            direction = D_800D3194[gOverlay11PadIndex];
        }
    }
    if (direction >= 33) {
        if (gOverlay11OptionIndex >= 2) {
            gOverlay11OptionIndex--;
            amSndPlay(0x32C, 0);
        } else {
            amSndPlay(0x32D, 0);
        }
    }

    for (index = 1; index < 6; index++) {
        value = (index == gOverlay11OptionIndex) ? gOverlay11OptionHighlight : 0;
        overlay45SetField22(gOverlay11OptionHandles[index], value);
    }

    if ((joyGetPressed(gOverlay11PadIndex) & 0x8000) || (gOverlay11OptionAction != 0)) {
        switch (gOverlay11OptionIndex) {
        case 1:
            overlay66Select(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            amTuneSetFadeScaled(0.5f, 0x7F);
            overlay11ReleaseCurrentGroup_o011Reloc();
            gOverlay11OptionDone = 1;
            break;
        case 2:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                D_8007BEFC.u = 3;
                D_8007BEF8.u = 4;
                sub = func_80005820(gOverlay11PadIndex)->sub64;
                gOverlay11OptionDone = 1;
                for (index = 0; index < 6; index++) {
                    status[index].field8 = 0;
                }
                mainChangeLevel(levelGetNumber(), sub->value1, 0, 5, 1, 0);
            }
            break;
        case 3:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                mainChangeCameras(1);
                mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        case 4:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                mainChangeCameras(1);
                mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        case 5:
            if (gOverlay11OptionAction == 0) {
                overlay11EnableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                overlay11DisableHandles(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                overlay11ReleaseGroup6C();
                gOverlay11NextModeReloc = 7;
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        }
    }
}

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 11 as none. */
static void overlay11UpdateModeSix(s32 updateRate) {
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
        func_overlay_045_F0001BF4_188E04C_o011Reloc(*handle, value);
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
            overlay11ReleaseCurrentGroup_o011Reloc();
            OVERLAY11_COUNTER = 1;
            break;
        case 2:
            if (OVERLAY11_MENU_INPUT == 0) {
                overlay11EnableHandles(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == -1) {
                overlay11DisableHandles(3);
                return;
            }
            if (OVERLAY11_MENU_INPUT == 1) {
                overlay11ReleaseGroup3B();
                D_lastMode = 6;
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                OVERLAY11_COUNTER = 1;
            }
            break;
        }
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup4(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd4;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup3A(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd3;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup6A(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd6;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup6B(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd6;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup6C(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd6;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseGroup3B(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd3;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
static void overlay11ReleaseHandles(void) {
    overlay45ReleaseDescriptor(gOverlay11Handles[0]);
    overlay45ReleaseDescriptor(gOverlay11Handles[1]);
    overlay45ReleaseDescriptor(gOverlay11Handles[2]);
    overlay45ReleaseDescriptor(gOverlay11Handles[3]);
    gOverlay11Handles[0] = NULL;
    gOverlay11Handles[1] = NULL;
    gOverlay11Handles[2] = NULL;
    gOverlay11Handles[3] = NULL;
}

/* DKR v77/v80 contain no matching state-to-release dispatcher. */
void overlay11ReleaseCurrentGroup(void) {
    Overlay11Status *state;

    if (gOverlay11GroupsActive != 0) {
        state = func_80028F54();
        switch (state->mode) {
        case 0:
            overlay11ReleaseGroup4();
            break;
        case 1:
            overlay11ReleaseGroup6A();
            break;
        case 3:
            overlay11ReleaseGroup3A();
            break;
        case 4:
        case 5:
            if (D_8007C1A0 == 1) {
                overlay11ReleaseGroup6C();
            } else {
                overlay11ReleaseGroup6B();
            }
            break;
        case 2:
            overlay11ReleaseGroup3B();
            break;
        }
    }
}

/* Tier B: the background initializes and updates all 13 by 17 signed
 * halfword phases. Qualified readers identify the three following words:
 * selected option/created-active state, handle mode and controller index.
 * Tier D: bounded storage views, not recovered original declarations.
 * Legacy data-vs-BSS aliases and reader/prototype debts remain unchanged. */
s16 gOverlay11StoragePhases[13][17];
s32 gOverlay11StorageOptionIndex;
s32 gOverlay11StorageHandleMode;
s32 gOverlay11StoragePadIndex;
