#include "PR/ultratypes.h"

typedef struct Overlay46Particle {
    s16 value00;
    s16 value02;
    s16 value04;
    s16 angle06;
    f32 scale08;
    f32 baseX0C;
    f32 baseY10;
    f32 value14;
    u16 angle18;
    s16 angle1A;
    f32 positionX1C;
    f32 positionY20;
    f32 progress24;
    f32 value28;
    f32 targetX2C;
    f32 targetY30;
    s16 angle34;
    s16 variant36;
    void *resource38;
} Overlay46Particle;

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

extern s32 D_10;
extern s32 D_14;
extern s32 D_18;
extern Overlay46Particle D_20[];
extern u8 D_2C[];
extern s32 D_5C;
extern s32 D_C;

/* Resident display-list, matrix and vertex cursors, reached through runtime
 * relocation records and passed by address. */
extern Overlay46DisplayCommand *gDisplayListHead;
extern void *gOverlay46MatrixHead;
extern void *gOverlay46VertexHead;

extern void func_80037658(void);
extern void texDPInit(void *commands);
extern void camStandardOrtho(void *commands, void *matrices);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern f32 func_8002A8C0(u16 angle);
extern void func_8004B0A4(s32 mode);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern void func_8004B0F8(void *commands, s32 x, s32 y, void *text,
                          s32 size);
extern void func_8002F618(void *commands, void *data, s32 x, s32 y,
                          s32 red, s32 green, s32 blue, s32 alpha);
extern void camDo2DSprite(void *commands, void *matrices, void *vertices,
                          Overlay46Particle *particle, void *resource,
                          s32 flags, s32 alpha);

#define O46_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))
#define O46_PRIM(packet, red, green, blue, alpha) { \
    Overlay46DisplayCommand *macroCommand = (Overlay46DisplayCommand *)(packet); \
    macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8); \
    macroCommand->w1 = O46_SHIFTL(red, 24, 8) | \
        O46_SHIFTL(green, 16, 8) | O46_SHIFTL(blue, 8, 8) | \
        O46_SHIFTL(alpha, 0, 8); \
}

/* Pinned DKR v77/v80 and JFG skeleton scans found no close donor.
 *
 * Matched 2026-10-01 by writing what the relocation records name instead of
 * the inherited placeholders:
 * - the three per-state step factors and the two captions are literals in
 *   this TU's own pool (LOCAL records against the module rodata), not
 *   globals, so each step is a hoisted invariant and the state-2 step shares
 *   the one compiler temporary with the draw loop's spilled walker;
 * - every "render data" argument is the address of one of three resident
 *   cursors (display list, matrix, vertex), not a loaded value;
 * - the primitive-colour command is the one-line packet macro, whose
 *   block-scoped pointer is the home below the table. */
s32 func_overlay_046_F0000874_188EC6C(s32 updateRate) {
    s32 finished;
    s32 count;
    s32 result;
    Overlay46Particle *particle;
    s32 pad; /* fifth scalar home above the table (L99) */
    Overlay46Particle *particlesByVariant[19];

    func_80037658();
    result = 1;
    texDPInit(&gDisplayListHead);
    camStandardOrtho(&gDisplayListHead, &gOverlay46MatrixHead);

    count = 0x12;
    do {
        particlesByVariant[count] = NULL;
    } while (count--);

    switch (D_C) {
    case 1: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 += updateRate * 0.01f;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }
            particle->baseX0C =
                particle->positionX1C
                + ((particle->targetX2C - particle->positionX1C)
                   * particle->progress24);
            particle->baseY10 =
                particle->positionY20
                + ((particle->targetY30 - particle->positionY20)
                   * particle->progress24);
            particle->value04 = (s16)(2.0f * (particle->progress24 * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            D_C = 2;
            particle = D_20;
            count = 0x12;
            do {
                particle->progress24 = 0.0f;
                particle++;
            } while (count--);
        }
        break;
    }

    case 2: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 += updateRate * 0.05f;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }

            particle->angle18 =
                particle->angle18 + (particle->angle1A * updateRate);
            if (particle->angle18 >= 0x8001) {
                particle->angle18 = particle->angle18 - 0x8000;
                particle->angle1A = mathRnd(0x600, 0xA00);
            }

            particle->angle34 += particle->angle06 * updateRate;
            if (particle->angle34 < -0x1000) {
                particle->angle34 = -0x1000;
                particle->angle06 = mathRnd(0x100, 0x200);
            } else if (particle->value04 >= 0x1001) {
                particle->angle34 = 0x1000;
                particle->angle06 =
                    -mathRnd(0x100, 0x200);
            }

            particle->baseX0C = particle->targetX2C;
            particle->baseY10 =
                (func_8002A8C0(particle->angle18) *
                 particle->progress24 * 5.0f) + particle->targetY30;
            particle->value04 =
                (s16)((f32)particle->angle34 * particle->progress24);
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            if (D_10 < 0xFF) {
                D_10 += updateRate * 4;
                if (D_10 >= 0x100) {
                    D_10 = 0xFF;
                }
            } else if (D_14 < 0xFF) {
                D_14 += updateRate * 4;
                if (D_14 >= 0x100) {
                    D_14 = 0xFF;
                }
            } else if (D_18 < 0xFE) {
                D_18 += updateRate * 4;
                if (D_18 >= 0xFF) {
                    D_18 = 0xFE;
                }
            } else {
                D_5C -= updateRate;
                if (D_5C <= 0) {
                    D_C = 4;
                }
            }
        }
        break;
    }

    case 4: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 -= updateRate * 0.025f;
            if (particle->progress24 <= 0.0f) {
                particle->progress24 = 0.0f;
            } else {
                finished = 0;
            }
            particle->baseX0C =
                particle->positionX1C
                + ((particle->targetX2C - particle->positionX1C)
                   * particle->progress24);
            particle->baseY10 =
                particle->positionY20
                + ((particle->targetY30 - particle->positionY20)
                   * particle->progress24);
            particle->value04 =
                particle->angle34
                + (s32)(2.0f * (particle->progress24 * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        D_10 -= updateRate * 8;
        if (D_10 < 0) {
            D_10 = 0;
        }
        D_14 -= updateRate * 8;
        if (D_14 < 0) {
            D_14 = 0;
        }
        D_18 -= updateRate * 8;
        if (D_18 < 0) {
            D_18 = 0;
        }
        if (finished != 0) {
            result = 0;
        }
        break;
    }
    }

    func_8004B0A4(2);
    func_8004B0DC(0, 0, 0, 0);
    if (D_10 != 0) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, D_10);
        func_8004B0F8(&gDisplayListHead, 0xA0, 0xAC, "< 2000 Disney",
                                0xC);
    }
    if (D_14 != 0) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, D_14);
        func_8004B0F8(&gDisplayListHead, 0xA0, 0xB6, "Licensed to Nintendo",
                                0xC);
    }
    if (D_18 != 0) {
        func_8002F618(&gDisplayListHead, D_2C, 0xA0, 0xCC,
                                  0xFF, 0xFF, 0xFF, D_18);
    }

    texDPInit(&gDisplayListHead);
    O46_PRIM(gDisplayListHead++, 255, 255, 255, 255);

    count = 0;
    do {
        particle = particlesByVariant[count++];
        if (particle != NULL) {
            camDo2DSprite(
                &gDisplayListHead, &gOverlay46MatrixHead,
                &gOverlay46VertexHead, particle, particle->resource38,
                0x8001, 0xFF);
        }
    } while (count != 19);

    return result;
}
