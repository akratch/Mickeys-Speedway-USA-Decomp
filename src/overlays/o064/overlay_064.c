#include "overlays/overlay_064.h"

/*
 * Rewritten from the listing (lane w2-ovld, 2026-10-02): every loop is
 * `while (n--)` (the target's dead copy of the old counter after each test),
 * one counter for the outer and single loops and a second for the inner ones,
 * a u8 local for the buffer select, innerWidth (now temp) as a variable so the counter is
 * built from it, and the clamp limit written back into `distance` so it is
 * computed before the call and saved in that local's own cell. 382 masked at
 * delta 0 against 401. Open: the frame is 0x78 against 0x70 (five compiler
 * temporaries against four), and the target keeps width in ra and the
 * counter in t5. 380 -> 372 (lane q-3): both smoothing loops hold their
 * sum in lo rather than value, so value's web no longer spans them.
 * 372 -> 371 (lane q-3): the first sum in the target's add order with in++
 * after the first clamp, not a natural placement; an in++ after the
 * loads' block stops uopt's loop-carried load webs (shard, 2026-10-08).
 * The second smoothing sum through pointer expressions rather than negative
 * subscripts: masked unchanged (371 at 0), aligned residual 297 -> 283, out
 * now takes a2 as shipped (lane opus-o064, 2026-10-14). 371 -> 369: the
 * product as `lo *= value >> 4`, which gives the shipped multiply operand order.
 * 369 -> 355 (2026-10-14): the pixel rows walk through dst (a byte row base;
 * the even rows store at dst + (distance << 1) + (j << 1), so j is the only
 * induction variable), the texel is read twice so the hi clamp compares hi,
 * and the second smoothing loop steps three pointers as shipped.
 * 355 -> 270 (2026-10-14, third pass): pixels walks the rows and dst is the
 * row base copied at each row top, the even-row address is assigned to pixels
 * before the texel; `value = lo` after the product stops uopt forwarding
 * value + 0x34 into the hi clamp; innerHeight is written as height - 6 and
 * its local becomes the smoothing loops' counter k (270 -> 264); the entry select reads the
 * byte through a signed char view, which makes it a different code-motion
 * item from the toggle's read, so the select load is no longer re-inserted
 * at both loop exits and the frame is 0x70 as shipped.
 * 264 -> 256 (2026-10-15, sixth pass): carriers read from the target's
 * registers. The first loops' distance magnitude is held in value (v1 there,
 * as in the pixel loops), so distance carries only the clamp limit and the
 * even-row offset (a3 in all three). The smoothing sums are held in the
 * width - 6 local (renamed temp), not lo, so the sum takes v0 and the
 * below pointer v1 as shipped; lo no longer spans the smoothing loops.
 * 252 -> 203 (2026-10-15, seventh pass): a region-by-region local rename
 * scan (each local's occurrences in one loop renamed to every other declared
 * local of its class that the loop does not use, behaviour checked by a host
 * harness, best adopted, rescanned): the second smoothing sum in j, the even
 * rows' lo in i, the first smoothing loop's counter in j and its sum in lo,
 * the copy loop and the first noise loop counting with k.
 * 203 -> 191: the first smoothing sum built in two statements, the first
 * three terms and then the fourth with the shift (a permuter cell's
 * redundant double mask on in[width - 1] at 168 showed one extra scratch
 * draw there; this is the natural spelling nearest it).
 * 191 -> 190: the noise value as *out + mathRnd(...), the shipped add order.
 * 190 -> 175: one counter k for both noise loops, assigned width - 6 once
 * before the branch (the target keeps one counter there), then a rescan:
 * the five-row smoothing loop counts with lo, its sum is held in distance,
 * the copy loop counts with distance. temp is now unused; deleting its
 * declaration moves the frame offsets and costs 17 words (192).
 * 175 -> 129 (2026-10-15, twelfth pass): the select byte and the
 * initialised flag are function-scope statics (the TU's 16-byte .data,
 * select at 0 and the flag at 4, the layout overlay64State.c gives them now).
 * uopt promotes a file-scope or extern scalar into a register across the
 * function and re-inserts its load at both noise-loop exits; it does not
 * promote a function static, so the entry test and `^= 1` read it plainly
 * and the toggle is exact (185, with no cast view). Then the first smoothing
 * sum as one expression shifted in place, centre texel first and the row
 * below's centre last (129), the second sum left to right and its in++
 * after the first clamp.
 * 129 -> 65 (thirteenth pass): k counts every outer loop (noise, both
 * smoothing passes, the copy, the rows), so the post-decrement temporary
 * no longer meets distance and takes a3; both smoothing sums are held in i
 * (v0); the pixel products are lo in both row kinds; the texel's high
 * nibble is named (temp), which takes the register ahead of lo; the first
 * smoothing pass counts its columns with temp too. select is still unused.
 * 65 -> 6: k declared first (its home is the first local slot, as shipped);
 * both smoothing passes store and then step in; the second sum takes the
 * two side texels before the centre; the even rows step in after the
 * product.
 * 6 -> 4: the row stride is read through a copy, `i = width` before the
 * row loop and `dst + (i << 1)` after the even rows. uopt numbers an
 * expression when it is first hashed: reading (codeimage, source order)
 * first, then the unroller's new copies, then copy propagation. `width << 1`
 * written directly is numbered at reading, between the even loop's `j + 1`
 * (its own increment) and the unroller's `j + 2`, and spilltemps serves
 * slots in that order, so it took 0x1C. Reached by propagating the copy, it
 * is numbered after `j + 2` and shares 0x18 with `width >> 1` as shipped.
 * 4 -> 5 (2026-10-16): pass 1 counts with j, both pixel loops assign lo
 * before the nibble, and the copy loop counts with temp. That last change
 * keeps the nibble live across an n-- test, so the shared while (n--)
 * temporary takes a3. k |= 0 at the top of the row loop is deleted by as1
 * and puts k back in t5 and width back in ra. The five words are the copy
 * counter, which is that same nibble web.
 * Putting the copy loop back on k (2026-10-16) returns that counter to t5
 * and drops web 42 off a3. A nibble use in the odd while test, an empty
 * if on temp, and a named pass-2 pointer all do that: the nibble's save
 * falls from 300 to about 119 once its span grows; web 42 remains at 181.
 * ADR 0017: k |= 0 and lo |= temp * 0 are initialized integer identities,
 * retained for exact allocation; cleaner forms remain in docs/cleanup-queue.md.
 */

