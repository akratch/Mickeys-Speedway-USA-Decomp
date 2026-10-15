typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;

typedef struct O64Image {
    u8 pad00[6];
    u16 width;
    u16 height;
    u8 pad0A[0x16];
    u16 pixels[1];
} O64Image;

extern u8 gO64BufferSelect;
extern s32 gO64Initialized;
extern void *D_800D60B0[4];
extern void *D_800D60C0[4];
extern s32 mathRnd(s32 minimum, s32 maximum);

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
 */
#ifdef NON_MATCHING
void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image, u8 *unused)
{
    s32 i;
    u8 select;
    s32 width;
    s32 height;
    s32 temp;
    s32 k;
    s32 j;
    s32 value;
    s32 hi;
    u8 *out;
    u8 *in;
    u8 *dst;
    u16 *pixels;
    u8 *src;
    s32 lo;
    s32 distance;

    width = image->width;
    height = image->height;
    if ((u8)*(signed char *)&gO64BufferSelect) {
        src = D_800D60B0[index];
    } else {
        src = D_800D60C0[index];
    }

    temp = width - 6;
    out = src + (height - 6) * width + 3;
    if (!gO64Initialized) {
        k = temp;
        while (k--) {
            value = (width >> 1) - k - 3;
            if (value <= 0) {
                value = -value;
            }
            *out++ = mathRnd(0, 0xFF - (value << 4));
        }
        gO64Initialized = 1;
    } else {
        i = temp;
        while (i--) {
            value = (width >> 1) - i - 3;
            if (value <= 0) {
                value = -value;
            }
            distance = 0xFF - (value << 4);
            value = mathRnd(-0x60, 0x60) + *out;
            if (value > distance) {
                value = distance;
            }
            if (value < 0) {
                value = 0;
            }
            *out++ = value;
        }
    }

    select = gO64BufferSelect ^ 1;
    gO64BufferSelect = select;
    if (select) {
        dst = D_800D60B0[index];
    } else {
        dst = D_800D60C0[index];
    }

    in = src;
    out = dst;
    i = height - 6;
    while (i--) {
        *out++ = 0;
        in++;
        j = width - 2;
        while (j--) {
            lo = in[width] * 4 + in[0] * 2 + in[width - 1];
            lo = (lo + in[width + 1]) >> 3;
            if (lo < 0xAA) {
                lo--;
            }
            in++;
            if (lo < 0) {
                lo = 0;
            }
            *out++ = lo;
        }
        *out++ = 0;
        in++;
    }

    out += width;
    in += width;
    i = 5;
    while (i--) {
        *out++ = 0;
        in++;
        k = width - 2;
        while (k--) {
            j = (*(in - width) + *(in - 1 - width) + *(in + 1 - width)) >> 2;
            in++;
            if (j < 0xAA) {
                j--;
            }
            if (j < 0) {
                j = 0;
            }
            *out++ = j;
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
    k = height;
    while (k--) {
        dst = (u8 *)pixels;
        if (k & 1) {
            j = width;
            while (j--) {
                value = *in++;
                lo = value;
                hi = value + 0x34;
                if (value >> 4) {
                    lo *= value >> 4;
                    value = lo;
                }
                if (hi >= 0x100) {
                    hi = 0xFF;
                }
                if (lo >= 0x100) {
                    lo = 0xFF;
                }
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
                value = *in++;
                i = value;
                hi = value + 0x34;
                if (value >> 4) {
                    i *= value >> 4;
                    value = i;
                }
                if (hi >= 0x100) {
                    hi = 0xFF;
                }
                if (i >= 0x100) {
                    i = 0xFF;
                }
                *pixels = (hi << 8) | i;
            }
            pixels = (u16 *)(dst + (width << 1));
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o064/overlay64GenerateTexture/func_overlay_064_F0000000_18C3B28.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_064_F0000000_18C3B28:start
 * symbol: func_overlay_064_F0000000_18C3B28
 * score: 191/420 words
 * frame: 0x70
 * relocations: 20
 * first-mismatch: +0x9C
 * summary: first smoothing sum in two statements (191); noise-loop counter slot next
 * PLATEAU-HANDOFF:func_overlay_064_F0000000_18C3B28:end
 */
