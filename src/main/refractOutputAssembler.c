#include "PR/ultratypes.h"

/*
 * PROVENANCE: control flow follows Jet Force Gemini's published
 * src/hasm/refractOutputAssembler.s. Mickey's own symbols and the linked
 * ROM remain authoritative. The Jet Force Gemini object is the same
 * extent and is not byte-identical.
 */

extern s32 D_800D1C34;
extern s32 D_800D1C38;
extern u32 *D_800D1C28[];
extern u32 D_800D2DC4[];
extern u8 *D_800D2FA0;

#ifdef NON_MATCHING
void refractOutputAssembler(void) {
    u32 *buf;
    u32 *cursor;
    u32 *end;
    u32 *span;
    u32 *out;
    u32 *src;
    u32 *dst;
    u32 *tab;
    u8 *screen;
    u8 *mark;
    u8 *pix;
    u32 word;
    u32 hi;
    u32 lo;
    u32 addend;
    u32 skip;
    u32 gap;
    s32 x;
    s32 y;
    s32 words;
    s32 i;

    if (D_800D1C34 != 0) {
        D_800D1C34 -= 1;
        return;
    }

    buf = D_800D1C28[D_800D1C38];
    if (buf == NULL) {
        return;
    }

    end = (u32 *)((u8 *)buf + 0x25800);
    screen = D_800D2FA0;
    cursor = buf;
    span = buf;
    out = D_800D2DC4;

    while (cursor < end) {
        word = cursor[0];
        if (word == 0) {
            word = cursor[1];
            if (word != 0) {
                cursor += 1;
            }
        }
        if (word == 0) {
            word = cursor[2];
            if (word != 0) {
                cursor += 2;
            }
        }
        if (word == 0) {
            word = cursor[3];
            if (word != 0) {
                cursor += 3;
            }
        }
        if (word == 0) {
            word = cursor[4];
            if (word != 0) {
                cursor += 4;
            }
        }
        if (word == 0) {
            word = cursor[5];
            if (word != 0) {
                cursor += 5;
            }
        }
        if (word == 0) {
            word = cursor[6];
            if (word != 0) {
                cursor += 6;
            }
        }
        if (word == 0) {
            word = cursor[7];
            if (word != 0) {
                cursor += 7;
            }
        }
        if (word == 0) {
            cursor += 8;
            if (cursor < end) {
                continue;
            }
            if (span < end) {
                *out = (u32)((u8 *)cursor - (u8 *)span);
            }
            break;
        }

        screen += (u8 *)cursor - (u8 *)span;
        *out = (u32)((u8 *)cursor - (u8 *)span);
        out += 1;
        mark = screen;

        while (cursor < end) {
            word = *cursor;
            hi = word >> 16;
            lo = word & 0xFFFF;
            if (hi != 0) {
                y = (s32)((hi << 26) >> 27) - 16;
                x = (s32)(hi >> 11) - 16;
                addend = (hi & 0x7C0) + ((hi & 0x7C0) >> 5);
                pix = screen + (x << 1) + (y << 7) + (y << 9);
                *(u16 *)cursor = *(u16 *)pix + (u16)addend;
            }
            if (lo != 0) {
                y = (s32)((lo << 26) >> 27) - 16;
                x = (s32)(lo >> 11) - 16;
                addend = (lo & 0x7C0) + ((lo & 0x7C0) >> 5);
                pix = screen + (x << 1) + (y << 7) + (y << 9) + 2;
                *(u16 *)((u8 *)cursor + 2) = *(u16 *)pix + (u16)addend;
            }
            cursor += 1;
            if (cursor >= end) {
                *out = (u32)((u8 *)cursor - (u8 *)span);
                out += 1;
                break;
            }
            screen += 4;
            if (*cursor == 0) {
                break;
            }
        }

        span = cursor;
        cursor += 1;
        *out = (u32)(screen - mark);
        out += 1;
        if (cursor >= end) {
            break;
        }
    }

    skip = D_800D2DC4[0];
    src = (u32 *)((u8 *)buf + skip);
    if (src >= end) {
        return;
    }
    dst = (u32 *)(D_800D2FA0 + skip);
    tab = D_800D2DC4 + 1;
    while (src < end) {
        words = (s32)*tab;
        tab += 1;
        if (words < 0) {
            words += 3;
        }
        words >>= 2;
        while (words >= 8) {
            for (i = 0; i < 8; i++) {
                dst[i] = src[i];
            }
            src += 8;
            dst += 8;
            words -= 8;
        }
        while (words > 0) {
            *dst = *src;
            dst += 1;
            src += 1;
            words -= 1;
        }
        gap = *tab;
        tab += 1;
        src = (u32 *)((u8 *)src + gap);
        dst = (u32 *)((u8 *)dst + gap);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/refractOutputAssembler/refractOutputAssembler.s")
#endif

/* PLATEAU-HANDOFF:refractOutputAssembler:start
 * symbol: refractOutputAssembler
 * score: 248 differing words
 * frame: 0x10
 * relocations: 12
 * first-mismatch: +0x0
 * summary: hypothesis=16-word gap is scan/decode/copy scheduling; spellings=loops -260, unrolled scan -140, copy unroll -140; stall=size stays -140, frame 0x10 vs 0x48
 * PLATEAU-HANDOFF:refractOutputAssembler:end
 */
