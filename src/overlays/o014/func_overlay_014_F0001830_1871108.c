#include "PR/ultratypes.h"

extern s32 gOverlay14ValueC0;
/* The resident definition (main/font.c) returns u8; this TU calls it as
 * returning int, as an undeclared call would. As u8 the zero-extension
 * becomes a second web copied out of v0 (one extra move, frame +8). */
extern s32 func_8004D5C0(s32 font);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha,
                       s32 opacity);
extern void func_8004B0A4(s32 font);
extern u8 *func_8004D40C(s32 font, char *text, s32 maxWidth,
                         u8 **lineStart, s32 *outWidth);
extern void func_8004B0F8(void *displayList, s32 x, s32 y, char *text,
                          s32 alignmentFlags);

/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/overlays/o7/overlay_7/func_overlay_7_007023D4_1EFD4FC.s;
 * body reconstructed from Mickey evidence. */
/* Matched 2026-09-23 (lane A2-ov), 201/201 words at frame 0x90, unforced.
 * The int-returning declaration of func_8004D5C0 closed size and frame
 * (Track B, lane B-ovsmall). The last five words were case 7's schedule:
 * the ROM loads the four colour bytes at +1..+5 and puts the stream
 * increment in the call's delay slot. as1 breaks a tie between the
 * increment and the loads on source line number (L59), and the increment
 * written first takes the smaller line, so it is picked first and the last
 * load fills the slot. Case 7 therefore draws from a block placed before its
 * label: its own statements are the increment and a jump back, so uopt
 * merges the two blocks with the increment keeping the later line. Temps
 * loaded before the increment reproduce the case too, but a third
 * temporary is globalcoloured to t0 and removes it from the ring (47). */
s32 func_overlay_014_F0001830_1871108(s32 context, u8 *stream, s32 skip) {
    s32 remaining;
    s32 y;
    s32 x;
    s32 extra;
    s32 width;
    s32 cellWidth;
    s32 done;
    s32 result;
    u8 saved;
    void *drawArgs[7];
    s32 adjust;
    u8 *cursor;

    result = 0;
    done = 0;
    cellWidth = func_8004D5C0(2);
    remaining = (0x58 / cellWidth) - 1;
    func_8004B0DC(0, 0, 0, 0);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF,
               (gOverlay14ValueC0 * 0xFF) >> 8);
    func_8004B0A4(2);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    do {
        x = 0x60; width = 0xC8; cursor = 0; adjust = 0; extra = 0;
        switch (*stream) {
        case 1:
            if (skip == 0) { y += cellWidth; remaining--; } else skip--;
            stream += 4;
            break;
        case 2:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 4; x = 0xC4;
            break;
        case 3:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            break;
        case 4:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 1; x = 0x127;
            break;
        case 5:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            adjust = 1;
            break;
        case 6:
            stream += 8;
            saved = stream[-2];
            fontColour(stream[-7], stream[-6], stream[-5], stream[-3],
                       (saved * gOverlay14ValueC0) >> 8);
            break;
        case7Draw:
            func_8004B0DC(stream[-7], stream[-6], stream[-5], stream[-3]);
            break;
        case 7:
            stream += 8;
            goto case7Draw;
        default:
            done = 1;
            break;
        }
        if ((cursor != 0) && (remaining >= 0)) {
            do {
                if (adjust != 0) { x += 8; width -= 8; }
                cursor = func_8004D40C(2, (char *)cursor, width,
                                       (u8 **)drawArgs, 0);
                if (cursor != 0) {
                    if (skip == 0) {
                        remaining--;
                        if (remaining >= 0) {
                            saved = *cursor; *cursor = 0;
                            func_8004B0F8((void *)context, x, y,
                                          (char *)drawArgs[0], extra);
                            *cursor = saved; y += cellWidth;
                        }
                    } else {
                        skip--;
                    }
                }
                if (adjust != 0) { x -= 8; width += 8; adjust = 0; }
            } while ((cursor != 0) && (remaining >= 0));
        }
        if (remaining < 0) {
            result = 1;
            done = 1;
        }
    } while (done == 0);
    return result;
}
