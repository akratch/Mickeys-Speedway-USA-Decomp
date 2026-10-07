#include "PR/ultratypes.h"
#include "game/memory.h"

extern s32 D_8007BEA8;
extern s32 D_8007BE90;
extern s32 D_8007BE94;
extern s32 D_8007BE98;
extern s32 D_8007BEB8;
extern s32 D_8007BE80;
extern s32 D_8007BEAC;
extern s32 D_8007BE9C;
extern s32 D_8007BEA0;
extern s32 D_8007BEA4;
extern s32 D_8007BEE0;
extern s32 D_800D2FA8;
typedef struct FrontendBufferPointers {
    void *unk0;
    void *unk4;
    void *unk8;
} FrontendBufferPointers;
extern FrontendBufferPointers D_8007BE88;
typedef struct FrontendVertex {
    s16 x;
    s16 y;
    s16 unk4;
    s8 r;
    s8 g;
    s8 b;
    s8 a;
} FrontendVertex;
/* func_800371BC writes the same layout with unsigned colour bytes: the
 * target loads 255, not -1. The s8 view stays for the shading functions,
 * whose register naming depends on it. */
typedef struct FrontendGridVertex {
    s16 x;
    s16 y;
    s16 unk4;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} FrontendGridVertex;
extern s32 D_8007BE84;
extern s32 D_8007BEB0;
extern s32 D_8007BEB4;
extern f32 D_800826A0;
extern void func_800378A4(f32 arg0, s32 arg1);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);
extern void func_80037AEC(f32 arg0, s32 arg1);

typedef struct FrontendGfxWords {
    u32 w0;
    u32 w1;
} FrontendGfxWords;
typedef struct Gfx {
    FrontendGfxWords words;
} Gfx;
typedef struct Mtx Mtx;
typedef struct MainVertex MainVertex;

void func_80037150(void) {
    D_8007BEA8 = 0;
    if (D_8007BE88.unk0 != NULL) {
        mmFree(D_8007BE88.unk0);
        D_8007BE88.unk0 = NULL;
    }
    if (D_8007BE88.unk4 != NULL) {
        mmFree(D_8007BE88.unk4);
        D_8007BE88.unk4 = NULL;
    }
    D_8007BE80 = 0;
}
extern s32 viGetVideoMode(void);
extern void *func_8002B280(s32, s32);
/* Allocates and fills both backdrop vertex buffers with a 17x17 grid -- the
 * same grid func_800378A4 later shades. The m2c draft transcribed the
 * compiler's 4x unrolled inner loop literally; this TU has the default
 * unroller, so the ordinary counted loop reproduces that shape by itself.
 * Both divides are signed `/ 16`, not `>> 4`: the target carries the
 * rounding correction at every site.
 *
 * Matched 2026-09-23 (144 -> 114 -> 0). It takes no parameters (the target
 * homes nothing in the incoming slots; its call site in func_80037414 changed
 * with it). Then, in order: `y` is a plain s32, not an s16 cast (the cast was
 * the two extra words); the buffer loop is an index over D_8007BE88, not a
 * walking pointer -- strength reduction then makes the s6/s7 cursor and bound
 * itself, and the tail's two loads share one materialised base address, the
 * missing word (L154/L160); the store precedes the copy into `vertex`, which
 * is read back from the table, so the result is stored before it is copied;
 * the row loop is a `for`, which puts `row = 0` after the hoisted `-half - 1`;
 * and the colour bytes are written through the u8 FrontendGridVertex view
 * (the target loads 255, not -1).
 *
 * No donor counterpart: JFG's src/menu.c has no function of this shape.
 * frontend_37D50.c is Mickey's own backdrop renderer, outside the JFG menu.c
 * crosswalk that names the rest of this front end. */
