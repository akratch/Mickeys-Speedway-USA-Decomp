#include "PR/ultratypes.h"

typedef struct Overlay60Command {
    u32 w0;
    u32 w1;
} Overlay60Command;

typedef struct Overlay60Color {
    u8 pad00[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad0B;
} Overlay60Color;

extern u8 gOverlay60LocalDataReloc[];
extern Overlay60Command *gOverlay60DisplayListReloc;
extern s16 gOverlay60InputAxisReloc;
extern u8 D_800000B0[];

/* Cross-overlay call through the loader's relocation surface: the shipped
 * word is the 0xF0000000 addend (a SYMBOL record, not a JUMP), so the
 * callee must be named by its o060 placeholder, as overlay60Prefix does. */
extern s32 overlay82IsActive_o060Reloc(void *object);
extern void func_80034554(Overlay60Command **commands, s32 resource, s32 mode,
                          s32 flags);
extern void func_80036600(Overlay60Color *color, s32 index);
extern void func_80036660(Overlay60Color *color, s32 amount);

/* One display-list append the way the SDK's packet macros write it: a
 * block-scoped pointer per expansion, on one physical line. */
#define O60_APPEND(a, b) { Overlay60Command *_g = gOverlay60DisplayListReloc++; _g->w0 = (a); _g->w1 = (b); }
/* The SDK's _SHIFTL: the mask is redundant on a u8 field and as1 folds it
 * into the lbu, but ugen has already drawn a ring temp for it (L149). */
#define OVERLAY60_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

/* Matched 2026-09-16 (lane s1-c), 222/222 words, frame 0xC0, from 84 in six
 * measured cycles, no colour forced. What each edit bought, measured alone:
 *  - the first append through the block-scoped macro (84 -> 78): the target
 *    loads three appends into v0 and the loop's last into v1, which one
 *    declared symbol web cannot do;
 *  - OVERLAY60_SHIFTL on the raw colour bytes (78 -> 19): each folded mask is
 *    a spent ring draw, and the target's arm-1 ring skips t2/t5/t8 exactly
 *    where the three byte reads sit; arm 2 needs it on red only;
 *  - no `offset` local, `(f32)(x - left)` strength-reduced in the loop
 *    (19 -> 7): x's web then outranks the display-list address web
 *    (61/3 against 62/3 with the declared offset) and takes s0;
 *  - `axis` declared and `topFloat` inlined (7 -> 0): every declared local,
 *    live or dead, takes a frame slot in declaration order, and the reduced
 *    form reserves one temp word more, so color's home at 0x9C needs six
 *    slots above it and one fewer below at frame 0xC0.
 */
s32 func_overlay_060_F0002F54_18BCD2C(s32 left, s32 bottom, s32 width,
                                       s32 height, s32 progress, s32 alpha,
                                       s32 ticks) {
    s16 axis;
    Overlay60Command *command;
    s32 right;
    s32 x;
    s32 split;
    u32 top;
    Overlay60Color color;
    f32 widthFloat;
    f32 fraction;
    f32 verticalOffset;

    if (overlay82IsActive_o060Reloc(
            *(void **)(gOverlay60LocalDataReloc + 0xA8)) != 0) {
        axis = gOverlay60InputAxisReloc;
        if (axis < -0x10) {
            progress -= ticks * 2;
            if (progress < 0) {
                progress = 0;
            }
        } else if (axis >= 0x11) {
            progress += ticks * 2;
            if (progress >= 0x101) {
                progress = 0x100;
            }
        }
    }

    right = left + width;
    func_80034554(&gOverlay60DisplayListReloc, 0, 0, 0);
    O60_APPEND(0x07020010, (u32)D_800000B0);

    x = left;
    if (left < right) {
        widthFloat = (f32)width;
        split = (s32)((f32)left +
                      widthFloat * ((f32)progress * 0.00390625f));
        do {
            fraction = (f32)(x - left) / widthFloat;
            verticalOffset = (f32)height * fraction;
            func_80036600(&color, 0xB);
            func_80036660(&color, (s32)(fraction * 60.0f));

            if (x < split) {
                command = gOverlay60DisplayListReloc++;
                command->w0 = 0xFA000000;
                command->w1 = OVERLAY60_SHIFTL(color.red, 24, 8) |
                              OVERLAY60_SHIFTL(color.green, 16, 8) |
                              OVERLAY60_SHIFTL(color.blue, 8, 8) |
                              OVERLAY60_SHIFTL(alpha, 0, 8);
            } else {
                command = gOverlay60DisplayListReloc++;
                command->w0 = 0xFA000000;
                command->w1 = OVERLAY60_SHIFTL(color.red >> 1, 24, 8) |
                              OVERLAY60_SHIFTL(color.green >> 1, 16, 8) |
                              OVERLAY60_SHIFTL(color.blue >> 1, 8, 8) |
                              OVERLAY60_SHIFTL(alpha >> 1, 0, 8);
            }

            command = gOverlay60DisplayListReloc++;
            command->w0 = 0xF6000000 |
                          OVERLAY60_SHIFTL(x + 2, 14, 10) |
                          OVERLAY60_SHIFTL(bottom, 2, 10);
            top = (u32)((f32)bottom - verticalOffset);
            command->w1 = OVERLAY60_SHIFTL(x, 14, 10) |
                          OVERLAY60_SHIFTL(top, 2, 10);
            x += 4;
        } while (x < right);
    }
    return progress;
}
