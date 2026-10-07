typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct O38Particle {
    f32 velocity, x, y, z, dx, dy, dz;
} O38Particle;

typedef struct O38Pool {
    s32 count, alpha;
    O38Particle particles[20];
} O38Pool;

typedef struct O38Object {
    s16 type, pad02;
    f32 pad04, age, x, y, z;
    char pad18[0x4C];
    O38Pool *pool;
} O38Object;

extern f32 gO38AgeRate;
extern f32 gO38AccelerationPosition;
extern f32 gO38AccelerationVelocity;
extern void o38ReleaseObject(O38Object *object);

/* 2026-10-07 (lane h-1): written from the listing, 159 -> 126 at size delta 0.
 * The loop is IDO's own 4x unroll of a plain 20-step loop. Two facts set the
 * target's FP colour ladder without any force: the timestep is converted once
 * and copied to a second local at entry (`loopDt = dt` in the declarations),
 * which is the target's cvt into f0 plus the f14 copy in the delay slot of the
 * alpha test; and exactly three body values are locals (dx, dz, x), whose
 * symbol webs span all four unrolled copies (save 80) and so rank above the
 * loop timestep (62), which then takes f14 (c27), the four dy CSE webs
 * c28..c31, and the two accelerations f24/f26. Two or four locals change the
 * size. The rates are literals (the extern names score 130). */
#ifdef NON_MATCHING
void func_overlay_038_F0000154_1885E64(O38Object *object, s32 ticks)
{
    O38Pool *pool = object->pool;
    f32 dt = ticks;
    f32 loopDt = dt;
    O38Particle *particle;
    f32 accelerationPosition;
    f32 accelerationVelocity;
    f32 x, dx, dz;
    s32 i;

    if (pool->alpha == 0) {
        o38ReleaseObject(object);
        return;
    }
    object->age += 0.1f * dt;
    pool->alpha -= (s32)(4.25f * dt);
    if (pool->alpha < 0) {
        pool->alpha = 0;
    }
    accelerationPosition = -0.05f * loopDt * loopDt;
    accelerationVelocity = -0.1f * loopDt;
    for (i = 0; i < 20; i++) {
        particle = &pool->particles[i];
        dx = particle->dx;
        dz = particle->dz;
        x = particle->x;
        particle->y = particle->y + (particle->dy * loopDt + accelerationPosition);
        particle->x = x + dx * loopDt;
        particle->dy += accelerationVelocity;
        particle->z = particle->z + dz * loopDt;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o038/overlay38UpdateParticles/func_overlay_038_F0000154_1885E64.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_038_F0000154_1885E64:start
 * symbol: func_overlay_038_F0000154_1885E64
 * score: 126 differing words
 * frame: 0x38
 * relocations: 7
 * first-mismatch: +0xC8
 * summary: 126 at size delta 0 on a natural loop: entry copy of the timestep and three body locals give the target's FP ladder unforced; the body schedule remains.
 * PLATEAU-HANDOFF:func_overlay_038_F0000154_1885E64:end
 */