void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image, u8 *unused)
{
    s32 k;
    u8 select;
    s32 width;
    s32 height;
    s32 temp;
    s32 i;
    s32 j;
    s32 value;
    s32 hi;
    u8 *out;
    u8 *in;
    u8 *dst;
    u16 *pixels;
    u8 *src;
    static u8 sBufferSelect = 0;
    static s32 sInitialized = 0;
    s32 lo;
    s32 distance;

    width = image->width;
    height = image->height;
    if (sBufferSelect) {
        src = D_800D60B0[index];
    } else {
        src = D_800D60C0[index];
    }

    k = width - 6;
    out = src + (height - 6) * width + 3;
    if (!sInitialized) {
        while (k--) {
            value = (width >> 1) - k - 3;
            if (value <= 0) {
                value = -value;
            }
            *out++ = mathRnd(0, 0xFF - (value << 4));
        }
        sInitialized = 1;
    } else {
        while (k--) {
            value = (width >> 1) - k - 3;
            if (value <= 0) {
                value = -value;
            }
            distance = 0xFF - (value << 4);
            value = *out + mathRnd(-0x60, 0x60);
            if (value > distance) {
                value = distance;
            }
            if (value < 0) {
                value = 0;
            }
            *out++ = value;
        }
    }

    sBufferSelect ^= 1;
    if (sBufferSelect) {
        dst = D_800D60B0[index];
    } else {
        dst = D_800D60C0[index];
    }

    in = src;
    out = dst;
    k = height - 6;
    while (k--) {
        *out++ = 0;
        in++;
        j = width - 2;
        while (j--) {
            i = in[0] * 2 + in[width - 1] + in[width + 1] + in[width] * 4;
            i >>= 3;
            if (i < 0xAA) {
                i--;
            }
            if (i < 0) {
                i = 0;
            }
            *out++ = i;
            in++;
        }
        *out++ = 0;
        in++;
    }

    out += width;
    in += width;
    k = 5;
    while (k--) {
        *out++ = 0;
        in++;
        j = width - 2;
        while (j--) {
            i = (*(in - 1 - width) + *(in + 1 - width) + *(in - width)) >> 2;
            if (i < 0xAA) {
                i--;
            }
            if (i < 0) {
                i = 0;
            }
            *out++ = i;
            in++;
        }
        *out++ = 0;
        in++;
    }

    out = dst + (height - 6) * width;
    in = out - width;
    k = width;
    while (k--) {
        *out++ = *in++;
    }

    in = dst;
    pixels = image->pixels;
    i = width;
    k = height;
    while (k--) {
        k |= 0;
        dst = (u8 *)pixels;
        if (k & 1) {
            j = width;
            while (j--) {
                value = *in++;
                lo = value;
                temp = value >> 4;
                hi = value + 0x34;
                if (temp) {
                    lo *= temp;
                    value = lo;
                }
                if (hi >= 0x100) {
                    hi = 0xFF;
                }
                if (lo >= 0x100) {
                    lo = 0xFF;
                }
                lo |= temp * 0;
                *pixels++ = (hi << 8) | lo;
            }
        } else {
            for (j = 0; j < width; j++) {
                if (j & 2) {
                    distance = -2;
                } else {
                    distance = 2;
                }
                pixels = (u16 *)(dst + (distance << 1) + (j << 1));
                value = *in;
                lo = value;
                temp = value >> 4;
                hi = value + 0x34;
                if (temp) {
                    lo *= temp;
                    value = lo;
                }
                in++;
                if (hi >= 0x100) {
                    hi = 0xFF;
                }
                if (lo >= 0x100) {
                    lo = 0xFF;
                }
                *pixels = (hi << 8) | lo;
            }
            pixels = (u16 *)(dst + (i << 1));
        }
    }
}
/* Preserve source-line positions of the retired assembly fallback.











 */
