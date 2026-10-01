#include "PR/ultratypes.h"

typedef struct O63Particle {
    void *resource;
    s16 x;
    s16 y;
    s16 height;
    u16 angle;
    s16 angleRate;
    s16 heightRate;
    s8 jitter;
    u8 pad11[3];
} O63Particle;

typedef struct O63RenderPosition {
    s16 x;
    s16 y;
    s16 z;
    u16 pad6;
    f32 scale;
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad18[0x10];
    f32 scratch28;
    u8 pad2C[0x68];
} O63RenderPosition;

typedef struct O63LocalObject {
    s32 word0;
    s32 word4;
    s32 fixed8;
} O63LocalObject;

typedef struct O63Gfx {
    u32 word0;
    u32 word1;
} O63Gfx;

extern s32 o63CheckTriggerReloc(void);
extern void o63StartTriggerReloc(s32, s32, s32, s32, s32, s32, s32);
extern void o63SetStateReloc(s32);
extern void o63CommitStateReloc(void);
extern void o63ConfigureStateReloc(s32, s32, s32, s32, s32, s32);
extern void o63ResetTimerReloc(s32);
extern s32 o63CanDrawReloc(void);
extern void o63DrawRectReloc(s32, O63LocalObject *, s32, s32, s32, s32, s32, s32);
extern void o63SetOpacityReloc(void *, s32);
extern void o63UpdateObjectReloc(s32, void *, s32, f32 *, s32);
extern void o63PrepareRenderReloc(s32);
extern void o63PrepareRender2Reloc(s32, void *);
extern s16 o63RandomReloc(s32, s32);
extern f32 o63SinReloc(u16);
extern void o63RenderParticleReloc(void *, void *, void *, O63RenderPosition *, void *, s32, s32);
extern void func_overlay_063_F000077C_18C3304(s32);

extern u32 gO63ExternalFlagsReloc;
extern s32 gO63ExternalTimerReloc;
extern s32 gO63ExternalStateAReloc;
extern s32 gO63ExternalStateBReloc;
extern O63Gfx *gO63DrawContextReloc;
extern void *gO63OpacityContextReloc;
extern void *gO63RenderContextReloc;
extern void *gO63RenderMatrixReloc;

extern s32 gO63Opacity;
extern f32 gO63ObjectFloat;
extern void *gO63Local2C;
extern O63LocalObject gO63Local30;
extern O63Particle gO63Particles[18];
extern s32 gO63Fade;
extern s32 gO63FadeDirection;
extern s32 gO63Triggered;
extern s32 gO63TriggerTimer;
extern s32 gO63FadeTimer;

/*
 * Plateau: 104 masked words at delta 0, frame 0xF8 exact.  The particle loop
 * decrements the cursor and then reads the fields in place (the carriers
 * angle/angleRate were inherited): 137 to 109, and one spare local word
 * restores the target's frame: 104.  What remains is the saved-register
 * rotation: the target holds updateRate in s6 and the fade address in s7
 * (ours the reverse), and the draw/render contexts in s5/s4 (ours reversed).
 */
