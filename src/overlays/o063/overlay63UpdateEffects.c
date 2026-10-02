#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

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

extern s32 o63CheckTriggerReloc(void);
extern void o63StartTriggerReloc(s32, s32, s32, s32, s32, s32, s32);
extern void o63SetStateReloc(s32);
extern void o63CommitStateReloc(void);
extern void o63ConfigureStateReloc(s32, s32, s32, s32, s32, s32);
extern void o63ResetTimerReloc(s32);
extern s32 o63CanDrawReloc(void);
extern void o63DrawRectReloc(void *, O63LocalObject *, s32, s32, s32, s32, s32, s32);
extern void o63SetOpacityReloc(void *, s32);
extern void o63UpdateObjectReloc(s32, void *, s32, f32 *, s32);
extern void o63PrepareRenderReloc(void *);
extern void o63PrepareRender2Reloc(void *, void *);
extern s16 o63RandomReloc(s32, s32);
extern f32 o63SinReloc(u16);
extern void o63RenderParticleReloc(void *, void *, void *, O63RenderPosition *, void *, s32, s32);
extern void func_overlay_063_F000077C_18C3304(s32);

extern u32 gO63ExternalFlagsReloc;
extern s32 gO63ExternalTimerReloc;
extern s32 gO63ExternalStateAReloc;
extern s32 gO63ExternalStateBReloc;
extern Gfx *gO63DrawContextReloc;
extern void *gO63OpacityContextReloc;
extern void *gO63RenderContextReloc;
extern void *gO63RenderMatrixReloc;

extern s32 gO63Opacity;
extern f32 gO63ObjectFloat;
extern void *gO63Local2C;
extern O63LocalObject gO63Local30;
extern O63Particle gO63Particles[19];
extern s32 gO63Fade;
extern s32 gO63FadeDirection;
extern s32 gO63Triggered;
extern s32 gO63TriggerTimer;
extern s32 gO63FadeTimer;

/*
 * Matched (lane j-o035, 2026-10-02). The draw-context calls take the
 * context address as a pointer, the particle cursor starts at the end of
 * gO63Particles (not at the fade word's address), the two packets use the
 * GBI macros, the loop is `count = 19; while (count--)`, the particle
 * position stores run x, y, z-plane, height, and the position reset is a
 * do-while(0) macro whose region ranks the render context ahead of the
 * draw context for the callee-saved registers.
 */
#define O63_RESET_POSITION(p) \
    do { \
        (p).x = 0; \
        (p).y = 0; \
        (p).z = 0; \
        (p).scale = 1.0f; \
        (p).scratch28 = 0.0f; \
    } while (0)

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
        o63DrawRectReloc(&gO63DrawContextReloc, &gO63Local30, 0x40, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        o63DrawRectReloc(&gO63DrawContextReloc, &gO63Local30, 0x100, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        o63SetOpacityReloc(gO63OpacityContextReloc, gO63Opacity);
        o63UpdateObjectReloc(gO63Local30.word0, &gO63Local2C, 2, &gO63ObjectFloat, updateRate);
        gO63Local30.fixed8 = (s32)(gO63ObjectFloat * 65536.0f);
    }

    o63PrepareRenderReloc(&gO63DrawContextReloc);
    o63PrepareRender2Reloc(&gO63DrawContextReloc, &gO63RenderContextReloc);
    if (gO63Fade != 0) {

        O63_RESET_POSITION(pos);

        gDPPipeSync(gO63DrawContextReloc++);
        gDPSetPrimColor(gO63DrawContextReloc++, 0, 0, 0xFF, 0xFF, 0xFF, gO63Fade);

        particle = &gO63Particles[19];
        count = 19;
        while (count--) {
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
            pos.posY = (f32)(particle->y + particle->jitter);
            pos.posZ = 0.0f;
            pos.z = particle->height;
            if (gO63Fade < 0xFF) {
                o63RenderParticleReloc(&gO63DrawContextReloc, &gO63RenderContextReloc,
                    &gO63RenderMatrixReloc, &pos, particle->resource, 5, gO63Fade);
            } else {
                o63RenderParticleReloc(&gO63DrawContextReloc, &gO63RenderContextReloc,
                    &gO63RenderMatrixReloc, &pos, particle->resource, 0x8001, gO63Fade);
            }
        }
    }
    func_overlay_063_F000077C_18C3304(updateRate);
}