void func_800371BC(void) {
    FrontendGridVertex *vertex;
    s32 spacing;
    s32 half;
    s32 row;
    s32 column;
    s32 y;
    s32 i;

    if (D_8007BE80 == 0) {
        if (viGetVideoMode() & 1) {
            spacing = 0x238;
        } else {
            spacing = 0x1AA;
        }
        for (i = 0; i < 2; i++) {
            ((FrontendGridVertex **) &D_8007BE88)[i] = func_8002B280(0xB4A, 0x87);
            vertex = ((FrontendGridVertex **) &D_8007BE88)[i];
            if (vertex != NULL) {
                half = spacing >> 1;
                for (row = 0; row != 0xFF0; row += 0xF0) {
                    y = 0x79 - (row / 16);
                    for (column = 0; column != 0x11; column++) {
                        vertex->x = (s16) (((column * spacing) / 16) - half - 1);
                        vertex->y = y;
                        vertex->unk4 = 0;
                        vertex->r = 0xFF;
                        vertex->g = 0xFF;
                        vertex->b = 0xFF;
                        vertex->a = 0xFF;
                        vertex++;
                    }
                }
            }
        }
        D_8007BE80 = 1;
        if ((D_8007BE88.unk0 == NULL) || (D_8007BE88.unk4 == NULL)) {
            func_80037150();
        }
    }
}
extern void TrapDanglingJump();
/* Matched 2026-09-23. Four edits took it from 57 masked words to 0:
 * the call to func_800371BC takes no arguments (the old +4 was the a2/a3
 * argument setup and the s0 carrier it forced); the three tail byte stores
 * are in address order; the first frame count is not a declared variable --
 * D_8007BE94 re-spells the expression and uopt CSEs it; and the
 * TrapDanglingJump argument is `var_a1 | 0`. uopt copy-propagates a bare
 * variable into a call argument but not into an operand of an operator, so
 * the plain `var_a1` argument was replaced by the expression temp and the two
 * webs traded a1/t0. The `| 0` keeps var_a1 itself as the argument, as the
 * target does, and folds away before code generation (`& -1` measures the
 * same). */
void func_80037414(s32 arg0, f32 arg1, f32 arg2, s32 arg3, s32 arg4,
                   s32 arg5, s32 arg6) {
    s32 var_a1;
    s32 sp28;
    s32 var_a2;

    var_a1 = (s32) (arg1 * 60.0f);
    var_a2 = (s32) (arg2 * 60.0f);
    sp28 = var_a2;
    if (D_8007BE80 == 0) {
        func_800371BC();
    }
    if ((D_8007BEA8 != 0) &&
        ((D_8007BE90 == 4) || (D_8007BE90 == 5))) {
        TrapDanglingJump(arg0, var_a1 | 0, var_a2);
    }
    if ((arg6 == 0) || (D_8007BEA8 == 0)) {
        if ((arg0 & 1) && (var_a2 != 0)) {
            D_8007BEA8 = 2;
        } else {
            D_8007BEA8 = 1;
        }
        D_8007BEAC = 0;
        D_8007BEB0 = 0;
        D_8007BEB4 = 0x8000;
    } else if (D_8007BEA8 == 2) {
        D_8007BEAC = (s32) (sp28 * D_8007BEAC) / D_8007BE98;
        D_8007BEB0 = 0;
    } else if (!(arg0 & 1)) {
        D_8007BEAC = (s32) (var_a1 * D_8007BEB0) / 1024;
    } else {
        D_8007BEAC = (s32) ((0x400 - D_8007BEB0) * var_a1) / 1024;
    }
    D_8007BE90 = arg0;
    D_8007BE94 = (s32) (arg1 * 60.0f);
    D_8007BE98 = var_a2;
    D_8007BE9C = (u8) arg3;
    D_8007BEA0 = (u8) arg4;
    D_8007BEA4 = (u8) arg5;
}
void func_80037658(void) {
    D_8007BEA8 = 0;
}
s32 func_80037664(void) {
    if ((D_8007BEA8 == 0) && (D_8007BEB8 == 0)) {
        return 0;
    }
    if ((D_8007BEA8 != 2) || ((D_8007BE90 & 1) != 0) ||
        (D_8007BEB8 != 0)) {
        return 1;
    }
    return 2;
}
/*
 * Fade state machine. The leftover time lives in the parameter: a separate
 * copy loses a2 to the cached mode bit and then has to save s0. `arg0 |= 0`
 * each iteration is the loop-weighted identity that keeps the remainder's
 * save above the mode bit's (L100); it emits no extra word. Duplicating the
 * loop-flag clear on both overflow arms, instead of a shared goto, is the
 * fallthrough the delay-slot copies need. old_state is register so it can
 * reuse ra after the save, which is dead before the jump.
 *
 * No donor: JFG PR 37 (head d45123d1c528955d5e12ddad805076267a690d76) does
 * not contain this function.
 */
