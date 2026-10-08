/*
 * Resident gzip front end -- ROM 0x4E1E0-0x4EA60.
 *
 * DKR's published src/gzip.c identifies this source unit and supplies the
 * byteswap32 and Huffman-table-builder bodies below. Mickey's bytes remain
 * decisive: the remaining function retains generated assembly.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "game/pi.h"
#include "PRinternal/piint.h"

extern s32 D_8007D630;
extern u8 *D_8007D634;
extern u8 *D_8007D638;
extern u8 D_800D6A90[];
extern s32 D_800D6AA0;
extern s32 D_800D6AA4;
extern void *func_8002B280(s32 size, s32 tag);
extern s32 gzip_inflate_block(void);
extern OSThread D_800D6740;
extern OSMesgQueue D_800D6A70;
extern OSMesg D_800D6A88;
extern void __osDevMgrMain(void *);
extern s32 D_800D6AA8;

typedef struct MickeyHuft MickeyHuft;
struct MickeyHuft {
    u8 e;
    u8 b;
    union {
        u16 n;
        MickeyHuft *t;
    } v;
};

#define BMAX 16
#define N_MAX 288

/*
 * PROVENANCE: adapted from the published Diddy Kong Racing libultra
 * `osCreatePiManager` source. Mickey's fixed BSS objects and instruction
 * stream remain authoritative; this body is compiled and independently
 * compared against Mickey's function.
 */
void func_8004D5E0(OSPri priority, OSMesgQueue *queue, OSMesg *messages,
                   s32 count) {
    u32 savedMask;
    OSPri oldPri;
    OSPri myPri;

    if (__osPiDevMgr.active) {
        return;
    }
    osCreateMesgQueue(queue, messages, count);
    osCreateMesgQueue(&D_800D6A70, &D_800D6A88, 1);
    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }
    osSetEventMesg(8, &D_800D6A70, (OSMesg)0x22222222);
    oldPri = -1;
    myPri = osGetThreadPri(NULL);
    if (myPri < priority) {
        oldPri = myPri;
        osSetThreadPri(NULL, priority);
    }
    savedMask = __osDisableInt();
    __osPiDevMgr.active = 1;
    __osPiDevMgr.thread = &D_800D6740;
    __osPiDevMgr.cmdQueue = queue;
    __osPiDevMgr.evtQueue = &D_800D6A70;
    __osPiDevMgr.acsQueue = &__osPiAccessQueue;
    __osPiDevMgr.dma = __osPiRawStartDma;
    __osPiDevMgr.edma = __osEPiRawStartDma;
    osCreateThread(&D_800D6740, 0, __osDevMgrMain, &__osPiDevMgr,
                   &D_800D6A70, priority);
    osStartThread(&D_800D6740);
    __osRestoreInt(savedMask);
    if (oldPri != -1) {
        osSetThreadPri(NULL, oldPri);
    }
}

void func_8004D750(void) {
    D_8007D630 = (s32) func_8002B280(0x2800, 0x8F);
}

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published src/gzip.c.
 * Mickey's compiled and linked function is independently byte-identical.
 */
s32 byteswap32(u8 *bytes) {
    s32 value;

    value = *bytes++;
    value |= *bytes++ << 8;
    value |= *bytes++ << 16;
    value |= *bytes << 24;
    return value;
}

s32 func_8004D7A8(s32 assetIndex, s32 assetOffset) {
    piRomLoadSection(assetIndex, (u32) D_800D6A90, assetOffset, 8);
    return byteswap32(D_800D6A90);
}

u8 *func_8004D7E0(u8 *compressed, u8 *output) {
    D_8007D634 = compressed + 5;
    D_8007D638 = output;
    D_800D6AA4 = 0;
    D_800D6AA0 = 0;
    if (gzip_inflate_block() != 0) {
        do {
        } while (gzip_inflate_block() != 0);
    }
    return output;
}

/* PROVENANCE: adapted from Diddy Kong Racing's published src/gzip.c,
 * gzip_huft_build. Mickey's globals, ABI, and instruction comparison remain
 * authoritative; the donor body is only the clean-room starting point. */
/* Spelling the table address as index-plus-base selects retail's temporary
 * evaluation order. All 390 instructions, the frame, and relocations match. */
void func_8004D840(u32 *b, u32 n, u32 s, u16 *d, u16 *e,
                   MickeyHuft **t, s32 *m) {
    u32 a;
    u32 c[BMAX + 1];
    u32 f;
    s32 g;
    s32 h;
    register u32 i;
    register u32 j;
    register s32 k;
    s32 l;
    register u32 *p;
    register MickeyHuft *q;
    MickeyHuft r;
    MickeyHuft *u[BMAX];
    u32 v[N_MAX];
    register s32 w;
    u32 x[BMAX + 1];
    u32 *xp;
    s32 y;
    u32 z;

    _bzero(c, sizeof(c));
    p = b;
    i = n;
    do {
        c[*p]++;
        p++;
    } while (--i);
    if (c[0] == n) {
        *t = NULL;
        *m = 0;
        return;
    }

    l = *m;
    for (j = 1; j <= BMAX; j++) {
        if (c[j]) {
            break;
        }
    }
    k = j;
    if ((u32) l < j) {
        l = j;
    }
    for (i = BMAX; i; i--) {
        if (c[i]) {
            break;
        }
    }
    g = i;
    if ((u32) l > i) {
        l = i;
    }
    *m = l;

    y = 1 << j;
    while (j < i) {
        y -= c[j];
        j++;
        y <<= 1;
    }
    y -= c[i];
    c[i] += y;

    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i) {
        *xp++ = (j += *p++);
    }

    p = b;
    i = 0;
    do {
        if ((j = *p++) != 0) {
            v[x[j]++] = i;
        }
    } while (++i < n);

    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = NULL;
    q = NULL;
    z = 0;

    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            while (k > w + l) {
                h++;
                w += l;
                z = (z = g - w) > (u32) l ? (u32) l : z;
                if ((f = 1 << (j = k - w)) > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    while (++j < z) {
                        if ((f <<= 1) <= *++xp) {
                            break;
                        }
                        f -= *xp;
                    }
                }
                z = 1 << j;

                q = D_800D6AA8 + (MickeyHuft *) D_8007D630;
                D_800D6AA8 += z + 1;

                *t = q + 1;
                *(t = &q->v.t) = NULL;
                u[h] = ++q;

                if (h) {
                    x[h] = i;
                    r.b = l;
                    r.e = 16 + j;
                    r.v.t = q;
                    j = i >> (w - l);
                    u[h - 1][j] = r;
                }
            }

            r.b = k - w;
            if (p >= v + n) {
                r.e = 99;
            } else if (*p < s) {
                r.e = *p < 256 ? 16 : 15;
                r.v.n = *p;
                p++;
            } else {
                r.e = ((u8 *) e)[*p - s];
                r.v.n = d[*p++ - s];
            }

            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f) {
                q[j] = r;
            }

            for (j = 1 << (k - 1); i & j; j >>= 1) {
                i ^= j;
            }
            i ^= j;

            while ((i & ((1 << w) - 1)) != x[h]) {
                h--;
                w -= l;
            }
        }
    }
}
