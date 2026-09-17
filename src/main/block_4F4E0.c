#include "PR/ultratypes.h"

extern s8 D_8007D640;
extern s32 D_800D6AB0;
extern s16 D_800D6AB4;
extern s16 D_800D6AB6;
extern s16 D_800D6AB8;
extern s16 D_800D6ABA;
extern s16 D_800D6ABC;
extern s16 D_800D6ABE;
extern s16 D_800D6AC0;
extern s16 D_800D6AC2;
extern s16 D_800D6AC4;
extern s16 D_800D6AC6;
extern s16 D_800D6AC8;
extern s32 D_800D6AD8[];
extern s32 D_800D6AE4;
extern s32 piRomGetFileSize(s32 fileId);
extern s32 func_8002B280(s32 size, s32 tag);

extern char *D_800D6AD0[];
extern u8 *D_800D6AE0;
extern void func_8004BB44(s32 windowId, s32 x1, s32 y1, s32 x2, s32 y2);
extern void func_8004BBE0(s32 windowId, s32 font);
extern void *func_8004BCC4(s32 windowId, s32 posX, s32 posY, char *text,
                           s32 number, s32 flags);
extern void fontWindowColour(s32 windowId, s32 red, s32 green, s32 blue,
                             s32 alpha);
extern void fontWindowFontBackground(s32 windowId, s32 red, s32 green,
                                     s32 blue, s32 alpha);
extern void fontWindowFontColour(s32 windowId, s32 red, s32 green, s32 blue,
                                 s32 alpha, s32 opacity);
extern void func_8004BFB0(s32 windowId);

/* PROVENANCE: adapted from Diddy Kong Racing's public decomp, src/game_text.c:
 * init_dialogue_text; Mickey's globals, constants, calls, and bytes remain authoritative. */
void func_8004E8E0(void) {
    D_800D6AB4 = (piRomGetFileSize(5) >> 2) - 2;
    D_800D6AD8[0] = func_8002B280(0x790, 0x8F);
    D_800D6AD8[1] = D_800D6AD8[0] + 0x3C0;
    D_800D6AB0 = D_800D6AD8[1] + 0x3C0;
    D_800D6AE4 = 0;
    D_800D6AC4 = 0;
    D_800D6AB6 = 0;
    D_800D6AB8 = 0x20;
    D_800D6AC8 = 0;
    D_800D6ABC = 0x20;
    D_800D6ABE = 0xCA;
    D_800D6AC0 = 0x120;
    D_800D6AC2 = 0xDE;
    D_8007D640 = 1;
}

extern void mmFree(void *ptr);
extern void func_8004BFD8(s32 windowId);
extern void func_8004BF64(s32 windowId);

void func_8004E99C(void) {
    if (D_8007D640 != 0) {
        mmFree((void *)D_800D6AD8[0]);
        func_8004BFD8(6);
        func_8004BF64(6);
        D_8007D640 = 0;
        D_800D6AC4 = 0;
    }
}

extern s32 D_8007D648;
extern void func_8004EC60(void);

void func_8004E9EC(s32 arg0) {
    D_8007D648 = arg0;
}

/* PROVENANCE: adapted from Diddy Kong Racing's public decomp, src/game_text.c:
 * render_subtitles; Mickey's own globals, calls, and linked bytes remain authoritative. */
