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
 * itself rather than through the declared `result` carrier (L145), which
 * keeps `result` a head-only web on a0 by argument affinity.
 * Also load-bearing: the one-argument `ext_o0_2d98` calls (the ROM cannot
 * distinguish them from two-argument calls whose a1 already holds the
 * pointer, and the arity frees a1 for the command pointer), the split advance
 * `*cursor++ = 0xC; cursor += 5;`, the payload stores in memory order, and the
 * cursor definition folded onto the `0x2C` store line for as1's tie (L59).
 * Eleven declarations give the 0xA8 frame; a twelfth is 0xB0. */
void func_overlay_086_F0000474_18D22AC(void *object, s32 steps) {
    f32 elapsed;
    s32 shake;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 scale;
    s32 result;
    s16 *cursor;
    void *state;
    void *racer;
    void *target;

    elapsed = (f32) steps;
    state = M2C_FIELD(object, void **, 0x64);
    M2C_FIELD(state, s16 *, 0x28) = 0x2000;
    target = ext_o0_53d0(M2C_FIELD(state, u8 *, 1));
    if (target != NULL) {
        racer = M2C_FIELD(target, void **, 0x64);
        shake = 0;
        result = M2C_FIELD(state, u8 *, 0);
        if (result != 0) {
            ext_o0_5a758(object, 0x3C88CE70, elapsed);
            M2C_FIELD(state, s16 *, 0x28) = 0x22;
            M2C_FIELD(state, s16 *, 0x2C) = 0x24; cursor = (s16 *)((u8 *)state + 0x30);
            M2C_FIELD(state, s16 *, 0x2A) = M2C_FIELD(state, s16 *, 0x1C);
            M2C_FIELD(state, s16 *, 0x2E) = M2C_FIELD(state, s16 *, 0x1C);
            /* Keep the cursor definition on the 0x2C store line for the as1 tie. */
            if (M2C_FIELD(state, s16 *, 0x24) != 0) {
                M2C_FIELD(state, s16 *, 0x26) = (s16) (M2C_FIELD(state, s16 *, 0x26) + (steps << 9));
                if (M2C_FIELD(state, u8 *, 0) != 2) {
                    M2C_FIELD(state, s16 *, 0x24) = (s16) (M2C_FIELD(state, s16 *, 0x24) - (steps * 0x11));
                    if (M2C_FIELD(state, s16 *, 0x24) < 0) {
                        M2C_FIELD(state, s16 *, 0x24) = 0;
                    }
                    result = M2C_FIELD(state, s32 *, 0x40);
                    if (result == 0) {
                        ext_o0_2b90(0x1BE, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 1, (u8 *)state + 0x40);
                    } else if (result != 0) {
                        ext_o0_2d70(result, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    }
                } else {
                    result = M2C_FIELD(state, s32 *, 0x40);
                    if (result != 0) {
                        ext_o0_2d98(result);
                    }
                }
                shake = (s32) (ext_o0_2a470(M2C_FIELD(state, s16 *, 0x26)) * 1024.0f);
                *cursor++ = 0xC;
                cursor += 5;
                M2C_FIELD(cursor, s16 *, -0xA) = (s16) M2C_FIELD(state, s16 *, 0x24);
                M2C_FIELD(cursor, s16 *, -8) = 3;
                M2C_FIELD(cursor, s16 *, -6) = (s16) (M2C_FIELD(state, s16 *, 0x24) * 2);
                M2C_FIELD(cursor, s16 *, -4) = 0xB;
                M2C_FIELD(cursor, s16 *, -2) = (s16) shake;
            } else {
                result = M2C_FIELD(state, s32 *, 0x40);
                if (result != 0) {
                    ext_o0_2d98(result);
                }
            }
            *cursor = 0x2000;
        }
        if (steps != 0) {
            scale = 0.05f;
            do {
                switch (M2C_FIELD(state, u8 *, 0)) {
                case 0:
                    if ((M2C_FIELD(racer, u8 *, 0x170) != 0) && !(M2C_FIELD(racer, u16 *, 0x1A8) & 1)) {
                        overlay86ScaledVectorPositionReloc(target, racer, &targetX, &targetY, &targetZ);
                        M2C_FIELD(object, s16 *, 0) = (s16) (M2C_FIELD(target, s16 *, 0) + 0x8000);
                        M2C_FIELD(object, s16 *, 2) = -0x1000;
                        M2C_FIELD(object, s16 *, 4) = 0;
                        M2C_FIELD(object, f32 *, 0xC) = targetX;
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (targetY + 320.0f);
                        M2C_FIELD(object, f32 *, 0x14) = targetZ;
                        M2C_FIELD(object, s16 *, 0x2E) = ext_o0_f690(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                        M2C_FIELD(state, s16 *, 2) = 0x3C;
                        M2C_FIELD(state, u8 *, 0) = 1U;
                        M2C_FIELD(state, f32 *, 0x10) = 0.0f;
                        M2C_FIELD(state, s16 *, 0x24) = 0;
                        M2C_FIELD(state, f32 *, 0x14) = 320.0f;
                        ext_o0_494ac(M2C_FIELD(state, u8 *, 1), 0x3F4CCCCD, -1.0f, 0, 0, 0, 0);
                        ext_o0_28d88(0x5A);
                        M2C_FIELD(object, s16 *, 6) = (s16) (M2C_FIELD(object, s16 *, 6) & 0xFBFF);
                        ext_o0_5a914(object, 1, -1, 0.0f);
                        M2C_FIELD(M2C_FIELD(target, void **, 0x48), u16 *, 6) &= 0xFFFE;
                    } else {
                        steps = 0;
                    }
                    break;
                case 1:
                    overlay86ScaledVectorPositionReloc(target, racer, &targetX, &targetY, &targetZ);
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_26:
                        M2C_FIELD(object, f32 *, 0xC) += (targetX - M2C_FIELD(object, f32 *, 0xC)) * 0.1f;
                        M2C_FIELD(object, f32 *, 0x14) += (targetZ - M2C_FIELD(object, f32 *, 0x14)) * 0.1f;
                        M2C_FIELD(state, f32 *, 0x10) = M2C_FIELD(state, f32 *, 0x14) * scale;
                        if (M2C_FIELD(state, f32 *, 0x10) > 10.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = 10.0f;
                        }
                        steps -= 1;
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        M2C_FIELD(state, f32 *, 0x14) = (f32) (M2C_FIELD(state, f32 *, 0x14) - M2C_FIELD(state, f32 *, 0x10));
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_26;
                        }
                    }
                    M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x14) + targetY);
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        M2C_FIELD(object, s16 *, 0) = (s16) (overlay86SelectPositionReloc(target, state) + 0x8000);
                        M2C_FIELD(object, s16 *, 2) = -0x1000;
                        M2C_FIELD(object, f32 *, 0xC) = (f32) M2C_FIELD(state, f32 *, 4);
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 8) + 400.0f);
                        M2C_FIELD(object, f32 *, 0x14) = (f32) M2C_FIELD(state, f32 *, 0xC);
                        overlay86BuildTransformReloc(object, target);
                        ext_o0_1bed0(target, M2C_FIELD(target, s32 *, 0xC), M2C_FIELD(target, s32 *, 0x10), M2C_FIELD(target, s32 *, 0x14), (s32) M2C_FIELD(target, s16 *, 0), (s32) M2C_FIELD(target, s16 *, 2), (s32) M2C_FIELD(target, s16 *, 4));
                        M2C_FIELD(racer, s8 *, 0x191) = 1;
                        M2C_FIELD(racer, u8 *, 0x170) = 0U;
                        M2C_FIELD(racer, u16 *, 0x1A8) = (u16) (M2C_FIELD(racer, u16 *, 0x1A8) & 0xFFF7);
                        M2C_FIELD(state, u8 *, 0) = 2U;
                        M2C_FIELD(state, s16 *, 0x24) = 0x400;
                        M2C_FIELD(state, s16 *, 2) = 0x5A;
                        ext_o0_494ac(M2C_FIELD(state, u8 *, 1), 0x3EB33333, 0.0f, 0, 0, 0, 0x80);
                        ext_o7_dbc(9);
                    }
                    result = ext_o0_f690(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    if (result != -1) {
                        M2C_FIELD(object, s16 *, 0x2E) = result;
                    }
                    break;
                case 2:
                case 3:
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_37:
                        M2C_FIELD(state, f32 *, 0x10) = (f32) ((M2C_FIELD(state, f32 *, 8) - M2C_FIELD(object, f32 *, 0x10)) * scale);
                        if (M2C_FIELD(state, f32 *, 0x10) < -6.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = -6.0f;
                        }
                        steps -= 1;
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x10) + M2C_FIELD(object, f32 *, 0x10));
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_37;
                        }
                    }
                    overlay86BuildTransformReloc(object, target);
                    M2C_FIELD(target, s16 *, 4) = (s16) shake;
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        if (M2C_FIELD(state, u8 *, 0) == 2) {
                            M2C_FIELD(state, s16 *, 2) = 0x1E;
                            M2C_FIELD(state, u8 *, 0) = 3U;
                        } else {
                            ext_o0_2b90(0x1BF, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 4, NULL);
                            M2C_FIELD(racer, s8 *, 0x191) = 0;
                            M2C_FIELD(racer, s8 *, 0x16C) = 1;
                            ext_o0_5a914(target, 0xC, -1, 0.0f);
                            M2C_FIELD(state, s16 *, 0x1E) = 0;
                            M2C_FIELD(state, s16 *, 0x20) = 0;
                            M2C_FIELD(state, s16 *, 0x22) = 0;
                            M2C_FIELD(state, s16 *, 2) = 0xB4;
                            M2C_FIELD(state, u8 *, 0) = 4U;
                            M2C_FIELD(state, f32 *, 0x10) = 0;
                            if (M2C_FIELD(racer, void **, 0x3E0) != NULL) {
                                M2C_FIELD(M2C_FIELD(racer, void **, 0x3E0), u16 *, 0x10) &= 0xFFF7;
                                M2C_FIELD(racer, void **, 0x3E0) = NULL;
                            }
                            M2C_FIELD(M2C_FIELD(target, void **, 0x48), u16 *, 6) |= 1;
                        }
                    }
                    result = ext_o0_f690(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    if (result != -1) {
                        M2C_FIELD(object, s16 *, 0x2E) = result;
                    }
                    break;
                case 4:
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_52:
                        M2C_FIELD(state, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x10) + scale);
                        M2C_FIELD(object, s16 *, 0) = (s16) (M2C_FIELD(state, s16 *, 0x1E) + M2C_FIELD(object, s16 *, 0));
                        M2C_FIELD(object, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 0x20) + M2C_FIELD(object, s16 *, 2));
                        M2C_FIELD(object, s16 *, 4) = (s16) (M2C_FIELD(state, s16 *, 0x22) + M2C_FIELD(object, s16 *, 4));
                        if (M2C_FIELD(state, s16 *, 0x1E) >= -0x3F) {
                            M2C_FIELD(state, s16 *, 0x1E) -= 2;
                        }
                        if (M2C_FIELD(state, s16 *, 0x20) < 0x80) {
                            M2C_FIELD(state, s16 *, 0x20) += 8;
                        }
                        if (M2C_FIELD(state, s16 *, 0x22) >= -0x7F) {
                            M2C_FIELD(state, s16 *, 0x22) -= 8;
                        }
                        steps -= 1;
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_52;
                        }
                    }
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        M2C_FIELD(object, s16 *, 0x2E) = -1;
                        M2C_FIELD(object, s16 *, 6) = (s16) (M2C_FIELD(object, s16 *, 6) | 0x400);
                        M2C_FIELD(state, u8 *, 0) = 0U;
                    } else {
                        if (M2C_FIELD(state, f32 *, 0x10) > 5.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = 5.0f;
                        }
                        if (M2C_FIELD(object, s16 *, 2) >= 0x4001) {
                            M2C_FIELD(object, s16 *, 2) = 0x4000;
                        }
                        if (M2C_FIELD(object, s16 *, 4) < -0x2000) {
                            M2C_FIELD(object, s16 *, 4) = -0x2000;
                        }
                        M2C_FIELD(object, f32 *, 0x24) = (f32) (-M2C_FIELD(state, f32 *, 0x10) * elapsed);
                        ext_o0_29b94(object, (u8 *)object + 0x1C);
                        M2C_FIELD(object, f32 *, 0xC) += M2C_FIELD(object, f32 *, 0x1C) * elapsed;
                        M2C_FIELD(object, f32 *, 0x10) += M2C_FIELD(object, f32 *, 0x20) * elapsed;
                        M2C_FIELD(object, f32 *, 0x14) += M2C_FIELD(object, f32 *, 0x24) * elapsed;
                        result = ext_o0_f690(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                        if (result != -1) {
                            M2C_FIELD(object, s16 *, 0x2E) = result;
                        }
                    }
                    break;
                }
            } while (steps != 0);
        }
        if (M2C_FIELD(state, u8 *, 0) != 0) {
            steps = M2C_FIELD(state, s32 *, 0x18);
            if (steps == 0) {
                ext_o0_2b90(0x16, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 1, (u8 *)state + 0x18);
                steps = M2C_FIELD(state, s32 *, 0x18);
                if (steps != 0) {
                    ext_o0_2c4c(steps, 0x7F);
                }
            }
            steps = M2C_FIELD(state, s32 *, 0x18);
            if (steps != 0) {
                scale = M2C_FIELD(state, f32 *, 0x10) * 8.0f;
                if (scale < 0.0f) {
                    scale = -scale;
                }
                scale += 100.0f;
                if (scale > 150.0f) {
                    scale = 150.0f;
                }
                scale += (f32) ext_o0_2952c(-5, 5);
                ext_o0_2d70(M2C_FIELD(state, s32 *, 0x18), M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                ext_o0_2c64(M2C_FIELD(state, s32 *, 0x18), (u32) scale & 0xFF);
                if (M2C_FIELD(state, u8 *, 0) == 4) {
                    ext_o0_2c4c(M2C_FIELD(state, s32 *, 0x18), ((s32) (M2C_FIELD(state, s16 *, 2) * 0x7F) / 180) & 0xFF);
                }
            }
            shake = (s32) (M2C_FIELD(state, f32 *, 0x10) * 256.0f);
            if (shake < 0) {
                shake = -shake;
            }
            shake += 0x800;
            if (shake >= 0x1001) {
                shake = 0x1000;
            }
            M2C_FIELD(state, s16 *, 0x1C) = (s16) (M2C_FIELD(state, s16 *, 0x1C) + (shake * (s32) elapsed));
            return;
        }
        steps = M2C_FIELD(state, s32 *, 0x18);
        if (steps != 0) {
            ext_o0_2d98(steps);
        }
    }
}