void func_800376CC(s32 arg0) {
    s32 temp_t0;
    register s32 old_state;
    register s32 var_a1;

    temp_t0 = D_8007BE90 & 1;
    old_state = D_8007BEA8;
    do {
        var_a1 = 1;
        arg0 |= 0; /* L100: loop weight so the remainder keeps a2 */
        if (D_8007BEA8 == 2) {
            if (D_8007BE98 >= 0) {
                D_8007BEAC += arg0;
                if (D_8007BEAC >= D_8007BE98) {
                    arg0 = D_8007BEAC - D_8007BE98;
                    if (temp_t0 != 0) {
                        D_8007BEA8 = 1;
                    } else {
                        D_8007BEA8 = 0;
                    }
                    D_8007BEAC = 0;
                    var_a1 = 0;
                }
            }
        } else if (D_8007BEA8 == 1) {
            D_8007BEAC += arg0;
            if (temp_t0 == 0) {
                D_8007BEB0 = (s32) (D_8007BEAC << 0xA) / D_8007BE94;
            } else {
                D_8007BEB0 = (s32) ((D_8007BE94 - D_8007BEAC) << 0xA) /
                              D_8007BE94;
            }
            if (D_8007BEAC >= D_8007BE94) {
                arg0 = D_8007BEAC - D_8007BE94;
                if (temp_t0 != 0) {
                    D_8007BEA8 = 0;
                } else {
                    D_8007BEA8 = 2;
                }
                D_8007BEAC = 0;
                var_a1 = 0;
            }
        }
    } while (var_a1 == 0);
    if (D_8007BEB8 != 0) {
        D_8007BEB8 -= 1;
    }
    if ((old_state != 2) && (D_8007BEA8 == 2)) {
        D_8007BEB8 = 1;
    }
    if ((old_state != 0) && (D_8007BEA8 == 0) &&
        ((D_8007BEB8 = 1, (D_8007BE90 == 4)) ||
         (D_8007BE90 == 5))) {
        TrapDanglingJump(&D_8007BEAC, var_a1, arg0, &D_8007BEA8);
    }
}
/* The front-end backdrop's radial shading pass: a 17x17 vertex grid whose
 * greyscale falls off with distance from the centre, modulated by an angle
 * table (func_8002A8C0) and the caller's intensity.
 *
 * `amountI` is the untruncated amount captured before the parameter is scaled
 * in place, and `base` is deliberately computed inside the null guard rather
 * than beside it: the target sinks the `li 255` and the subtract past the
 * `beqz`, keeping only the `trunc.w.s`/`mfc1` in the prologue. Hoisting the
 * whole subtraction to the top -- the obvious spelling -- costs 46 words and
 * moves nothing else.
 *
 * No donor counterpart: JFG's src/menu.c has no function of this shape.
 * frontend_37D50.c is Mickey's own backdrop renderer, not part of the JFG
 * menu.c crosswalk that names the rest of this front end. */
