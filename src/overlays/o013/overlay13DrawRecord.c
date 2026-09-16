#include "PR/ultratypes.h"

typedef struct O13Record {
    u8 pad00[6];
    u8 state;
    u8 timer;
    f32 scale;
    f32 x;
    f32 z;
    f32 y;
    f32 floorZ;
    f32 velocityX;
    f32 velocityZ;
    f32 velocityY;
    f32 phase;
    s32 vertexBank;
    u8 vertices[0x50];
} O13Record;

typedef struct O13Command { u32 w0, w1; } O13Command;
typedef struct O13RenderState { u8 pad00[0xF7]; u8 r, g, b; } O13RenderState;

extern void *D_20;
extern u8 *D_24;
extern void *D_28;
extern f32 D_4;
extern u8 D_80000000[];

extern O13RenderState *o13GetRenderState(void);
extern void o13SetColor(u8, u8, u8, u8, u8, u8);
extern void o13DrawRecord(O13Command **, s32, s32, O13Record *, void *, s32, s32);
extern void o13FinishDraw(void);
extern void o13SetupRecord(O13Command **, void *, s32, s32);

/* Each command is one gbi-style block, `{ Gfx *_g = pkt; _g->w0 = ..; _g->w1 = ..; }`
 * (PROVENANCE: the shape of DKR's `fast3d_cmd`/`gDma1p` macros in
 * include/f3ddkr.h and gbi.h; the words are this ROM's own). The block-scoped
 * cursor is load-bearing: a shared `cmd` local is a phantom web that pushes the
 * render pointer off t1 and rotates the ring; the vertex word must be built
 * `(0x04 << 24 | p << 16) | 0x30` in gDma1p's order, and the colour bytes need
 * the `& 0xFF` of _SHIFTL so uopt evaluates r before g. Matched 2026-09-16
 * (lane s1-a): 97 to 39 on the block cursor and OR order, 39 to 0 on the masks. */
#define O13_GFX(w0v, w1v) { O13Command *_g = (*commands)++; _g->w0 = (w0v); _g->w1 = (w1v); }

void overlay13DrawRecord(
    O13Record *record, O13Command **commands, s32 arg2, s32 arg3) {
    f32 savedScale;
    O13RenderState *render;

    if (((record->state == 1) && (D_20 != 0)) || record->state == 2) {
        render = o13GetRenderState();
        savedScale = record->scale;
        if (record->state != 2) {
            o13SetColor(0xFF, 0xFF, 0xFF, render->r, render->g, render->b);
            record->scale = savedScale;
            o13DrawRecord(commands, arg2, arg3, record, D_20, 6, 0xFF);
            o13FinishDraw();
        } else {
            o13SetupRecord(commands, D_28, 0xE, 0);
            if (record->timer < 0x20) {
                O13_GFX(0xFA000000, ((record->timer * 8) & 0xFF) | 0xFFFFFF00);
            }

            O13_GFX(0x04000000 |
                        ((((((u32)record + record->vertexBank * 0x28 +
                             0x80000030U) & 6) | 0x20) & 0xFF) << 16) | 0x30,
                    (u32)record + record->vertexBank * 0x28 + 0x80000030U);
            O13_GFX(0x05110020, (u32)&D_80000000);

            if (D_24 != 0 && record->phase < (f32)(u32)*D_24) {
                record->scale = savedScale * D_4;
                O13_GFX(0xFA000000, ((render->r & 0xFF) << 24) |
                                        ((render->g & 0xFF) << 16) |
                                        ((render->b & 0xFF) << 8) | 0xA0);
                O13_GFX(0xFB000000, 0xFFFFFF00);
                o13DrawRecord(commands, arg2, arg3, record, D_24, 0xE, 0);
            }
        }

        record->scale = savedScale;
        O13_GFX(0xE7000000, 0);
        O13_GFX(0xFB000000, 0xFFFFFFFF);
        O13_GFX(0xFA000000, 0xFFFFFFFF);
    }
}
