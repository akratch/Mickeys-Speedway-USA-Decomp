#include "PR/ultratypes.h"
#include "overlays/overlay_045.h"
#include "overlays/overlay057.h"

/* Overlay 57 bss. Every name below is the one already adopted by a matched
 * overlay 57 translation unit for the same runtime relocation identity; the
 * three that are new here (+0x100, +0x108, +0x10C) are the selection indices
 * this function shares with the +0x104 mode value. */
extern Overlay45ResourceDescriptor *gOverlay57Resource000;
extern Overlay45ResourceDescriptor *gOverlay57Range008Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range008End[];
extern Overlay45ResourceDescriptor *gOverlay57Range030Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range030End[];
extern Overlay45ResourceDescriptor *gOverlay57Range058Start[];
extern Overlay45ResourceDescriptor *gOverlay57Resource06C;
extern Overlay45ResourceDescriptor *gOverlay57Resource070;
extern Overlay45ResourceDescriptor *gOverlay57Resource074;
extern Overlay45ResourceDescriptor *gOverlay57Resource078;
extern Overlay45ResourceDescriptor *gOverlay57Range080Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range080End[];
extern Overlay45ResourceDescriptor *gOverlay57Range0E0Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range0E0End[];
extern Overlay45ResourceDescriptor *gOverlay57Resource0F8;
extern Overlay45ResourceDescriptor *gOverlay57Resource0FC;
extern s32 gOverlay57Selection100;
extern s32 O57_D_104;
extern s32 gOverlay57Selection108;
extern s32 gOverlay57Mode10C;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;
extern s32 O57_D_128;
extern s32 D_130;
extern s32 D_134;
extern s32 D_138;
extern s32 gOverlay57ModeFlag;
extern void *D_148;
extern s32 O57_D_160;
extern s32 O57_D_164;
extern s32 O57_D_194;

/* Overlay 57 initialized data. */
extern s32 gOverlay57UpdateSelection;
extern s32 D_500;
extern s32 D_504;

/* Runtime-linked globals shared with the resident module. */
extern s32 O57_mode0954;
extern u8 gOverlay57TableIndex;

/* Runtime-linked calls: overlay 45 +0x314 and +0x1BE0, and the resident
 * trackSetFog, amSndStop and amSndPlay. Each is spelled as an overlay
 * placeholder so the stored `jal 0` addend is preserved. */
extern void overlay57LayoutDescriptorReloc(Overlay45ResourceDescriptor *descriptor,
                                           s32 x, s32 y, s32 flags);
extern void overlay57SetDescriptorModeReloc(Overlay45ResourceDescriptor *descriptor,
                                            s32 mode);
extern void overlay57SetFogReloc(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                                 u8 red, u8 green, u8 blue, s8 state);
extern void overlay57SoundStopReloc(void *handle);
extern void overlay57SoundPlayReloc(u16 soundId, void **handle);

extern void overlay57EaseAndLatch(s32 updateRate);
extern void overlay57SmoothAndCheckDistance(s32 updateRate);
extern void overlay57CheckDistance(s32 updateRate);
extern void overlay57UpdateTransition(s32 updateRate);
extern void overlay57UpdateModeState(s32 updateRate);
extern void overlay57BeginMode(s32 updateRate);
extern void overlay57StartMode(s32 updateRate);
extern void func_overlay_057_F0004460_18A8058(s32 updateRate);
extern void overlay57UpdateModeTrigger(s32 updateRate);
extern void overlay57InitializeMode(s32 updateRate);
extern void func_overlay_057_F0004E18_18A8A10(s32 updateRate);
extern void func_overlay_057_F00060F8_18A9CF0(s32 updateRate);

/* Fresh workbench: structure-mismatch, 614 differing words, first mismatch
 * +0x04; 850 words against 883 with the exact 0x38 frame and its counter
 * spill slot. Dispatch order, the three distinct zero-valued globals, the
 * five runtime-linked call identities, the accumulator sign and both clamp
 * placements are all settled; the +0x4 residual is only the unresolved
 * placeholder value in the guarded build.
 *
 * What remains is one callee-saved assignment. IDO gives the single saved
 * register to the six-entry loop counter here, where the shipped body gives
 * it to the +0xE0 descriptor base and spills the counter instead; with the
 * base uncommoned, the four unrolled fade copies also keep a spare register
 * and address their stores directly rather than through the assembler
 * temporary. That accounts for the whole -33 word deficit, twenty of it the
 * store addresses.
 *
 * Eliminated: the flag lattice (-O1/-O2, with and without -g3, -mips1/2/3,
 * loop unroll 0/2/4/8, -Wab,-r4300_mul) leaves the configured -O2 -mips2
 * strictly best; a separate pointer local for the +0xE0 base, `register`
 * qualifiers, the direct array spelling and separate loop counters all leave
 * the assignment unchanged. Local declaration order does matter and is the
 * one that reproduces the frame and spill slot.
 *
 * Promotion note for whoever closes this: the dispatch is a real compiler
 * jump table, so an exact body will emit a read-only table this overlay
 * already carries in its retained data segment. Overlay 1's dispatch owner
 * is the working precedent for that link recipe -- name the retained table
 * with an absolute anchor at its stored offset, rebind the table's two text
 * relocations onto that name, then externalize and drop the duplicate
 * compiler section. Without it the overlay grows and the ROM stops
 * rebuilding. */