void func_800378A4(f32 arg0, s32 intensity) {
    FrontendVertex *vertex;
    s32 base;
    s32 amountI;
    s32 x;
    s32 y;
    s32 height;
    s32 value;
    f32 dy;
    f32 dx;
    f32 distance;
    f32 scale;

    amountI = (s32) arg0;
    arg0 *= D_800826A0;
    vertex = ((FrontendVertex **) &D_8007BE88)[D_8007BE84];
    if (vertex != NULL) {
        base = 0xFF - amountI;
        for (y = 0; y != 0x11; y++) {
            dy = (f32) (y - 8) * 15.0f;
            for (x = 0; x != 0x11; x++) {
                dx = (f32) (x - 8) * 20.0f;
                distance = sqrtf((dx * dx) + (dy * dy));
                scale = (200.0f - distance) * arg0;
                if (scale < 0.0f) {
                    scale = 0.0f;
                }
                height = (s32) (func_8002A8C0((s32) (distance * 1000.0f) + D_8007BEB4) * scale);
                value = (s32) ((base + height) * intensity) >> 8;
                if (value < 0) {
                    value = 0;
                }
                if (value >= 0x100) {
                    value = 0xFF;
                }
                vertex->unk4 = (s16) (height + 5);
                vertex->r = (s8) value;
                vertex->g = (s8) value;
                vertex->b = (s8) value;
                vertex++;
            }
        }
    }
}
void func_80037A78(void) {
    D_8007BEB4 = 0x8000 - (D_8007BEB0 << 8);
    if (D_8007BEB0 < 0x200) {
        func_800378A4((f32) ((s32) D_8007BEB0 >> 3), 0x100);
        return;
    }
    func_800378A4(64.0f, (s32) (0x400 - D_8007BEB0) >> 1);
}
/* Workbench verdict: allocation-mismatch, 7 differing words. */
/* First mismatch: +0x54; target and candidate are 66 words with 0x40 frames. */
/* Structural gap: only register allocation remains. */
void func_80037AEC(f32 arg0, s32 arg1)
{
  FrontendVertex *vertex;
  s32 row;
  s32 phase;
  vertex = ((FrontendVertex **) (&D_8007BE88))[D_8007BE84];
  if (vertex != ((void *) 0))
  {
    s32 contrast = 0xFF - (((s32) arg0) * 2);
    phase = D_8007BEB4;
    if (1)
    {
      row = 0;
      do
      {
        s32 column = 0;
        s32 angle = phase;
        do
        {
          f32 value = func_8002A8C0(angle) * arg0;
          s32 integerValue;
          s32 intensity;
          column += 1;
          angle += 0x2000;
          vertex += 1;
          integerValue = (s32) value;
          vertex[-1].unk4 = (s16) (integerValue + 5);
          intensity = ((s32) (((integerValue * 2) + contrast) * arg1)) >> 8;
          vertex[-1].r = (s8) intensity;
          vertex[-1].g = (s8) intensity;
          vertex[-1].b = (s8) (intensity & 0xFFFFFFFFFFFFFFFF);
        }
        while (column != 0x11);
        row += 1;
        phase += 0x800;
      }
      while (row != 0x11);
    }
  }
}
void func_80037BF4(void) {
    D_8007BEB4 = (D_8007BEB0 << 8) + 0x8000;
    if (D_8007BEB0 < 0x200) {
        func_80037AEC((f32) D_8007BEB0 * 0.0625f, 0x100);
    } else {
        func_80037AEC(32.0f, (s32) (0x400 - D_8007BEB0) >> 1);
    }
}
extern u8 D_7BE40[];
extern s32 D_800D2FAC;
extern void camStandardPersp(Gfx **, Mtx **);
extern void func_80034920(Gfx **);
extern u8 D_8007BEC0[];
extern void viGetCurrentSize(s32 *, s32 *);
extern void func_80037A78(void);
extern void func_80037BF4(void);

#define FRONTEND_EMIT(pkt, opcode, data) \
    do { \
        Gfx *_cmd = *(pkt); \
        _cmd->words.w0 = (u32) (opcode); \
        _cmd->words.w1 = (u32) (data); \
        *(pkt) = _cmd + 1; \
    } while (0)

/* Draws the backdrop as a 16x8 grid of texture tiles, each loaded with the
 * six-packet tile load and drawn as two vertex rows and four triangles.
 * Matched 2026-10-07 (lane a-front) from 191 words by four edits:
 *  - the tile coordinates are induction variables, x += 40 per tile and
 *    y += 15 per row, not products of the row and column indices;
 *  - the triangle packet reads D_7BE40 through a block-scoped pointer local:
 *    written inline, uopt hoists the address out of the loop and spills it;
 *  - the vertex pointer is declared first (row and colour homes), and x is
 *    reset before the row colour is copied;
 *  - the vertex packet's two constant terms are one parenthesised group, so
 *    ugen spends no extra ring draw on a second immediate OR. */