#ifdef NON_MATCHING
void overlay63UpdateEffects(s32 updateRate) {
    O63RenderPosition pos;
    O63Particle *particle;
    s32 count;
    s32 pad0; /* frame 0xF8: the target keeps 8 more bytes of locals */

    if (gO63Triggered == 0) {
        if ((gO63ExternalFlagsReloc & 0x9000) && (gO63ExternalTimerReloc <= 0)) {
            if (o63CheckTriggerReloc() == 1) {
                o63StartTriggerReloc(0, 0x3FC00000, 0x3F800000, 0, 0, 0, 1);
            }
            o63SetStateReloc(5);
            o63CommitStateReloc();
            o63ConfigureStateReloc(0, 0, 0, 0x10, 1, 1);
            gO63Triggered = 1;
        } else {
            gO63TriggerTimer += updateRate;
            if (gO63TriggerTimer >= 0xE10) {
                o63ResetTimerReloc(0);
                gO63ExternalStateAReloc = -1;
                gO63ExternalStateBReloc = 1;
                o63CommitStateReloc();
                o63ConfigureStateReloc(0x12, 0, 0, 0xF, 1, 0);
                o63SetStateReloc(1);
                gO63Triggered = 1;
            }
        }
    }

    if (gO63FadeTimer != 0) {
        gO63FadeTimer -= updateRate;
        if (gO63FadeTimer < 0) {
            gO63FadeTimer = 0;
        }
        if (gO63Fade == 0xFF) {
            if (gO63ExternalTimerReloc > 0) {
                gO63ExternalTimerReloc -= updateRate;
            } else {
                gO63Opacity += updateRate * 4;
                if (gO63Opacity >= 0x100) {
                    gO63Opacity = 0xFF;
                }
            }
        }
    } else if (gO63FadeDirection == 1) {
        gO63Fade += updateRate * 4;
        if (gO63Fade >= 0x100) {
            gO63Fade = 0xFF;
        }
        if (gO63Fade == 0xFF) {
            gO63FadeDirection = 0;
            gO63FadeTimer = 0xF0;
        }
    } else {
        gO63Fade -= updateRate * 4;
        if (gO63Fade < 0) {
            gO63Fade = 0;
        }
        if (gO63Fade == 0) {
            gO63FadeDirection = 1;
            gO63FadeTimer = 0x1E0;
        }
    }

    if ((gO63Opacity != 0) && (o63CanDrawReloc() == 0)) {
        o63DrawRectReloc((s32)&gO63DrawContextReloc, &gO63Local30, 0x40, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        o63DrawRectReloc((s32)&gO63DrawContextReloc, &gO63Local30, 0x100, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        o63SetOpacityReloc(gO63OpacityContextReloc, gO63Opacity);
        o63UpdateObjectReloc(gO63Local30.word0, &gO63Local2C, 2, &gO63ObjectFloat, updateRate);
        gO63Local30.fixed8 = (s32)(gO63ObjectFloat * 65536.0f);
    }

    o63PrepareRenderReloc((s32)&gO63DrawContextReloc);
    o63PrepareRender2Reloc((s32)&gO63DrawContextReloc, &gO63RenderContextReloc);
    if (gO63Fade != 0) {
        O63Gfx *gfx;

        pos.x = 0;
        pos.y = 0;
        pos.z = 0;
        pos.scale = 1.0f;
        pos.scratch28 = 0.0f;

        gfx = gO63DrawContextReloc++;
        gfx->word0 = 0xE7000000;
        gfx->word1 = 0;
        gfx = gO63DrawContextReloc++;
        gfx->word0 = 0xFA000000;
        gfx->word1 = (gO63Fade & 0xFF) | ~0xFF;

        particle = (O63Particle *)&gO63Fade;
        count = 18;
        do {
            particle--;
            particle->angle = particle->angle + particle->angleRate * updateRate;
            if (particle->angle >= 0x8001) {
                particle->angle -= 0x8000;
                particle->angleRate = o63RandomReloc(0x600, 0xA00);
            }
            particle->height += particle->heightRate * updateRate;
            if (particle->height < -0x1000) {
                particle->height = -0x1000;
                particle->heightRate = o63RandomReloc(0x100, 0x200);
            } else if (particle->height >= 0x1001) {
                particle->height = 0x1000;
                particle->heightRate = -o63RandomReloc(0x100, 0x200);
            }
            particle->jitter = (s8)(s32)(o63SinReloc(particle->angle) * 5.0f);
            pos.posX = (f32)particle->x;
            pos.posZ = 0.0f;
            pos.posY = (f32)(particle->y + particle->jitter);
            pos.z = particle->height;
            if (gO63Fade < 0xFF) {
                o63RenderParticleReloc(&gO63DrawContextReloc, &gO63RenderContextReloc,
                    &gO63RenderMatrixReloc, &pos, particle->resource, 5, gO63Fade);
            } else {
                o63RenderParticleReloc(&gO63DrawContextReloc, &gO63RenderContextReloc,
                    &gO63RenderMatrixReloc, &pos, particle->resource, 0x8001, gO63Fade);
            }
        } while (count--);
    }
    func_overlay_063_F000077C_18C3304(updateRate);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o063/overlay63UpdateEffects/func_overlay_063_F00001D4_18C2D5C.s")
#endif

/* PLATEAU-HANDOFF:overlay63UpdateEffects:start
 * symbol: overlay63UpdateEffects
 * score: 104/350 words
 * frame: 0xF8
 * relocations: 71
 * first-mismatch: +0x14
 * summary: Loop reads fields after the decrement, one spare local word: 137 to 104. Open: s6/s7 and s4/s5 pairs reversed.
 * PLATEAU-HANDOFF:overlay63UpdateEffects:end
 */
