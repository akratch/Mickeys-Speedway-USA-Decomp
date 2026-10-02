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
extern u8 *gO64BuffersA[];
extern u8 *gO64BuffersB[];
extern s32 o64RandomRange(s32 minimum, s32 maximum);

/*
 * Rewritten from the listing (lane w2-ovld, 2026-10-02): every loop is
 * `while (n--)` (the target's dead copy of the old counter after each test),
 * one counter for the outer and single loops and a second for the inner ones,
 * a u8 local for the buffer select, innerWidth as a variable so the counter is
 * built from it, and the clamp limit written back into `distance` so it is
 * computed before the call and saved in that local's own cell. 382 masked at
 * delta 0 against 401. Open: the frame is 0x78 against 0x70 (five compiler
 * temporaries against four), and the target keeps width in ra and the
 * counter in t5.
 */
#ifdef NON_MATCHING
void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image, u8 *unused)
{
    s32 i;
    u8 select;
    s32 width;
    s32 height;
    s32 innerWidth;
    s32 innerHeight;
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

    select = gO64BufferSelect;
    width = image->width;
    height = image->height;
    if (select) {
        src = gO64BuffersA[index];
    } else {
        src = gO64BuffersB[index];
    }

    innerWidth = width - 6;
    innerHeight = height - 6;
    out = src + innerHeight * width + 3;
    if (!gO64Initialized) {
        i = innerWidth;
        while (i--) {
            distance = (width >> 1) - i - 3;
            if (distance <= 0) {
                distance = -distance;
            }
            *out++ = o64RandomRange(0, 0xFF - (distance << 4));
        }
        gO64Initialized = 1;
    } else {
        i = innerWidth;
        while (i--) {
            distance = (width >> 1) - i - 3;
            if (distance <= 0) {
                distance = -distance;
            }
            distance = 0xFF - (distance << 4);
            value = o64RandomRange(-0x60, 0x60) + *out;
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
        dst = gO64BuffersA[index];
    } else {
        dst = gO64BuffersB[index];
    }

    in = src;
    out = dst;
    i = innerHeight;
    while (i--) {
        *out++ = 0;
        in++;
        j = width - 2;
        while (j--) {
            value = (in[width] * 4 + in[0] * 2 + in[width - 1] + in[width + 1]) >> 3;
            in++;
            if (value < 0xAA) {
                value--;
            }
            if (value < 0) {
                value = 0;
            }
            *out++ = value;
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
        j = width - 2;
        while (j--) {
            value = (in[-width] + in[-width - 1] + in[-width + 1]) >> 2;
            in++;
            if (value < 0xAA) {
                value--;
            }
            if (value < 0) {
                value = 0;
            }
            *out++ = value;
        }
        *out++ = 0;
        in++;
    }

    out = dst + innerHeight * width;
    in = out - width;
    i = width;
    while (i--) {
        *out++ = *in++;
    }

    in = dst;
    pixels = image->pixels;
    i = height;
    while (i--) {
        if (i & 1) {
            j = width;
            while (j--) {
                value = *in++;
                lo = value;
                hi = value + 0x34;
                if (value >> 4) {
                    lo = value * (value >> 4);
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
                value = *in++;
                lo = value;
                hi = value + 0x34;
                if (value >> 4) {
                    lo = value * (value >> 4);
                }
                if (hi >= 0x100) {
                    hi = 0xFF;
                }
                if (lo >= 0x100) {
                    lo = 0xFF;
                }
                pixels[distance + j] = (hi << 8) | lo;
            }
            pixels += width;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o064/overlay64GenerateTexture/func_overlay_064_F0000000_18C3B28.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_064_F0000000_18C3B28:start
 * symbol: func_overlay_064_F0000000_18C3B28
 * score: 382/420 words
 * frame: 0x78
 * relocations: 18
 * first-mismatch: +0x0
 * summary: Listing rewrite with while (n--) loops: 401 to 382 at delta 0. Open: frame 0x78 vs 0x70 (five temporaries vs four), width in ra, counter in t5.
 * PLATEAU-HANDOFF:func_overlay_064_F0000000_18C3B28:end
 */
