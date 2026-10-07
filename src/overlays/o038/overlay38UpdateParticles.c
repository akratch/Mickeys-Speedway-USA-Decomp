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

extern void o38ReleaseObject(O38Object *object);

/* Matched 2026-10-07 (lane h-1), written from the listing. The loop is IDO's
 * own 4x unroll of a plain 20-step loop. The timestep is converted once and
 * copied to a second local at entry (`loopDt = dt`): the target's cvt into
 * f0 plus the f14 copy in the delay slot of the alpha test. The three
 * displacements are locals; their symbol webs span all four unrolled copies
 * and rank above the loop timestep, which is what gives the target's FP
 * colours with no force. The dy update sits between deltaY and deltaZ, which
 * is the target's store order in the later unrolled copies. The rates are
 * float literals in the module's own rodata. */
void func_overlay_038_F0000154_1885E64(O38Object *object, s32 ticks)
{
    O38Pool *pool = object->pool;
    f32 dt = ticks;
    f32 loopDt = dt;
    O38Particle *particle;
    f32 accelerationPosition;
    f32 accelerationVelocity;
    f32 deltaX, deltaY, deltaZ;
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
        deltaX = particle->dx * loopDt;
        deltaY = particle->dy * loopDt + accelerationPosition;
        particle->dy += accelerationVelocity;
        deltaZ = particle->dz * loopDt;
        particle->x += deltaX;
        particle->y += deltaY;
        particle->z += deltaZ;
    }
}
