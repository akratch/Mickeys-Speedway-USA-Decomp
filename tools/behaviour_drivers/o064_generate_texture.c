/*
 * Behaviour driver for src/overlays/o064/overlay64GenerateTexture.c
 * (func_overlay_064_F0000000_18C3B28), used by tools/behaviour_check.py.
 *
 * Synthetic inputs only: source/destination buffers in five patterns, a byte image
 * with the TU's header layout (u16 width at +6, u16 height at +8, pixels at
 * +0x20) placed with room on both sides for the even rows' -4 byte offset,
 * nine image sizes, both initialised states and both buffer selects.
 * mathRnd is a deterministic LCG. The digest is an FNV-1a hash of every byte
 * the function can write: the image block, both buffer sets and the globals.
 */
#include <stdio.h>
#include <string.h>

typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;

u8 gO64BufferSelect;
s32 gO64Initialized;
void *D_800D60B0[4];
void *D_800D60C0[4];

static unsigned seed;

s32 mathRnd(s32 minimum, s32 maximum)
{
    seed = seed * 1103515245u + 12345u;
    return minimum + (s32)((seed >> 8) % (unsigned)(maximum - minimum + 1));
}

void func_overlay_064_F0000000_18C3B28(s32 index, void *image, u8 *unused);

static unsigned digest;

/*
 * Buffer contents by pattern: a ramp, all zeros, near zero (so the smoothing
 * sums reach 0 and every clamp is taken), near 0xFF, and LCG noise. A
 * pattern set that never makes a sum zero lets a cell that drops the
 * `< 0` clamp pass, which happened once (2026-10-15).
 */
static u8 fill(int k, int salt, int pattern)
{
    unsigned x = (unsigned)(k * 2654435761u + salt * 40503u);

    switch (pattern % 5) {
        case 0:
            return (u8)(k * 7 + salt);
        case 1:
            return 0;
        case 2:
            return (u8)((x >> 13) % 3);
        case 3:
            return (u8)(0xFF - (x >> 11) % 4);
        default:
            return (u8)(x >> 16);
    }
}

static void mix(const void *p, size_t n)
{
    const u8 *q = p;
    while (n--) {
        digest = digest * 16777619u ^ *q++;
    }
}

int main(void)
{
    static u8 bufA[4][8192], bufB[4][8192], block[20000];
    static const int dims[][2] = {
        {16, 12}, {17, 13}, {20, 20}, {9, 7}, {32, 24}, {10, 11}, {6, 7}, {7, 6}, {8, 8},
    };
    int t, r, i, k;

    for (i = 0; i < 4; i++) {
        D_800D60B0[i] = bufA[i];
        D_800D60C0[i] = bufB[i];
    }
    digest = 2166136261u;
    for (t = 0; t < 9; t++) {
        for (r = 0; r < 3; r++) {
            u8 *im = block + 4096;
            seed = t * 100 + r;
            for (i = 0; i < 4; i++) {
                for (k = 0; k < 8192; k++) {
                    bufA[i][k] = fill(k, i + r, t + r);
                    bufB[i][k] = fill(k, i + 4, t + r + 1);
                }
            }
            memset(block, 0x5A, sizeof block);
            *(u16 *)(im + 6) = dims[t][0];
            *(u16 *)(im + 8) = dims[t][1];
            gO64Initialized = (r > 0);
            gO64BufferSelect = r & 1;
            func_overlay_064_F0000000_18C3B28(t & 3, im, 0);
            mix(block, sizeof block);
            mix(bufA, sizeof bufA);
            mix(bufB, sizeof bufB);
            mix(&gO64BufferSelect, 1);
            mix(&gO64Initialized, sizeof gO64Initialized);
        }
    }
    printf("%08x\n", digest);
    return 0;
}
