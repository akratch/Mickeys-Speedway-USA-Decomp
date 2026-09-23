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
/* Workbench verdict: size-mismatch, 114 differing words (was 144), size
 * delta +4, first mismatch +0x2C.
 *
 * Allocates and fills both backdrop vertex buffers with a 17x17 grid -- the
 * same grid func_800378A4 later shades. The m2c draft transcribed the
 * compiler's 4x unrolled inner loop literally (a peeled first vertex, then
 * four vertices and four parallel `var_s2 * 4` accumulators per pass); this
 * TU has the default unroller, so the ordinary counted loop reproduces that
 * shape by itself. Both divides are signed `/ 16`, not `>> 4`: the target
 * carries the `bgez` / `addiu at,x,15` rounding correction at every site, so
 * the source could not prove the dividend non-negative.
 *
 * func_800371BC takes NO PARAMETERS: the target homes nothing in the
 * incoming argument slots. Applied 2026-09-23 together with its one call site
 * in func_80037414, which it also moved to size delta 0: 144 -> 114 here.
 *
 * No donor counterpart: JFG's src/menu.c has no function of this shape.
 * frontend_37D50.c is Mickey's own backdrop renderer, outside the JFG menu.c
 * crosswalk that names the rest of this front end. */
#ifdef NON_MATCHING
void func_800371BC(void) {
    FrontendVertex *vertex;
    void **buffer;
    s32 spacing;
    s32 half;
    s32 row;
    s32 column;
    s16 y;

    if (D_8007BE80 == 0) {
        if (viGetVideoMode() & 1) {
            spacing = 0x238;
        } else {
            spacing = 0x1AA;
        }
        buffer = (void **) &D_8007BE88;
        do {
            vertex = func_8002B280(0xB4A, 0x87);
            *buffer = vertex;
            if (vertex != NULL) {
                half = spacing >> 1;
                row = 0;
                do {
                    y = (s16) (0x79 - (row / 16));
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
                    row += 0xF0;
                } while (row != 0xFF0);
            }
            buffer++;
        } while (buffer != (void **) &D_8007BE90);
        D_8007BE80 = 1;
        if ((D_8007BE88.unk0 == NULL) || (D_8007BE88.unk4 == NULL)) {
            func_80037150();
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_800371BC.s")
#endif
extern void TrapDanglingJump();
#ifdef NON_MATCHING
/* 57 -> 13 masked words, size delta +4 -> 0, frame 0x38 -> 0x30 (2026-09-23).
 * Three edits: the call to func_800371BC takes no arguments (the +4 was the
 * a2/a3 argument setup and the s0 carrier it forced); the three tail byte
 * stores are in address order; and the first frame count is not a declared
 * variable -- D_8007BE94 re-spells the expression and uopt CSEs it, which
 * leaves exactly three declared homes (var_a1 0x2C, sp28 0x28, var_a2 0x24)
 * and two compiler temps, the target's slot ladder word for word.
 * What remains is one decision: the target passes the var_a1 web to the
 * TrapDanglingJump call and keeps the expression temp for D_8007BE94 alone,
 * while uopt here propagates the expression into the call argument, so the
 * two webs trade a1/t0 and their spill slots (5 naming, 8 immediate). */
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
        TrapDanglingJump(arg0, var_a1, var_a2);
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80037414.s")
#endif
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

/* Workbench verdict: structure-mismatch, 314 differing words; target 327/candidate 311 words. */
/* First mismatch: +0x0; both frames now 0xE0. */
/* Structural gap: uopt hoists this loop's opcode and address constants into
 * callee-saved registers where the ROM re-materialises them inside the loop. */
#ifdef NON_MATCHING
void func_80037C74(Gfx **arg0, Mtx **arg1, MainVertex **arg2) {
    s32 spD8;
    s32 spB8;
    s32 sp40;
    s32 temp_s3;
    s32 temp_t0;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a2;
    s32 var_ra;
    s32 var_s0;
    s32 var_s1;
    s32 var_t2;
    s32 var_t3;
    s32 var_t4;
    s32 var_v1;

    if (D_8007BE80 != 0) {
        camStandardPersp(arg0, arg1);
        FRONTEND_EMIT(arg0, 0xE7000000, 0);
        FRONTEND_EMIT(arg0, 0xED000000, 0x5003C0);
        FRONTEND_EMIT(arg0, 0xEF30000F, 0);
        FRONTEND_EMIT(arg0, 0xF7000000, 0x10001);
        FRONTEND_EMIT(arg0, 0xF64FC3BC, 0);
        FRONTEND_EMIT(arg0, 0xE7000000, 0);
        FRONTEND_EMIT(arg0, 0xB6000000, 0x10001);
        var_a0 = 0;
        if ((D_8007BE90 == 2) || (D_8007BE90 == 3)) {
            FRONTEND_EMIT(arg0, 0xFC357E04, 0x1F10F3FF);
            FRONTEND_EMIT(arg0, 0xEF182C0F, 0x0F0A4000);
        } else {
            FRONTEND_EMIT(arg0, 0xFC121824, 0xFF33FFFF);
            FRONTEND_EMIT(arg0, 0xEF082C0F, 0x0F0A4000);
        }
        FRONTEND_EMIT(arg0, 0xFD10013F, D_800D2FAC);
        sp40 = 0;
        spD8 = 0;
        spB8 = (D_8007BEB0 << 5) / 1024;
        do {
            var_v1 = 0;
            var_s0 = spB8;
            var_ra = sp40 * 0xA;
            var_s1 = 0;
            temp_s3 = var_a0 + 0xF;
loop_7:
            if (var_v1 - 1 > 0) {
                var_t3 = var_v1 - 1;
            } else {
                var_t3 = 0;
            }
            if (var_a0 - 1 > 0) {
                var_t4 = var_a0 - 1;
            } else {
                var_t4 = 0;
            }
            if (var_v1 + 0x28 < 0x13F) {
                var_t2 = var_v1 + 0x28;
            } else {
                var_t2 = 0x13F;
            }
            temp_t0 = var_t2 - var_t3;
            if (temp_s3 < 0xEF) {
                var_a2 = temp_s3;
            } else {
                var_a2 = 0xEF;
            }
            FRONTEND_EMIT(arg0, (((((temp_t0 * 2) + 9) >> 3) & 0x1FF) << 9) | 0xF5100000, 0x07080200);
            FRONTEND_EMIT(arg0, 0xE6000000, 0);
            FRONTEND_EMIT(arg0,
                          (((var_t3 * 4) & 0xFFF) << 12) |
                              0xF4000000 | ((var_t4 * 4) & 0xFFF),
                          (((var_t2 * 4) & 0xFFF) << 12) |
                              0x07000000 | ((var_a2 * 4) & 0xFFF));
            FRONTEND_EMIT(arg0, 0xE7000000, 0);
            FRONTEND_EMIT(arg0, (((((temp_t0 * 2) + 9) >> 3) & 0x1FF) << 9) | 0xF5100000, 0x00080200);
            FRONTEND_EMIT(arg0, 0xF2000000,
                          ((((temp_t0 - 1) * 4) & 0xFFF) << 12) |
                              (((var_a2 - var_t4 - 1) * 4) & 0xFFF));
            if ((D_8007BE90 == 2) || (D_8007BE90 == 3)) {
                FRONTEND_EMIT(arg0, 0xFA000000, var_s0);
                var_s0 ^= -0x100;
            }
            var_s1 += 2;
            temp_v1 = ((Gfx **) &D_8007BE88)[D_8007BE84];
            temp_v1 += var_ra + 0x80000000;
            FRONTEND_EMIT(arg0,
                          (((((temp_v1 & 6) | 0x18) & 0xFF) << 16) |
                              0x04000026),
                          temp_v1);
            temp_v1_2 = ((Gfx **) &D_8007BE88)[D_8007BE84];
            temp_v1_2 += var_ra + 0xAA + 0x80000000;
            FRONTEND_EMIT(arg0,
                          (((((temp_v1_2 & 6) | 0x18) & 0xFF) << 16) |
                              0x04000626),
                          temp_v1_2);
            FRONTEND_EMIT(arg0, 0x05310040, D_7BE40);
            var_ra += 0x14;
            var_v1 = var_v1 + 0x28;
            if (var_s1 != 0x10) {
                goto loop_7;
            }
            var_a0 = temp_s3;
            if (spD8 & 1) {
                spB8 ^= -0x100;
            }
            spD8 = spD8 + 1;
            sp40 += 0x11;
        } while (spD8 != 0x10);
        func_80034920(arg0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80037C74.s")
#endif

/* Workbench verdict: structure-mismatch, 365 differing words; target 368/candidate 342 words. */
/* First mismatch: +0x0; both frames are 0xE8, with 86 relocation-site differences. */
/* Structural gap: Gfx emission schedule, relocation web, and render-loop shape remain unresolved. */
#ifdef NON_MATCHING
void func_80038190(Gfx **arg0, Mtx **arg1, MainVertex **arg2) {
    s32 spE0;
    s32 spDC;
    f32 spD8;
    u8 *viewport;
    s32 var_a2;
    s32 var_a3;
    s32 var_t2;
    s32 var_t3;
    s32 var_t4;
    s32 var_v0;
    u32 temp_v1;
    u32 temp_v1_2;

    if ((D_8007BE80 != 0) && (D_8007BEA8 != 0)) {
        FRONTEND_EMIT(arg0, 0xE7000000, 0);
        FRONTEND_EMIT(arg0, 0xED000000, 0x5003C0);
        if (D_8007BEA8 == 2) {
            FRONTEND_EMIT(arg0, 0xEF30000F, 0);
            FRONTEND_EMIT(arg0, 0xF7000000, 0x10001);
            FRONTEND_EMIT(arg0, 0xF64FC3BC, 0);
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
                TrapDanglingJump(spD8, D_8007BEB0, 0x10, 0x10, 0x14, 0xF);
                break;
            }
            D_8007BEE0 = (D_8007BEE0 + 1) & 1;
            viGetCurrentSize(&spE0, &spDC);
            viewport = D_8007BEC0 + (D_8007BEE0 * 0x10);
            *(s16 *) (viewport + 8) = (s16) (spE0 * 2);
            *(s16 *) (viewport + 0xA) = (s16) (spDC * 2);
            *(s16 *) (viewport + 0) = (s16) (spE0 * 2);
            *(s16 *) (viewport + 2) = (s16) (spE0 * 2);
            FRONTEND_EMIT(arg0, 0xE7000000, 0);
            FRONTEND_EMIT(arg0, 0xBC000406,
                          (u32) D_800D2FAC + 0x80000000u);
            FRONTEND_EMIT(arg0, 0xBC001006,
                          (u32) D_800D2FAC + 0x7FFFFB00u);
            FRONTEND_EMIT(arg0, 0xBC000806, 0x80000000u);
            FRONTEND_EMIT(arg0, 0xFF10013F, 0x01000000);
            FRONTEND_EMIT(arg0, 0xB6000000, 0x10001);
            FRONTEND_EMIT(arg0, 0xEF20000F, 0);
            var_a3 = 0;
            var_a2 = 0;
            do {
                var_t2 = var_a2 + 4;
                FRONTEND_EMIT(arg0, 0xFD100000,
                              (u32) D_800D2FA8 + var_a3);
                var_a3 += 0xA00;
                FRONTEND_EMIT(arg0, 0xF5100000, 0x07080200);
                FRONTEND_EMIT(arg0, 0xE6000000, 0);
                FRONTEND_EMIT(arg0, 0xF3000000, 0x074FF01A);
                FRONTEND_EMIT(arg0, 0xE7000000, 0);
                FRONTEND_EMIT(arg0, 0xF510A000, 0x80200);
                FRONTEND_EMIT(arg0, 0xF2000000, 0x4FC00C);
                FRONTEND_EMIT(arg0, 0xE4500000 | ((var_t2 * 4) & 0xFFF),
                              (var_a2 * 4) & 0xFFF);
                FRONTEND_EMIT(arg0, 0xB3000000, 0);
                FRONTEND_EMIT(arg0, 0xB2000000, 0x10000400);
                var_a2 = var_t2;
            } while (var_t2 != 0xF0);
            FRONTEND_EMIT(arg0, 0xE7000000, 0);
            FRONTEND_EMIT(arg0, 0xBC000406,
                          (u32) D_800D2FA8 + 0x80000000u);
            FRONTEND_EMIT(arg0, 0xBC001006,
                          (u32) D_800D2FA8 + 0x7FFFFB00u);
            FRONTEND_EMIT(arg0, 0xFF10013F, 0x01000000);
            FRONTEND_EMIT(arg0, 0x03800010, (u32) viewport + 0x80000000u);
            if ((D_8007BE90 == 4) || (D_8007BE90 == 5)) {
                TrapDanglingJump((f32) (u32) arg0, (s32) arg1, (s32) arg2,
                                 spD8, 0x10, 0x10, 4, 0x28, 0xF);
            } else {
                func_80037C74(arg0, arg1, arg2);
            }
            D_8007BE84 ^= 1;
        }
        FRONTEND_EMIT(arg0, 0xE7000000, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80038190.s")
#endif
#undef FRONTEND_EMIT

/* PLATEAU-HANDOFF:func_800371BC:start
 * symbol: func_800371BC
 * score: 114/150 words
 * frame: 0x38
 * relocations: 15
 * first-mismatch: +0x2C
 * summary: Declared (void) with its call site in func_80037414: 144 to 114; size delta +4 remains
 * PLATEAU-HANDOFF:func_800371BC:end
 */

/* PLATEAU-HANDOFF:func_80037414:start
 * symbol: func_80037414
 * score: 13/145 words
 * frame: 0x30
 * relocations: 42
 * first-mismatch: +0x34
 * summary: Delta 0 and frame closed; last 13 words are uopt propagating the expression, not var_a1, into the TrapDanglingJump argument
 * PLATEAU-HANDOFF:func_80037414:end
 */


/* PLATEAU-HANDOFF:func_80037C74:start
 * symbol: func_80037C74
 * score: 314 differing words
 * frame: 0xE0
 * relocations: 18
 * first-mismatch: +0x0
 * summary: Frame closed and one missing emit site restored, 324 to 314 words and size delta -76 to -64. The frame was a pointer census, not a scalar one: every block-scoped Gfx cursor inside the emit macro owns four bytes of frame even though it is register-allocated, so the frame is 0x40 plus four bytes per cursor plus four per declared scalar plus twelve bytes of compiler temp, rounded to eight -- collapsing twenty-one per-site cursors to one shared cursor moved the frame 0xF0 to 0xA0, exactly four times twenty. The target's 0xE0 solves to twenty-two cursors and sixteen declared scalars where the m2c draft had twenty-one and twenty-one; four of the draft's temps were inlined to reach it and the masked count fell only ten words, so the frame was never the dominant cause. Counting the target's cursor write-backs gives twenty-two emit sites against the draft's twenty-one; the missing one repeats the loop's first command word against a different second word, immediately after the fourth command of the loop body, and restoring it is worth six words of the size deficit. What remains is one axis and it is not colouring: uopt hoists this loop's opcode and address constants into callee-saved registers and the ROM does not, re-materialising each one inside the loop through the assembler's own temporary. Both sides use all nine callee-saved registers and disagree only on which invariants win them, so the lever is loop register pressure, not declaration order.
 * PLATEAU-HANDOFF:func_80037C74:end
 */

/* PLATEAU-HANDOFF:func_80038190:start
 * symbol: func_80038190
 * score: 365 differing words
 * frame: 0xE8
 * relocations: 32
 * first-mismatch: +0x0
 * summary: Candidate saves only s0 and ra where the target saves s0-s8: about 18 of the 26 words it is short are those eight missing save/restore pairs, so the source holds eight more values live across its calls than the m2c draft does. Frames already agree at 0xE8. Rebuild it as ordinary C first, as func_800378A4 and func_800371BC in this TU were.
 * PLATEAU-HANDOFF:func_80038190:end
 */
