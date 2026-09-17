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

typedef union Overlay46StateC {
    s32 state;
    u8 bytes[4];
} Overlay46StateC;

extern s32 D_10;
extern s32 D_14;
extern s32 D_18;
extern u8 D_1C[];
extern Overlay46Particle D_20[];
extern u8 D_2C[];
extern f32 D_4C;
extern f32 D_50;
extern f32 D_54;
extern s32 D_5C;
extern Overlay46StateC D_C;

extern u8 gOverlay46RenderData0[];
extern u8 gOverlay46RenderData1[];
extern u8 gOverlay46RenderData2[];
extern void *gOverlay46ParticleModel;
extern void *gOverlay46ParticleMaterial;
extern Overlay46DisplayCommand *gDisplayListHead;

extern void overlay46RenderBeginReloc(void);
extern void overlay46RenderLoadReloc(void *data);
extern void overlay46RenderBindReloc(void *data0, void *data1);
extern s32 overlay46RandomRangeReloc(s32 minimum, s32 maximum);
extern f32 overlay46SinReloc(u16 angle);
extern void overlay46SetRenderModeReloc(s32 mode);
extern void overlay46SetColorReloc(s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay46SetPrimColorReloc(s32 red, s32 green, s32 blue,
                                       s32 alpha, s32 intensity);
extern void overlay46DrawPanelReloc(void *data, s32 x, s32 y, void *state,
                                    s32 size);
extern void overlay46DrawPanelExReloc(void *data0, void *data1, s32 x, s32 y,
                                      s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay46LoadParticleMaterialReloc(void *material);
extern void overlay46DrawParticleReloc(void *data, void *model,
                                       void *material,
                                       Overlay46Particle *particle,
                                       void *resource, s32 flags, s32 alpha);

/* Pinned DKR v77/v80 and JFG skeleton scans found no close donor. */
/* Workbench: size-mismatch; 452/450 instructions, 50 masked words, first
 * +0x1FC; frame 0xC0. Deleting decompiler interpolation/fade/display-list
 * temps closed the 32-byte non-save excess. Case-local step is required so
 * a second callee-saved float is not kept. L99 unused pointer above the
 * 19-slot table places the table at the target home. Indexed draw keeps
 * that colouring; an explicit walking pointer for the same loop rotates the
 * saved-register assignment across the whole function. Remains: draw-loop
 * strength reduction of the index, and overlay of the two 4-byte spill
 * homes onto the target's one shared temp. */
#ifdef NON_MATCHING
s32 func_overlay_046_F0000874_188EC6C(s32 updateRate) {
    s32 finished;
    s32 count;
    s32 result;
    Overlay46Particle *particle;
    void *unused; /* L99: 4-byte home above the variant table */
    Overlay46Particle *particlesByVariant[19];

    overlay46RenderBeginReloc();
    result = 1;
    overlay46RenderLoadReloc(gOverlay46RenderData0);
    overlay46RenderBindReloc(gOverlay46RenderData1, gOverlay46RenderData2);

    count = 0x12;
    do {
        particlesByVariant[count] = NULL;
    } while (count--);

    switch (D_C.state) {
    case 1: {
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_4C;
        do {
            particle->progress24 += step;
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
            D_C.state = 2;
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
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_50;
        do {
            particle->progress24 += step;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }

            particle->angle18 =
                particle->angle18 + (particle->angle1A * updateRate);
            if (particle->angle18 >= 0x8001) {
                particle->angle18 = particle->angle18 - 0x8000;
                particle->angle1A = overlay46RandomRangeReloc(0x600, 0xA00);
            }

            particle->angle34 += particle->angle06 * updateRate;
            if (particle->angle34 < -0x1000) {
                particle->angle34 = -0x1000;
                particle->angle06 = overlay46RandomRangeReloc(0x100, 0x200);
            } else if (particle->value04 >= 0x1001) {
                particle->angle34 = 0x1000;
                particle->angle06 =
                    -overlay46RandomRangeReloc(0x100, 0x200);
            }

            particle->baseX0C = particle->targetX2C;
            particle->baseY10 =
                (overlay46SinReloc(particle->angle18) *
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
                    D_C.state = 4;
                }
            }
        }
        break;
    }

    case 4: {
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_54;
        do {
            particle->progress24 -= step;
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

    overlay46SetRenderModeReloc(2);
    overlay46SetColorReloc(0, 0, 0, 0);
    if (D_10 != 0) {
        overlay46SetPrimColorReloc(0xFF, 0xFF, 0xFF, 0xFF, D_10);
        overlay46DrawPanelReloc(gOverlay46RenderData0, 0xA0, 0xAC, &D_C,
                                0xC);
    }
    if (D_14 != 0) {
        overlay46SetPrimColorReloc(0xFF, 0xFF, 0xFF, 0xFF, D_14);
        overlay46DrawPanelReloc(gOverlay46RenderData0, 0xA0, 0xB6, D_1C,
                                0xC);
    }
    if (D_18 != 0) {
        overlay46DrawPanelExReloc(gOverlay46RenderData0, D_2C, 0xA0, 0xCC,
                                  0xFF, 0xFF, 0xFF, D_18);
    }

    overlay46LoadParticleMaterialReloc(gOverlay46ParticleMaterial);
    gDisplayListHead->w1 = 0xFFFFFFFF;
    gDisplayListHead->w0 = 0xFA000000;
    gDisplayListHead++;

    count = 0;
    do {
        particle = particlesByVariant[count];
        if (particle != NULL) {
            overlay46DrawParticleReloc(
                gOverlay46RenderData0, gOverlay46ParticleModel,
                gOverlay46ParticleMaterial, particle, particle->resource38,
                0x8001, 0xFF);
        }
        count++;
    } while (count != 19);

    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o046/func_overlay_046_F0000874_188EC6C/func_overlay_046_F0000874_188EC6C.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_046_F0000874_188EC6C:start
 * symbol: func_overlay_046_F0000874_188EC6C
 * score: 50/450 words
 * frame: 0xC0
 * relocations: 96
 * first-mismatch: +0x1FC
 * summary: Frame 0xC0 closed; indexed draw is 50 masked at delta +8. A walking pointer rotates colouring. Next is draw-loop strength reduction.
 * PLATEAU-HANDOFF:func_overlay_046_F0000874_188EC6C:end
 */
