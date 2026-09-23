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

#define CASE_PREINC 1

#ifdef NON_MATCHING
/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/overlays/o7/overlay_7/func_overlay_7_007023D4_1EFD4FC.s;
 * body reconstructed from Mickey evidence. */
/* Retained configured-isolated evidence is exact-size at 201 words/frame 0x90
 * with eight raw sites at +0x98,+0x11C,+0x1C0,+0x1F4,+0x1F8,+0x1FC,+0x200,
 * +0x208 and five after local-relocation normalization at +0x1F4..+0x208.
 * Its 21 runtime records and seven-entry switch payload are structurally exact,
 * but all eight calls previously used one false overlay-local identity. The
 * declarations below repair the six resident font identities; configured V0
 * must re-establish the relocation surface before one target-shaped case-7
 * load-before-increment probe. Park after those two builds if flat. */
/* Track B (2026-09-23): the int-returning declaration above closed size
 * and frame (804/804 bytes, frame 0x90); 5 masked words remain, all in
 * case 7, where the target schedules the stream increment into the call's
 * delay slot and this spelling schedules the last argument load there. */
/* Ownership trial (2026-08-28): fixed the TU's +0x174..+0x190 .rodata range;
 * linked promotion established module-growth/table ownership only, not exact C
 * text or relocation identity. Module growth is cleared. */
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
#if CASE_PREINC
            stream += 8;
            saved = stream[-2];
            fontColour(stream[-7], stream[-6], stream[-5], stream[-3],
                       (saved * gOverlay14ValueC0) >> 8);
#else
            fontColour(stream[1], stream[2], stream[3], stream[5],
                       (stream[6] * gOverlay14ValueC0) >> 8);
            stream += 8;
#endif
            break;
        case 7:
            stream += 8;
            func_8004B0DC(stream[-7], stream[-6], stream[-5], stream[-3]);
            break;
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F0001830_1871108/func_overlay_014_F0001830_1871108.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_014_F0001830_1871108:start
 * symbol: func_overlay_014_F0001830_1871108
 * score: 5/201 words
 * frame: 0x90
 * relocations: 21
 * first-mismatch: +0x1F4
 * summary: Delta 0, frame closed, 186 to 5: callee declared int-returning. Left: case 7 fills the jal delay with the last arg load; target fills it with the increment.
 * PLATEAU-HANDOFF:func_overlay_014_F0001830_1871108:end
 */