#define _SHIFTL(v, s, w) ((u32) (((u32) (v) & ((0x01 << (w)) - 1)) << (s)))
#define FE_PKT(pkt, word0, word1) \
    { \
        Gfx *_g = (Gfx *) (pkt); \
        _g->words.w0 = (word0); \
        _g->words.w1 = (word1); \
    }
#define FE_LOADTILE(pkt, uls, ult, lrs, lrt) \
    FE_PKT(pkt, 0xF4000000 | _SHIFTL((uls) << 2, 12, 12) | _SHIFTL((ult) << 2, 0, 12), \
           _SHIFTL(7, 24, 3) | _SHIFTL((lrs) << 2, 12, 12) | _SHIFTL((lrt) << 2, 0, 12))
#define FE_SETTILE(pkt, line, word1) FE_PKT(pkt, 0xF5100000 | _SHIFTL(line, 9, 9), word1)
#define FE_TILESIZE(pkt, lrs, lrt) \
    FE_PKT(pkt, 0xF2000000, _SHIFTL((lrs) << 2, 12, 12) | _SHIFTL((lrt) << 2, 0, 12))
#define FE_VERTEX(pkt, v, n, v0) \
    FE_PKT(pkt, 0x04000000 | _SHIFTL(((n) << 3) | ((u32) (v) & 6), 16, 8) | (((v0) << 9) | ((n) * 10 + 8)), \
           (u32) (v))
#define OS_K0_TO_PHYSICAL(x) (u32) (((char *) (x) - 0x80000000))

void func_80037C74(Gfx **gfx, Mtx **mtx, MainVertex **vtx) {
    FrontendVertex *v;
    s32 row;
    s32 col;
    s32 x;
    s32 y;
    s32 uls;
    s32 ult;
    s32 lrs;
    s32 lrt;
    s32 colour;
    s32 rowColour;

    if (D_8007BE80 != 0) {
        camStandardPersp(gfx, mtx);
        FE_PKT((*gfx)++, 0xE7000000, 0);
        FE_PKT((*gfx)++, 0xED000000, 0x5003C0);
        FE_PKT((*gfx)++, 0xEF30000F, 0);
        FE_PKT((*gfx)++, 0xF7000000, 0x10001);
        FE_PKT((*gfx)++, 0xF64FC3BC, 0);
        FE_PKT((*gfx)++, 0xE7000000, 0);
        FE_PKT((*gfx)++, 0xB6000000, 0x10001);
        if (D_8007BE90 == 2 || D_8007BE90 == 3) {
            FE_PKT((*gfx)++, 0xFC357E04, 0x1F10F3FF);
            FE_PKT((*gfx)++, 0xEF182C0F, 0x0F0A4000);
        } else {
            FE_PKT((*gfx)++, 0xFC121824, 0xFF33FFFF);
            FE_PKT((*gfx)++, 0xEF082C0F, 0x0F0A4000);
        }
        FE_PKT((*gfx)++, 0xFD10013F, D_800D2FAC);
        colour = (D_8007BEB0 << 5) / 1024;
        y = 0;
        for (row = 0; row < 16; row++) {
            x = 0;
            rowColour = colour;
            for (col = 0; col < 16; col += 2) {
                if (x - 1 > 0) {
                    uls = x - 1;
                } else {
                    uls = 0;
                }
                if (y - 1 > 0) {
                    ult = y - 1;
                } else {
                    ult = 0;
                }
                if (x + 40 < 319) {
                    lrs = x + 40;
                } else {
                    lrs = 319;
                }
                if (y + 15 < 239) {
                    lrt = y + 15;
                } else {
                    lrt = 239;
                }

                FE_SETTILE((*gfx)++, ((((lrs - uls) + 1) * 2) + 7) >> 3, 0x07080200);
                FE_PKT((*gfx)++, 0xE6000000, 0);
                FE_LOADTILE((*gfx)++, uls, ult, lrs, lrt);
                FE_PKT((*gfx)++, 0xE7000000, 0);
                FE_SETTILE((*gfx)++, ((((lrs - uls) + 1) * 2) + 7) >> 3, 0x00080200);
                FE_TILESIZE((*gfx)++, (lrs - uls) - 1, lrt - ult - 1);
                if (D_8007BE90 == 2 || D_8007BE90 == 3) {
                    FE_PKT((*gfx)++, 0xFA000000, rowColour);
                    rowColour ^= ~0xFF;
                }
                v = &((FrontendVertex **) &D_8007BE88)[D_8007BE84][row * 17 + col];
                FE_VERTEX((*gfx)++, OS_K0_TO_PHYSICAL(v), 3, 0);
                v = &((FrontendVertex **) &D_8007BE88)[D_8007BE84][row * 17 + col + 17];
                FE_VERTEX((*gfx)++, OS_K0_TO_PHYSICAL(v), 3, 3);
                { u8 *_d = D_7BE40; FE_PKT((*gfx)++, 0x05310040, _d); }
                x += 40;
            }
            if (row & 1) {
                colour ^= ~0xFF;
            }
            y += 15;
        }
        func_80034920(gfx);
    }
}