void func_8004E9F8(void) {
    s32 textX;
    s32 textY;
    s32 i;
    s32 textFlags;
    char **textData;

    func_8004BF64(6);
    func_8004BB44(6, D_800D6ABC, D_800D6ABE, D_800D6AC0,
                  (s32)D_800D6AC2);
    fontWindowColour(6, 0, 0x60, 0,
                     (s32)(D_800D6AB6 * 0x64) >> 8);
    fontWindowFontBackground(6, 0, 0, 0, 0);
    textY = (s32)((((D_800D6AC2 - D_800D6ABE) -
                    (D_800D6AC6 * 0xC)) - (D_800D6AC6 * 2)) + 2) >> 1;
    for (i = 0; i < D_800D6AC6; i++) {
        textData = &D_800D6AD0[0];
        func_8004BBE0(6, (s32)textData[i][5]);
        textFlags = textData[i][6];
        if (textFlags == 4) {
            textX = (D_800D6AC0 - D_800D6ABC) >> 1;
        } else {
            if (textFlags == 1) {
                textX = (D_800D6AC0 - D_800D6ABC) - 8;
            } else {
                textX = 8;
            }
        }
        fontWindowFontColour(6, textData[i][1], textData[i][2],
                             textData[i][3], 0xFF,
                             (textData[i][4] * D_800D6AB6) >> 8);
        func_8004BCC4(6, textX, textY, textData[i] + 8, 1, textFlags);
        fontWindowFontColour(6, 0, 0, 0, 0xFF,
                             (D_800D6AB6 * 0xFF) >> 8);
        func_8004BCC4(6, textX + 1, textY + 1, textData[i] + 8, 1,
                      textFlags);
        textY += 0xE;
    }
    func_8004BFB0(6);
}
/* Census against the target: `addiu +1, beqzl +1, move -3, nop +2`, +1 word
 * net, frame 0x10 against 0x8.
 *
 * The three missing copies are the record cursor's, and they are recoverable.
 * The target advances the cursor by computing the next address, storing *that*
 * to D_800D6AE0, reading the next byte through it, and only then copying it
 * into the lasting carrier -- `sw t8,0(t1); lbu a0,0(t8); move v1,t8` -- which
 * is three references to `data + 1` and therefore two live carriers. Writing
 * the three advances as `D_800D6AE0 = data + 1; next = data[1]; data += 1;`
 * instead of `data += 1; D_800D6AE0 = data; next = *data;` restores all three
 * copies and takes the census delta from seven to three. It is not adopted
 * because it does not improve the residual: the remaining `addiu +1`,
 * `beqzl +1` and `nop +1` put it three words long instead of one, and the
 * positional score goes 79 to 84. It is recorded as the proved cause of the
 * `move -3` term, not as a better candidate.
 *
 * The `beqzl`/`nop` pair is the same branch-likely delay-slot effect as
 * overlay2ClassifyBoundary's: the target's inner scan uses a plain `beqz`
 * whose delay slot holds the `data + 1` it needs on the fall-through, and it
 * re-reads D_800D6AE0 after the two arms rather than keeping the cursor live.
 * Reproducing that re-read alone costs an `lw`, an `sw` and a `beqz`; combined
 * with the cursor fix it is four words long. The frame difference is the same
 * question again -- the target keeps the sentinel 12 in s0 and the constant 10
 * in a scratch t5, while the candidate promotes both to s0/s1 and pays a
 * second save/restore pair.
 *
 * The whole flag lattice is flat: every -O1/-O2/-O3 row at -mips1/-mips2/
 * -mips3, with and without r4300_mul, loopunroll and g3, is at least as long
 * with at least this census delta. */