#ifdef NON_MATCHING
void func_overlay_057_F0001AE8_18A56E0(s32 updateRate) {
    s32 limit;
    s32 value;
    s32 count;
    Overlay45ResourceDescriptor **cursor;

    D_130 = 0;
    switch (gOverlay57Timer) {
        case 11:
            overlay57EaseAndLatch(updateRate);
            break;
        case 14:
            overlay57SmoothAndCheckDistance(updateRate);
            break;
        case 5:
            overlay57CheckDistance(updateRate);
            break;
        case 7:
            overlay57UpdateTransition(updateRate);
            break;
        case 1:
            overlay57UpdateSelection(updateRate);
            break;
        case 4:
            overlay57UpdateModeState(updateRate);
            break;
        case 12:
            overlay57BeginMode(updateRate);
            break;
        case 13:
            overlay57HandleModeInput(updateRate);
            break;
        case 15:
            overlay57StartMode(updateRate);
            break;
        case 16:
            func_overlay_057_F0004460_18A8058(updateRate);
            break;
        case 9:
            overlay57UpdateModeTrigger(updateRate);
            break;
        case 21:
            overlay57InitializeMode(updateRate);
            break;
        case 10:
            func_overlay_057_F0004E18_18A8A10(updateRate);
            break;
        case 20:
            func_overlay_057_F00060F8_18A9CF0(updateRate);
            break;
    }

    overlay57Draw32A0(updateRate);

    if (gOverlay57State == 1) {
        overlay57LayoutDescriptorReloc(gOverlay57Resource000, 0xA0, 0x20, 0x104);
        if (O57_mode0954 == 1) {
            overlay57LayoutDescriptorReloc(
                gOverlay57Range008Start[gOverlay57Selection100], 0xA0, 0xBE, 0x104);
        } else {
            overlay57LayoutDescriptorReloc(
                gOverlay57Range030Start[gOverlay57Selection100], 0xA0, 0xBE, 0x104);
        }

        cursor = gOverlay57Range008Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor < gOverlay57Range008End);
        cursor = gOverlay57Range030Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor < gOverlay57Range030End);

        if (O57_mode0954 == 1) {
            overlay57SetDescriptorModeReloc(
                gOverlay57Range008Start[gOverlay57Selection108], gOverlay57Mode10C);
            overlay57SetDescriptorModeReloc(
                gOverlay57Range008Start[gOverlay57Selection100], O57_D_104);
        } else {
            overlay57SetDescriptorModeReloc(
                gOverlay57Range030Start[gOverlay57Selection108], gOverlay57Mode10C);
            overlay57SetDescriptorModeReloc(
                gOverlay57Range030Start[gOverlay57Selection100], O57_D_104);
        }

        cursor = gOverlay57Range080Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor < gOverlay57Range080End);
        cursor = gOverlay57Range0E0Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor != gOverlay57Range0E0End);
    } else {
        overlay57LayoutDescriptorReloc(gOverlay57Resource000, 0xA0, -0x1E, 0x104);
        if (O57_mode0954 == 1) {
            overlay57LayoutDescriptorReloc(
                gOverlay57Range008Start[gOverlay57Selection100], 0xA0, 0x104, 0x104);
        } else {
            overlay57LayoutDescriptorReloc(
                gOverlay57Range030Start[gOverlay57Selection100], 0xA0, 0x104, 0x104);
        }
    }

    if (gOverlay57State == 2) {
        if (gOverlay57Timer == 10) {
            if (O57_D_194 == 1) {
                overlay57LayoutDescriptorReloc(
                    gOverlay57Range080Start[O57_D_160], 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource0FC, 0xA0, -0x20, 0x104);
            } else {
                overlay57LayoutDescriptorReloc(
                    gOverlay57Range080Start[O57_D_160], 0xA0, 0xBE, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource0FC, 0xA0, 0x20, 0x104);
            }

            for (count = 0, limit = 0x18; count != limit; count++) {
                if ((count != gOverlay57Selection108) &&
                    (count != gOverlay57Selection100)) {
                    overlay57SetDescriptorModeReloc(gOverlay57Range080Start[count], 0);
                }
            }
            overlay57SetDescriptorModeReloc(
                gOverlay57Range080Start[O57_D_164], gOverlay57Mode10C);
            overlay57SetDescriptorModeReloc(
                gOverlay57Range080Start[O57_D_160], O57_D_104);
        }

        cursor = gOverlay57Range008Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor != gOverlay57Range008End);
    } else {
        if (O57_D_160 < 0x18) {
            overlay57LayoutDescriptorReloc(
                gOverlay57Range080Start[O57_D_160], 0xA0, 0x104, 0x104);
        }
        overlay57LayoutDescriptorReloc(gOverlay57Resource0FC, 0xA0, -0x20, 0x104);
    }

    cursor = gOverlay57Range0E0Start;
    if (gOverlay57State == 6) {
        overlay57LayoutDescriptorReloc(cursor[O57_D_160], 0xA0, 0xBE, 0x104);
        overlay57LayoutDescriptorReloc(gOverlay57Resource0FC, 0xA0, 0x20, 0x104);
        for (count = 0; count < 6; count++) {
            if ((count != gOverlay57Selection108) &&
                (count != gOverlay57Selection100)) {
                overlay57SetDescriptorModeReloc(cursor[count], 0);
            }
        }
        overlay57SetDescriptorModeReloc(cursor[O57_D_164], gOverlay57Mode10C);
        overlay57SetDescriptorModeReloc(cursor[O57_D_160], O57_D_104);

        cursor = gOverlay57Range008Start;
        do {
            overlay57SetDescriptorModeReloc(*cursor, 0);
            cursor++;
        } while (cursor != gOverlay57Range008End);
    } else if (O57_D_160 < 6) {
        overlay57LayoutDescriptorReloc(cursor[O57_D_160], 0xA0, 0x104, 0x104);
    }

    if (gOverlay57State == 3) {
        overlay57LayoutDescriptorReloc(gOverlay57Resource0F8, 0xA0, 0x1E, 0x104);
    } else {
        overlay57LayoutDescriptorReloc(gOverlay57Resource0F8, 0xA0, -0x1E, 0x104);
    }

    if (gOverlay57State == 5) {
        switch (gOverlay57TableIndex) {
            case 0:
                overlay57LayoutDescriptorReloc(gOverlay57Resource06C, 0xA0, 0xBE, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 1:
                overlay57LayoutDescriptorReloc(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource070, 0xA0, 0xBE, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 2:
                overlay57LayoutDescriptorReloc(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource074, 0xA0, 0xBE, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 3:
                overlay57LayoutDescriptorReloc(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay57LayoutDescriptorReloc(gOverlay57Resource078, 0xA0, 0xBE, 0x104);
                break;
        }
        overlay57LayoutDescriptorReloc(
            gOverlay57Range058Start[gOverlay57UpdateSelection], 0xA0, 0x20, 0x104);
    } else {
        overlay57LayoutDescriptorReloc(
            gOverlay57Range058Start[gOverlay57UpdateSelection], 0xA0, -0x28, 0x104);
        overlay57LayoutDescriptorReloc(gOverlay57Resource074, 0xA0, 0x104, 0x104);
        overlay57LayoutDescriptorReloc(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
        overlay57LayoutDescriptorReloc(gOverlay57Resource070, 0xA0, 0x104, 0x104);
        overlay57LayoutDescriptorReloc(gOverlay57Resource078, 0xA0, 0x104, 0x104);
    }

    D_500 += D_504 * updateRate;
    if (D_500 < 0) {
        D_500 = -D_500;
        D_504 = -D_504;
    } else if (D_500 >= 0x100) {
        D_500 = 0x1FE - D_500;
        D_504 = -D_504;
    }

    if ((gOverlay57State != 0) && (gOverlay57State != 3) &&
        (gOverlay57State != 4) && (gOverlay57State != 5) &&
        ((gOverlay57State != 2) || (O57_D_194 != 1))) {
        O57_D_128 += updateRate * 4;
        if (O57_D_128 > 0) {
            O57_D_128 = 0;
        }
    } else {
        O57_D_128 -= updateRate * 4;
        if (O57_D_128 < -0x50) {
            O57_D_128 = -0x50;
        }
    }

    if (gOverlay57ModeFlag != 0) {
        for (count = 0; count < updateRate; count++) {
            value = D_134;
            D_134 = value + ((0x3DE00 - value) >> 4);
            value = D_138;
            D_138 = value + ((0x3E800 - value) >> 4);
        }
    } else {
        for (count = 0; count < updateRate; count++) {
            value = D_134;
            D_134 = value + ((0x3FA00 - value) >> 6);
            value = D_138;
            D_138 = value + ((0x3FF00 - value) >> 6);
        }
    }

    overlay57SetFogReloc(0, D_134 >> 8, D_138 >> 8, D_134 >> 8, 0, 0, 0, 0);
    if (D_130 != 0) {
        if (D_148 != NULL) {
            overlay57SoundStopReloc(D_148);
        }
        overlay57SoundPlayReloc(0xF, &D_148);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0001AE8_18A56E0/func_overlay_057_F0001AE8_18A56E0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_057_F0001AE8_18A56E0:start
 * symbol: func_overlay_057_F0001AE8_18A56E0
 * score: 614/883 words
 * frame: 0x38
 * relocations: 405
 * first-mismatch: +0x160
 * summary: Guarded statement order is a fixed point; ring phase dominates from +0x140 and structural tail from +0x580. Unguarded 611 is non-adoptable.
 * PLATEAU-HANDOFF:func_overlay_057_F0001AE8_18A56E0:end
 */