/* func_80038190 matched 2026-10-07 (lane a-front) from 199 words: the
 * segment additions are K0-to-physical subtractions (no shared 0x80000000
 * register), var_a2 is declared first (frame homes), the tile loop is a for
 * with both zero inits in its header, and OR-with-zero probes (law L109;
 * uopt deletes them, globalcolor still counts them) on var_a2, var_a3 and
 * two packet cursors rank those webs above var_t2, which then takes t2 as
 * shipped. A source form carrying those reference counts is still unknown. */
/* Resident runtime records 245/246 bind these distinct typed call sites to
 * overlay 99's height-grid builder and framebuffer-grid renderer. Both retain
 * the static trap carrier until the runtime linker installs their callees. */
#pragma weak frontend38190BuildHeightGridReloc = TrapDanglingJump
#pragma weak frontend38190DrawHeightGridReloc = TrapDanglingJump
extern void frontend38190BuildHeightGridReloc(f32 scale, void *unused,
                                              s32 widthMinusOne,
                                              s32 heightMinusOne,
                                              s32 stepX, s32 stepY);
extern void frontend38190DrawHeightGridReloc(Gfx **displayList, Mtx **matrices,
                                             MainVertex **vertices, f32 scale,
                                             s32 columns, s32 rows,
                                             s32 triangles, s32 stepX,
                                             s32 stepY);

#define FRONTEND38190_EMIT(pkt, opcode, data) \
    { \
        Gfx *_cmd = (*(pkt))++; \
        _cmd->words.w0 = (u32) (opcode); \
        _cmd->words.w1 = (u32) (data); \
    }

#define FE38190_PROBE(pkt, opcode, data) \
    { \
        Gfx *_cmd = (*(pkt))++; \
        _cmd = (Gfx *) ((u32) _cmd | 0); \
        _cmd->words.w0 = (u32) (opcode); \
        _cmd->words.w1 = (u32) (data); \
    }