#ifdef NON_MATCHING
void func_8004EC60(void) {
    register s32 var_v0;
    s32 sentinel;
    register u8 *data;
    register u8 next;

    sentinel = 0xC;
    data = D_800D6AE0;
    D_800D6AC6 = 0;
    D_800D6ABA = 0;
    next = *data;
    var_v0 = 0;
    if (next != 0) {
loop_2:
        D_800D6AC8 = next - 1;
        D_800D6AD0[D_800D6AC6] = (char *)data;
        D_800D6AE0 = data + 8;
        D_800D6ABA = data[7] * 6;
        data += 8;
        next = *data;
        while (next != 0) {
            if (next & 0x80) {
                data += 2;
            } else {
                data += 1;
            }
            D_800D6AE0 = data;
            next = *data;
        }
        D_800D6AC6 += 1;
        data += 1;
        if (D_800D6AC6 >= 2) {
            var_v0 = 1;
        }
        D_800D6AE0 = data;
        next = *data;
        if (next == 0xA) {
            data += 1;
            D_800D6AE0 = data;
            next = *data;
        } else {
            if (next == sentinel) {
                data += 1;
                D_800D6AE0 = data;
                next = *data;
                var_v0 = 1;
            }
        }
        if ((next != 0) && (var_v0 == 0)) {
            goto loop_2;
        }
    }
    if (D_800D6AC6 > 0) {
        D_800D6AC4 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/block_4F4E0/func_8004EC60.s")
#endif
void func_8004EDA8(s32 arg0)
{
  s16 var_a0;
  if (D_8007D640 != 0)
  {
    if (D_8007D648 == 0)
    {
      D_800D6AC4 = 0;
    }
    var_a0 = D_800D6AC4;
    if (var_a0 != 0)
    {
      if (D_800D6ABA <= 0)
      {
        D_800D6AB6 -= arg0 * D_800D6AB8;
        if (D_800D6AB6 < 0)
        {
          D_800D6AB6 = 0;
          D_800D6AC4 = 0;
          func_8004BFD8(6);
          func_8004BF64(6);
          var_a0 = D_800D6AC4;
        }
      }
      else
      {
        D_800D6AB6 += arg0 * D_800D6AB8;
        if (D_800D6AB6 >= 0x101)
        {
          D_800D6AB6 = 0x100;
        }
        D_800D6ABA -= arg0;
        if (D_800D6ABA <= 0)
        {
          func_8004EC60();
          var_a0 = D_800D6AC4;
        }
      }
    }
    if (var_a0 != 0)
    {
      func_8004E9F8();
    }
  }
}
extern s32 frontGetLanguage(void);
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset,
                            s32 size);

/* PROVENANCE: adapted from Jet Force Gemini src/subtitles.c subtitleStart
 * and Diddy Kong Racing src/game_text.c set_current_text. Mickey's globals,
 * language addends, calls, and bytes remain authoritative. */
/* Matched 2026-09-17 (lane w8-block), 28 -> 0 masked words at delta 0,
 * frame 0x20, 22 relocations, unforced. JFG carrier shape: named table
 * pointer, high-byte mask, and section size, with subscript access rather
 * than a walking pointer plus loaded-word CSE. That reorders ugen's
 * post-call scratch ring (L145/L160); colour-forcing the loaded-word web
 * to t2 scored 18 and did not close. */
void func_8004EED0(s32 arg0) {
    s32 *entries;
    s32 language;
    s32 temp;
    s32 size;

    if ((D_8007D640 != 0) && (arg0 >= 0) && (arg0 < D_800D6AB4)) {
        language = frontGetLanguage();
        switch (language) {
        case 2:
            arg0 += 2;
            break;
        case 1:
            arg0 += 1;
            break;
        case 3:
            arg0 += 3;
            break;
        case 5:
            arg0 += 4;
            break;
        }
        piRomLoadSection(5, D_800D6AB0, (arg0 & ~1) << 2, 0x10);
        entries = (s32 *)D_800D6AB0;
        temp = entries[arg0 & 1] & 0xFF000000;
        size = (entries[(arg0 & 1) + 1] & 0xFFFFFF) - (entries[arg0 & 1] & 0xFFFFFF);
        piRomLoadSection(4, D_800D6AD8[D_800D6AE4], entries[arg0 & 1] ^ temp, size);
        D_800D6AE0 = (u8 *)D_800D6AD8[D_800D6AE4];
        func_8004EC60();
        D_800D6AE4 = (D_800D6AE4 + 1) & 1;
    }
}
s32 func_8004F020(void) {
    return 0;
}

/* PLATEAU-HANDOFF:func_8004EC60:start
 * symbol: func_8004EC60
 * score: 79/82 words
 * frame: 0x10
 * relocations: 12
 * first-mismatch: +0x0
 * summary: Proc-4 census records nine draws; extra saved-register setup and direct global-address expression shape remain the live structural blocker.
 * PLATEAU-HANDOFF:func_8004EC60:end
 */
