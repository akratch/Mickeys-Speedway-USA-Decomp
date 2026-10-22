#include "PR/ultratypes.h"
typedef struct G { u32 w0, w1; } G;
typedef struct V { s16 x,y,z; u8 r,g,b,a; } V;
typedef struct Pair { V first, second; } Pair;
typedef struct Strip { s16 count; u8 buffer,pad03; void *material; u8 pad08[0x24]; Pair *buffers[2]; void *triangles; } Strip;
extern void texDPTextureX(G **, void *, s32, s32);
#define S(v,s,w) (((u32)(v)&((1U<<(w))-1U))<<(s))
#define PRIM(p) { G *m=(G *)(p); m->w0=0xFA000000; m->w1=0xFFFFFFFF; }
#define VTX(p,a,n) { G *m=(G *)(p); m->w0=S(4,24,8)|S(((n)<<3)|((u32)(a)&6),16,8)|S(((n)<<3)+((n)<<1)+8,0,16); m->w1=(u32)(a); }
#define STRIP(p,a,n,t) { G *m=(G *)(p); m->w0=S(5,24,8)|S((((n)-1)<<4)|(t),16,8)|S((n)<<4,0,16); m=(G *)((u32)m | 0); m->w1=(u32)(a); }
#define SYNC(p) { G *m=(G *)(p); m->w1=0; m->w0=0xE7000000; }

/* Tier A: 119 stock-IDO words match the linked retail range.
 * STRIP deliberately reassigns its initialized cursor through unsigned OR zero.
 * On this 32-bit ABI that preserves every pointer bit and adds no memory access.
 * It gives the cursor two definitions and five references, lowering its measured
 * allocation save from 30 to 25; IDO then removes the operation. A plain copy
 * gives the wrong allocation. See docs/cleanup-queue.md for naturalization. */
void overlay17DrawStrip(G **commands, Strip *strip) {
    Pair *pair;
    Pair *previous;
    Pair *start;
    s32 textured;
    s32 vertices;
    s32 flush;
    u32 remaining;
    u32 segment;
    if (strip == 0) return;
    segment=0x80000000U;
    textured = strip->material ? 1 : 0;
    textured |= (texDPTextureX(commands, strip->material, 0x1F, 0), 0);
    PRIM((*commands)++);
    vertices=strip->count;
    remaining=vertices;
    previous=0;
    pair=strip->buffers[strip->buffer];
    start=0; vertices=0; flush=0;
    /* Post-decrement, not a separate test-then-decrement: cfe reuses ONE temp
     * symbol for this entry test and for the do-while's own `remaining--`, so
     * the two share a single web instead of two. That one merge moves the
     * prologue's whole colour assignment onto the target's and empties the
     * structural residual: 14 -> 3 masked words, zero size delta. */
    if (remaining--) {
        do {
            vertices += 2;
            if (remaining == 0) {
                flush=1;
                goto check_flush;
            }
            if (pair->first.a == 0) {
                flush=1;
                goto check_flush;
            }
            if (start == 0) {
                if (previous != 0) { start=previous; vertices=4; }
                else { start=pair; vertices=2; }
            }
            if (vertices >= 17) flush=1;
check_flush:
            if (flush) {
                flush=0;
                if (start != 0) {
                    VTX((*commands)++, (void *)((u32)start+segment), vertices);
                    STRIP((*commands)++, (void *)((u32)strip->triangles+segment), vertices-2,
                          textured);
                    start=0;
                }
                vertices=0;
            }
            previous=pair++;
        } while (remaining--);
    }
    pair = (Pair *)(*commands)++;
    ((G *)pair)->w1 = 0;
    ((G *)pair)->w0 = 0xE7000000;
}
