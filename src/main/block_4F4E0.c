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
/* PROVENANCE: adapted from Jet Force Gemini src/subtitles.c
 * find_next_subtitle and Diddy Kong Racing src/game_text.c find_next_subtitle.
 * Mickey's globals, the unwrapped *6 timer, and linked bytes remain
 * authoritative. Direct global cursor and do-while inner scan closed the
 * extra instruction and extra s-save. sentinel == new_var2 is the 12-compare
 * operand order. Remaining 15 words are the inner u8 taking a0; an identity-
 * gated p2 force of that web onto a1 scores 0. An outer u8 carrier that
 * numbers first reintroduces an s-save. */
#ifdef NON_MATCHING
void func_8004EC60(void) {
    u8 new_var;
    s32 new_var2;
    s32 done;
    s32 sentinel;

    D_800D6AC6 = 0;
    D_800D6ABA = 0;
    sentinel = 12;
    done = FALSE;
    while (D_800D6AE0[0] != 0 && done == FALSE) {
        D_800D6AC8 = D_800D6AE0[0] - 1;
        D_800D6AD0[D_800D6AC6] = (char *)D_800D6AE0;
        D_800D6ABA = D_800D6AE0[7] * 6;
        D_800D6AE0 += 8;
        do {
            new_var = D_800D6AE0[0];
            if (new_var & 0x80) {
                D_800D6AE0 += 2;
            } else {
                D_800D6AE0 += 1;
            }
        } while (D_800D6AE0[0] != 0);
        D_800D6AC6 += 1;
        if (D_800D6AC6 >= 2) {
            done = TRUE;
        }
        D_800D6AE0 += 1;
        new_var2 = D_800D6AE0[0];
        if (D_800D6AE0[0] == 10) {
            D_800D6AE0 += 1;
        } else if (sentinel == new_var2) {
            D_800D6AE0 += 1;
            done = TRUE;
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
 * score: 15/82 words
 * frame: 0x8
 * relocations: 12
 * first-mismatch: +0x2C
 * summary: hypothesis=unforced outer-before-inner byte numbering; spellings=s32 new_var, (u8)(s32) store, (s16)0 index; stall=the widen merges to the 13-word a1/a2 swap and the index cast stays the 15-word pair
 * PLATEAU-HANDOFF:func_8004EC60:end
 */