void func_80038190(Gfx **arg0, Mtx **arg1, MainVertex **arg2) {
    s32 var_a2;
    s32 spE0;
    s32 spDC;
    f32 spD8;
    u8 *viewport;
    s32 var_a3;
    s32 var_t2;

    if ((D_8007BE80 != 0) && (D_8007BEA8 != 0)) {
        FRONTEND38190_EMIT(arg0, 0xE7000000, 0);
        FRONTEND38190_EMIT(arg0, 0xED000000, 0x5003C0);
        if (D_8007BEA8 == 2) {
            FRONTEND38190_EMIT(arg0, 0xEF30000F, 0);
            FRONTEND38190_EMIT(arg0, 0xF7000000, 0x10001);
            FRONTEND38190_EMIT(arg0, 0xF64FC3BC, 0);
        } else {
            spD8 = (f32) D_8007BEAC / (f32) D_8007BE94;
            switch (D_8007BE90) {
            case 0:
            case 1:
                func_80037A78();
                break;
            case 2:
            case 3:
                func_80037BF4();
                break;
            case 4:
            case 5:
                frontend38190BuildHeightGridReloc(
                    spD8, (void *) (u32) D_8007BEB0,
                    0x10, 0x10, 0x14, 0xF);
                break;
            }
            D_8007BEE0 = (D_8007BEE0 + 1) & 1;
            viGetCurrentSize(&spE0, &spDC);
            viewport = D_8007BEC0 + (D_8007BEE0 * 0x10);
            *(s16 *) (viewport + 8) = (s16) (spE0 * 2);
            *(s16 *) (viewport + 0xA) = (s16) (spDC * 2);
            *(s16 *) (viewport + 0) = (s16) (spE0 * 2);
            *(s16 *) (viewport + 2) = (s16) (spE0 * 2);
            FRONTEND38190_EMIT(arg0, 0xE7000000, 0);
            FRONTEND38190_EMIT(arg0, 0xBC000406,
                          (u32) ((char *) D_800D2FAC - 0x80000000));
            FRONTEND38190_EMIT(arg0, 0xBC001006,
                          (u32) D_800D2FAC + 0x7FFFFB00u);
            FRONTEND38190_EMIT(arg0, 0xBC000806, 0x80000000u);
            FRONTEND38190_EMIT(arg0, 0xFF10013F, 0x01000000);
            FRONTEND38190_EMIT(arg0, 0xB6000000, 0x10001);
            FRONTEND38190_EMIT(arg0, 0xEF20000F, 0);
            for (var_a2 = 0, var_a3 = 0; var_a2 != 0xF0; var_a2 = var_t2) {
                var_t2 = var_a2 + 4;
                FRONTEND38190_EMIT(arg0, 0xFD100000,
                              (u32) D_800D2FA8 + var_a3);
                var_a3 += 0xA00;
                FRONTEND38190_EMIT(arg0, 0xF5100000, 0x07080200);
                FRONTEND38190_EMIT(arg0, 0xE6000000, 0);
                FE38190_PROBE(arg0, 0xF3000000, 0x074FF01A);
                FRONTEND38190_EMIT(arg0, 0xE7000000, 0);
                FRONTEND38190_EMIT(arg0, 0xF510A000, 0x80200);
                FRONTEND38190_EMIT(arg0, 0xF2000000, 0x4FC00C);
                FE38190_PROBE(arg0, 0xE4500000 | ((var_t2 * 4) & 0xFFF),
                              (var_a2 * 4) & 0xFFF);
                FRONTEND38190_EMIT(arg0, 0xB3000000, 0);
                FRONTEND38190_EMIT(arg0, 0xB2000000, 0x10000400);
                var_a2 = var_a2 | 0;
                var_a2 = var_a2 | 0;
                var_a3 = var_a3 | 0;
            }
            FRONTEND38190_EMIT(arg0, 0xE7000000, 0);
            FRONTEND38190_EMIT(arg0, 0xBC000406,
                          (u32) ((char *) D_800D2FA8 - 0x80000000));
            FRONTEND38190_EMIT(arg0, 0xBC001006,
                          (u32) D_800D2FA8 + 0x7FFFFB00u);
            FRONTEND38190_EMIT(arg0, 0xFF10013F, 0x01000000);
            FRONTEND38190_EMIT(arg0, 0x03800010, (u32) (viewport - 0x80000000));
            if ((D_8007BE90 == 4) || (D_8007BE90 == 5)) {
                frontend38190DrawHeightGridReloc(
                    arg0, arg1, arg2, spD8, 0x10, 0x10, 4, 0x28, 0xF);
            } else {
                func_80037C74(arg0, arg1, arg2);
            }
            D_8007BE84 ^= 1;
        }
        FRONTEND38190_EMIT(arg0, 0xE7000000, 0);
    }
}
#undef FRONTEND38190_EMIT
#undef FE38190_PROBE
#undef FRONTEND_EMIT
