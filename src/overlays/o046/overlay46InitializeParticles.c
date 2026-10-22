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
    s16 angle18;
    s16 angle1A;
    f32 positionX1C;
    f32 positionY20;
    f32 value24;
    f32 value28;
    f32 targetX2C;
    f32 targetY30;
    s16 value34;
    s16 variant36;
    void *resource38;
} Overlay46Particle;

typedef struct Overlay46ParticleConfig {
    u8 resourceIndex;
    u8 variant;
    s8 x;
    s8 y;
} Overlay46ParticleConfig;

extern void *D_800D31C8[];
extern s32 gOverlay46StateC;
extern s32 gOverlay46Value10;
extern s32 gOverlay46Value14;
extern s32 gOverlay46Value18;
extern Overlay46Particle gOverlay46Particles20[];
extern f32 gOverlay46Spacing48;
extern s32 gOverlay46Timer5C;
extern Overlay46ParticleConfig gOverlay46Configs1C8[];

extern s32 mathRnd(s32 minimum, s32 maximum);
extern f32 overlay46RandomSignedReloc(s32 magnitude);
extern s32 func_800299E8(s32 minimum, s32 maximum);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
/*
 * IDO 5.3 uses the loop opener's source-line boundary when scheduling the
 * independent resource-table address and loop count initializations. Keeping
 * the count assignment with the do opener reproduces the retail order.
 */
void overlay46InitializeParticles(void) {
    Overlay46Particle *particle;
    Overlay46ParticleConfig *config;
    void **resources;
    f32 offsetY;
    f32 baseY;
    f32 spacing;
    s32 magnitude;
    s32 index;

    particle = gOverlay46Particles20;
    config = gOverlay46Configs1C8;
    spacing = gOverlay46Spacing48;
    magnitude = -0x4000;
    resources = D_800D31C8;
    index = 0x12; do {
        particle->value00 = 0;
        particle->value02 = 0;
        particle->value04 = 0;
        particle->angle06 = mathRnd(0x100, 0x200);
        particle->scale08 = 1.0f;
        particle->baseX0C = config->x;
        particle->baseY10 = config->y;
        particle->value14 = 0.0f;
        particle->angle18 = mathRnd(0, 0x8000);
        particle->angle1A = mathRnd(0x600, 0xA00);
        particle->positionX1C =
            overlay46RandomSignedReloc(magnitude) * 400.0f +
            particle->baseX0C;
        offsetY = overlay46RandomSignedReloc(magnitude) * 400.0f;
        baseY = particle->baseY10;
        particle->value28 = 0.0f;
        particle->positionY20 = offsetY + baseY;
        particle->value24 = (f32)index * spacing;
        particle->resource38 = resources[config->resourceIndex];
        particle->value34 = 0;
        particle->targetY30 = baseY;
        particle->targetX2C = particle->baseX0C;
        particle->variant36 = config->variant;
        if (func_800299E8(0, 1) != 0) {
            particle->angle06 = -particle->angle06;
        }
        magnitude += 0x1000;
        particle++;
        config++;
    } while (index--);

    gOverlay46StateC = 1;
    gOverlay46Value10 = 0;
    gOverlay46Value14 = 0;
    gOverlay46Value18 = 0;
    gOverlay46Timer5C = 0x3C;
}
