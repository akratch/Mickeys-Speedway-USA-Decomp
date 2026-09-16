#include "PR/ultratypes.h"

#define NULL ((void *)0)
#define M2C_UNK s32
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))

/* Same-module callees whose shipped call words are SYMBOL relocation records
 * with a zero addend, so the C names the runtime-table placeholder rather than
 * the in-module definition (CLAUDE.md, "Promoting an overlay function"). */
M2C_UNK overlay86BuildTransformReloc(void *, void *);    /* extern */
M2C_UNK overlay86ScaledVectorPositionReloc(void *, void *, f32 *, f32 *, f32 *); /* extern */
s32 overlay86SelectPositionReloc(void *, void *);        /* extern */
M2C_UNK ext_o0_2b90(M2C_UNK, f32, f32, f32, s32, void *); /* extern */
M2C_UNK ext_o0_2c4c(s32, s32);                     /* extern */
M2C_UNK ext_o0_2c64(s32, s32);                     /* extern */
M2C_UNK ext_o0_1bed0(void *, s32, s32, s32, s32, s32, s32); /* extern */
M2C_UNK ext_o0_2d70(s32, f32, f32, f32);           /* extern */
M2C_UNK ext_o0_2d98();                             /* extern */
M2C_UNK ext_o0_28d88(M2C_UNK);                     /* extern */
s32 ext_o0_2952c(M2C_UNK, M2C_UNK);                /* extern */
M2C_UNK ext_o0_29b94(void *, void *);              /* extern */
f32 ext_o0_2a470(s16);                             /* extern */
void *ext_o0_53d0(u8);                             /* extern */
M2C_UNK ext_o0_494ac(u8, M2C_UNK, f32, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK ext_o0_5a758(void *, M2C_UNK, f32);        /* extern */
void ext_o0_5a914(void *, M2C_UNK, s32, f32);   /* extern */
s16 ext_o0_f690(f32, f32, f32);                    /* extern */
M2C_UNK ext_o7_dbc(M2C_UNK);                       /* extern */

/* Matched 2026-09-16 (lane lm-a), 662/662 words at frame 0xA8, unforced.
 * The fact the score does not show, measured on the allocator's own records
 * (docs/lastmile-region-boundary.md; the shard has the seven passes before):
 * a call's unused result is a v0 web that occupies the call's whole basic
 * block, so a pointer loaded after the call in that block is denied v0 (L101
 * is block-granular).  `ext_o0_5a914` is the last call before both the
 * case-0 `+0x48` access and the `+0x3E0` access, its result is never used,
 * and declaring it `void` removes the web: both loads then take v0 as the ROM
 * does.  An `if (1) { }` region boundary between the call and the load is the
 * byte-identical alternative.  Both accesses are written as the expression
 * itself rather than through the declared `temp_a0` carrier (L145), which
 * keeps `temp_a0` a head-only web on a0 by argument affinity.
 * Also load-bearing: the one-argument `ext_o0_2d98` calls (the ROM cannot
 * distinguish them from two-argument calls whose a1 already holds the
 * pointer, and the arity frees a1 for the command pointer), the split advance
 * `*var_a1++ = 0xC; var_a1 += 5;`, the payload stores in memory order, and the
 * cursor definition folded onto the `0x2C` store line for as1's tie (L59).
 * Eleven declarations give the 0xA8 frame; a twelfth is 0xB0. */
void func_overlay_086_F0000474_18D22AC(void *arg0, s32 arg1) {
    f32 temp_f20;
    s32 spA0;
    f32 sp9C;
    f32 sp98;
    f32 sp94;
    f32 temp_f20_2;
    s32 temp_a0;
    s16 *var_a1;
    void *temp_s0;
    void *temp_s4;
    void *temp_v0;

    temp_f20 = (f32) arg1;
    temp_s0 = M2C_FIELD(arg0, void **, 0x64);
    M2C_FIELD(temp_s0, s16 *, 0x28) = 0x2000;
    temp_v0 = ext_o0_53d0(M2C_FIELD(temp_s0, u8 *, 1));
    if (temp_v0 != NULL) {
        temp_s4 = M2C_FIELD(temp_v0, void **, 0x64);
        spA0 = 0;
        temp_a0 = M2C_FIELD(temp_s0, u8 *, 0);
        if (temp_a0 != 0) {
            ext_o0_5a758(arg0, 0x3C88CE70, temp_f20);
            M2C_FIELD(temp_s0, s16 *, 0x28) = 0x22;
            M2C_FIELD(temp_s0, s16 *, 0x2C) = 0x24; var_a1 = (s16 *)((u8 *)temp_s0 + 0x30);
            M2C_FIELD(temp_s0, s16 *, 0x2A) = M2C_FIELD(temp_s0, s16 *, 0x1C);
            M2C_FIELD(temp_s0, s16 *, 0x2E) = M2C_FIELD(temp_s0, s16 *, 0x1C);
            /* Keep the cursor definition on the 0x2C store line for the as1 tie. */
            if (M2C_FIELD(temp_s0, s16 *, 0x24) != 0) {
                M2C_FIELD(temp_s0, s16 *, 0x26) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x26) + (arg1 << 9));
                if (M2C_FIELD(temp_s0, u8 *, 0) != 2) {
                    M2C_FIELD(temp_s0, s16 *, 0x24) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x24) - (arg1 * 0x11));
                    if (M2C_FIELD(temp_s0, s16 *, 0x24) < 0) {
                        M2C_FIELD(temp_s0, s16 *, 0x24) = 0;
                    }
                    temp_a0 = M2C_FIELD(temp_s0, s32 *, 0x40);
                    if (temp_a0 == 0) {
                        ext_o0_2b90(0x1BE, M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14), 1, (u8 *)temp_s0 + 0x40);
                    } else if (temp_a0 != 0) {
                        ext_o0_2d70(temp_a0, M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                    }
                } else {
                    temp_a0 = M2C_FIELD(temp_s0, s32 *, 0x40);
                    if (temp_a0 != 0) {
                        ext_o0_2d98(temp_a0);
                    }
                }
                spA0 = (s32) (ext_o0_2a470(M2C_FIELD(temp_s0, s16 *, 0x26)) * 1024.0f);
                *var_a1++ = 0xC;
                var_a1 += 5;
                M2C_FIELD(var_a1, s16 *, -0xA) = (s16) M2C_FIELD(temp_s0, s16 *, 0x24);
                M2C_FIELD(var_a1, s16 *, -8) = 3;
                M2C_FIELD(var_a1, s16 *, -6) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x24) * 2);
                M2C_FIELD(var_a1, s16 *, -4) = 0xB;
                M2C_FIELD(var_a1, s16 *, -2) = (s16) spA0;
            } else {
                temp_a0 = M2C_FIELD(temp_s0, s32 *, 0x40);
                if (temp_a0 != 0) {
                    ext_o0_2d98(temp_a0);
                }
            }
            *var_a1 = 0x2000;
        }
        if (arg1 != 0) {
            temp_f20_2 = 0.05f;
            do {
                switch (M2C_FIELD(temp_s0, u8 *, 0)) {
                case 0:
                    if ((M2C_FIELD(temp_s4, u8 *, 0x170) != 0) && !(M2C_FIELD(temp_s4, u16 *, 0x1A8) & 1)) {
                        overlay86ScaledVectorPositionReloc(temp_v0, temp_s4, &sp9C, &sp98, &sp94);
                        M2C_FIELD(arg0, s16 *, 0) = (s16) (M2C_FIELD(temp_v0, s16 *, 0) + 0x8000);
                        M2C_FIELD(arg0, s16 *, 2) = -0x1000;
                        M2C_FIELD(arg0, s16 *, 4) = 0;
                        M2C_FIELD(arg0, f32 *, 0xC) = sp9C;
                        M2C_FIELD(arg0, f32 *, 0x10) = (f32) (sp98 + 320.0f);
                        M2C_FIELD(arg0, f32 *, 0x14) = sp94;
                        M2C_FIELD(arg0, s16 *, 0x2E) = ext_o0_f690(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                        M2C_FIELD(temp_s0, s16 *, 2) = 0x3C;
                        M2C_FIELD(temp_s0, u8 *, 0) = 1U;
                        M2C_FIELD(temp_s0, f32 *, 0x10) = 0.0f;
                        M2C_FIELD(temp_s0, s16 *, 0x24) = 0;
                        M2C_FIELD(temp_s0, f32 *, 0x14) = 320.0f;
                        ext_o0_494ac(M2C_FIELD(temp_s0, u8 *, 1), 0x3F4CCCCD, -1.0f, 0, 0, 0, 0);
                        ext_o0_28d88(0x5A);
                        M2C_FIELD(arg0, s16 *, 6) = (s16) (M2C_FIELD(arg0, s16 *, 6) & 0xFBFF);
                        ext_o0_5a914(arg0, 1, -1, 0.0f);
                        M2C_FIELD(M2C_FIELD(temp_v0, void **, 0x48), u16 *, 6) &= 0xFFFE;
                    } else {
                        arg1 = 0;
                    }
                    break;
                case 1:
                    overlay86ScaledVectorPositionReloc(temp_v0, temp_s4, &sp9C, &sp98, &sp94);
                    if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
loop_26:
                        M2C_FIELD(arg0, f32 *, 0xC) += (sp9C - M2C_FIELD(arg0, f32 *, 0xC)) * 0.1f;
                        M2C_FIELD(arg0, f32 *, 0x14) += (sp94 - M2C_FIELD(arg0, f32 *, 0x14)) * 0.1f;
                        M2C_FIELD(temp_s0, f32 *, 0x10) = M2C_FIELD(temp_s0, f32 *, 0x14) * temp_f20_2;
                        if (M2C_FIELD(temp_s0, f32 *, 0x10) > 10.0f) {
                            M2C_FIELD(temp_s0, f32 *, 0x10) = 10.0f;
                        }
                        arg1 -= 1;
                        M2C_FIELD(temp_s0, s16 *, 2) = (s16) (M2C_FIELD(temp_s0, s16 *, 2) - 1);
                        M2C_FIELD(temp_s0, f32 *, 0x14) = (f32) (M2C_FIELD(temp_s0, f32 *, 0x14) - M2C_FIELD(temp_s0, f32 *, 0x10));
                        if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
                            goto loop_26;
                        }
                    }
                    M2C_FIELD(arg0, f32 *, 0x10) = (f32) (M2C_FIELD(temp_s0, f32 *, 0x14) + sp98);
                    if (M2C_FIELD(temp_s0, s16 *, 2) == 0) {
                        M2C_FIELD(arg0, s16 *, 0) = (s16) (overlay86SelectPositionReloc(temp_v0, temp_s0) + 0x8000);
                        M2C_FIELD(arg0, s16 *, 2) = -0x1000;
                        M2C_FIELD(arg0, f32 *, 0xC) = (f32) M2C_FIELD(temp_s0, f32 *, 4);
                        M2C_FIELD(arg0, f32 *, 0x10) = (f32) (M2C_FIELD(temp_s0, f32 *, 8) + 400.0f);
                        M2C_FIELD(arg0, f32 *, 0x14) = (f32) M2C_FIELD(temp_s0, f32 *, 0xC);
                        overlay86BuildTransformReloc(arg0, temp_v0);
                        ext_o0_1bed0(temp_v0, M2C_FIELD(temp_v0, s32 *, 0xC), M2C_FIELD(temp_v0, s32 *, 0x10), M2C_FIELD(temp_v0, s32 *, 0x14), (s32) M2C_FIELD(temp_v0, s16 *, 0), (s32) M2C_FIELD(temp_v0, s16 *, 2), (s32) M2C_FIELD(temp_v0, s16 *, 4));
                        M2C_FIELD(temp_s4, s8 *, 0x191) = 1;
                        M2C_FIELD(temp_s4, u8 *, 0x170) = 0U;
                        M2C_FIELD(temp_s4, u16 *, 0x1A8) = (u16) (M2C_FIELD(temp_s4, u16 *, 0x1A8) & 0xFFF7);
                        M2C_FIELD(temp_s0, u8 *, 0) = 2U;
                        M2C_FIELD(temp_s0, s16 *, 0x24) = 0x400;
                        M2C_FIELD(temp_s0, s16 *, 2) = 0x5A;
                        ext_o0_494ac(M2C_FIELD(temp_s0, u8 *, 1), 0x3EB33333, 0.0f, 0, 0, 0, 0x80);
                        ext_o7_dbc(9);
                    }
                    temp_a0 = ext_o0_f690(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                    if (temp_a0 != -1) {
                        M2C_FIELD(arg0, s16 *, 0x2E) = temp_a0;
                    }
                    break;
                case 2:
                case 3:
                    if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
loop_37:
                        M2C_FIELD(temp_s0, f32 *, 0x10) = (f32) ((M2C_FIELD(temp_s0, f32 *, 8) - M2C_FIELD(arg0, f32 *, 0x10)) * temp_f20_2);
                        if (M2C_FIELD(temp_s0, f32 *, 0x10) < -6.0f) {
                            M2C_FIELD(temp_s0, f32 *, 0x10) = -6.0f;
                        }
                        arg1 -= 1;
                        M2C_FIELD(arg0, f32 *, 0x10) = (f32) (M2C_FIELD(temp_s0, f32 *, 0x10) + M2C_FIELD(arg0, f32 *, 0x10));
                        M2C_FIELD(temp_s0, s16 *, 2) = (s16) (M2C_FIELD(temp_s0, s16 *, 2) - 1);
                        if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
                            goto loop_37;
                        }
                    }
                    overlay86BuildTransformReloc(arg0, temp_v0);
                    M2C_FIELD(temp_v0, s16 *, 4) = (s16) spA0;
                    if (M2C_FIELD(temp_s0, s16 *, 2) == 0) {
                        if (M2C_FIELD(temp_s0, u8 *, 0) == 2) {
                            M2C_FIELD(temp_s0, s16 *, 2) = 0x1E;
                            M2C_FIELD(temp_s0, u8 *, 0) = 3U;
                        } else {
                            ext_o0_2b90(0x1BF, M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14), 4, NULL);
                            M2C_FIELD(temp_s4, s8 *, 0x191) = 0;
                            M2C_FIELD(temp_s4, s8 *, 0x16C) = 1;
                            ext_o0_5a914(temp_v0, 0xC, -1, 0.0f);
                            M2C_FIELD(temp_s0, s16 *, 0x1E) = 0;
                            M2C_FIELD(temp_s0, s16 *, 0x20) = 0;
                            M2C_FIELD(temp_s0, s16 *, 0x22) = 0;
                            M2C_FIELD(temp_s0, s16 *, 2) = 0xB4;
                            M2C_FIELD(temp_s0, u8 *, 0) = 4U;
                            M2C_FIELD(temp_s0, f32 *, 0x10) = 0;
                            if (M2C_FIELD(temp_s4, void **, 0x3E0) != NULL) {
                                M2C_FIELD(M2C_FIELD(temp_s4, void **, 0x3E0), u16 *, 0x10) &= 0xFFF7;
                                M2C_FIELD(temp_s4, void **, 0x3E0) = NULL;
                            }
                            M2C_FIELD(M2C_FIELD(temp_v0, void **, 0x48), u16 *, 6) |= 1;
                        }
                    }
                    temp_a0 = ext_o0_f690(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                    if (temp_a0 != -1) {
                        M2C_FIELD(arg0, s16 *, 0x2E) = temp_a0;
                    }
                    break;
                case 4:
                    if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
loop_52:
                        M2C_FIELD(temp_s0, f32 *, 0x10) = (f32) (M2C_FIELD(temp_s0, f32 *, 0x10) + temp_f20_2);
                        M2C_FIELD(arg0, s16 *, 0) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x1E) + M2C_FIELD(arg0, s16 *, 0));
                        M2C_FIELD(arg0, s16 *, 2) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x20) + M2C_FIELD(arg0, s16 *, 2));
                        M2C_FIELD(arg0, s16 *, 4) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x22) + M2C_FIELD(arg0, s16 *, 4));
                        if (M2C_FIELD(temp_s0, s16 *, 0x1E) >= -0x3F) {
                            M2C_FIELD(temp_s0, s16 *, 0x1E) -= 2;
                        }
                        if (M2C_FIELD(temp_s0, s16 *, 0x20) < 0x80) {
                            M2C_FIELD(temp_s0, s16 *, 0x20) += 8;
                        }
                        if (M2C_FIELD(temp_s0, s16 *, 0x22) >= -0x7F) {
                            M2C_FIELD(temp_s0, s16 *, 0x22) -= 8;
                        }
                        arg1 -= 1;
                        M2C_FIELD(temp_s0, s16 *, 2) = (s16) (M2C_FIELD(temp_s0, s16 *, 2) - 1);
                        if ((M2C_FIELD(temp_s0, s16 *, 2) != 0) && (arg1 != 0)) {
                            goto loop_52;
                        }
                    }
                    if (M2C_FIELD(temp_s0, s16 *, 2) == 0) {
                        M2C_FIELD(arg0, s16 *, 0x2E) = -1;
                        M2C_FIELD(arg0, s16 *, 6) = (s16) (M2C_FIELD(arg0, s16 *, 6) | 0x400);
                        M2C_FIELD(temp_s0, u8 *, 0) = 0U;
                    } else {
                        if (M2C_FIELD(temp_s0, f32 *, 0x10) > 5.0f) {
                            M2C_FIELD(temp_s0, f32 *, 0x10) = 5.0f;
                        }
                        if (M2C_FIELD(arg0, s16 *, 2) >= 0x4001) {
                            M2C_FIELD(arg0, s16 *, 2) = 0x4000;
                        }
                        if (M2C_FIELD(arg0, s16 *, 4) < -0x2000) {
                            M2C_FIELD(arg0, s16 *, 4) = -0x2000;
                        }
                        M2C_FIELD(arg0, f32 *, 0x24) = (f32) (-M2C_FIELD(temp_s0, f32 *, 0x10) * temp_f20);
                        ext_o0_29b94(arg0, (u8 *)arg0 + 0x1C);
                        M2C_FIELD(arg0, f32 *, 0xC) += M2C_FIELD(arg0, f32 *, 0x1C) * temp_f20;
                        M2C_FIELD(arg0, f32 *, 0x10) += M2C_FIELD(arg0, f32 *, 0x20) * temp_f20;
                        M2C_FIELD(arg0, f32 *, 0x14) += M2C_FIELD(arg0, f32 *, 0x24) * temp_f20;
                        temp_a0 = ext_o0_f690(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                        if (temp_a0 != -1) {
                            M2C_FIELD(arg0, s16 *, 0x2E) = temp_a0;
                        }
                    }
                    break;
                }
            } while (arg1 != 0);
        }
        if (M2C_FIELD(temp_s0, u8 *, 0) != 0) {
            arg1 = M2C_FIELD(temp_s0, s32 *, 0x18);
            if (arg1 == 0) {
                ext_o0_2b90(0x16, M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14), 1, (u8 *)temp_s0 + 0x18);
                arg1 = M2C_FIELD(temp_s0, s32 *, 0x18);
                if (arg1 != 0) {
                    ext_o0_2c4c(arg1, 0x7F);
                }
            }
            arg1 = M2C_FIELD(temp_s0, s32 *, 0x18);
            if (arg1 != 0) {
                temp_f20_2 = M2C_FIELD(temp_s0, f32 *, 0x10) * 8.0f;
                if (temp_f20_2 < 0.0f) {
                    temp_f20_2 = -temp_f20_2;
                }
                temp_f20_2 += 100.0f;
                if (temp_f20_2 > 150.0f) {
                    temp_f20_2 = 150.0f;
                }
                temp_f20_2 += (f32) ext_o0_2952c(-5, 5);
                ext_o0_2d70(M2C_FIELD(temp_s0, s32 *, 0x18), M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x10), M2C_FIELD(arg0, f32 *, 0x14));
                ext_o0_2c64(M2C_FIELD(temp_s0, s32 *, 0x18), (u32) temp_f20_2 & 0xFF);
                if (M2C_FIELD(temp_s0, u8 *, 0) == 4) {
                    ext_o0_2c4c(M2C_FIELD(temp_s0, s32 *, 0x18), ((s32) (M2C_FIELD(temp_s0, s16 *, 2) * 0x7F) / 180) & 0xFF);
                }
            }
            spA0 = (s32) (M2C_FIELD(temp_s0, f32 *, 0x10) * 256.0f);
            if (spA0 < 0) {
                spA0 = -spA0;
            }
            spA0 += 0x800;
            if (spA0 >= 0x1001) {
                spA0 = 0x1000;
            }
            M2C_FIELD(temp_s0, s16 *, 0x1C) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x1C) + (spA0 * (s32) temp_f20));
            return;
        }
        arg1 = M2C_FIELD(temp_s0, s32 *, 0x18);
        if (arg1 != 0) {
            ext_o0_2d98(arg1);
        }
    }
}
