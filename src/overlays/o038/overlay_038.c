#include "overlays/overlay_038.h"

/* Emit one burst of particles for an object, seeding each with a random
 * horizontal offset, speed and pitch.
 * PROVENANCE: Mickey-derived; no external C is adapted here.
 *
 * Matched 2026-09-10 by three edits that only work together:
 *
 * 1. `direction` is initialised inside a `if (1) { }` region. Written as a
 *    plain statement, uopt reassociates `particle + 16` through `particle`'s
 *    own definition into `pool + 24`, and the direction cursor stops
 *    depending on the particle cursor -- which is what reorders the whole
 *    six-instruction preheader. Only control flow opens a uopt region, and a
 *    region boundary anywhere between the two definitions blocks the fold; a
 *    bare block does not, and neither does any spelling of the address.
 *
 * 2. The scratch rotation aggregate is eight bytes, not sixteen. The region
 *    costs eight bytes of compiler temporary below the declared block, so the
 *    aggregate has to give the same eight back for the frame to stay 0x60 and
 *    for its address to stay at sp+76.
 *
 * 3. `offset = 0;` and the region sit on one physical line. as1 breaks a tie
 *    between two ready nodes on their source line numbers, and on separate
 *    lines it emits the counter's clear ahead of the direction cursor; equal
 *    line numbers drop the tie to ready-list position, which is the shipped
 *    order. Reversing the two statements instead of folding them regresses,
 *    because the definition order is also what assigns s2 and s3.
 */
void func_overlay_038_F0000000_1885D10(O38Object *object,
                                       O38Descriptor *descriptor)
{
    O38Pool *pool = object->pool;
    O38Particle *particle;
    s32 offset;
    f32 *direction;
    O38DirectionInput randomDirection;

    object->trans.rotationY = descriptor->angle;
    particle = pool->particles;
    offset = 0; if (1) { direction = &particle->dx; }
    pool->count = 60;
    pool->alpha = 255;
    for (; offset != 0x230; offset += sizeof(O38Particle)) {
        particle->x = object->trans.x + (f32)mathRnd(-20, 20);
        particle->y = object->trans.y;
        particle->z = object->trans.z + (f32)mathRnd(-20, 20);
        particle->scale = (f32)mathRnd(1, 10) / 10.0f;
        randomDirection.angles[0] = mathRnd(-0x7FFF, 0x7FFF);
        randomDirection.angles[1] = mathRnd(0x2000, 0x4000);
        particle->dz = (f32)mathRnd(20, 50) / -10.0f;
        mathOneFloatPY(randomDirection.angles, direction);
        particle++;
        direction += sizeof(O38Particle) / sizeof(f32);
    }
}

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
        func_80006EA0(object);
        return;
    }
    object->trans.scale += 0.1f * dt;
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

/* Draw the object's quad, then one camera-facing quad per live particle.
 *
 * Matched 2026-10-02 (lane x-ovlb), 90 -> 0 masked words, by rewriting it in
 * the shape of the matched resident func_8003D25C (src/main/particles.c).
 * The old candidate held 87 register-naming words behind volatile packet
 * cursors, a struct standing in for the frame, a pool byte cursor and a
 * K&R call. The plain form needs none of them:
 * - every packet is one GBI macro on `(*dList)++`;
 * - the transform is the ordinary 0x18-byte struct, and the object and the
 *   camera both start with one, so their fields are read by name;
 * - the loop is `for (i = 0; i < 20; i++)` over `&pool->particles[i]`; IDO
 *   makes the byte counter and the pool cursor itself;
 * - the locals are declared pool, particle, camera, transform, then the
 *   scalars, which puts the transform at sp+0xC4 under two pointer cells. */
void func_overlay_038_F000047C_188618C(Gfx **dList, void *context, O38Object *object) {
    O38Pool *pool;
    O38Particle *particle;
    O38Camera *camera;
    O38Transform transform;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 i;

    pool = object->pool;
    transform.rotationY = object->trans.rotationY;
    transform.rotationX = 0x4000;
    transform.rotationZ = 0;
    transform.scale = object->trans.scale;
    transform.x = object->trans.x;
    transform.y = object->trans.y;
    transform.z = object->trans.z;
    camPushModelMtx(dList, context, &transform, 1.0f, 0.0f);
    texDPTextureX(dList, object->resource[0], 0x10, 0);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, pool->alpha);
    gSPVertexJFG((*dList)++, gO38ObjectVertices, 4, 0);
    gSPPolygon((*dList)++, gO38ObjectTriangles, 2, 1);
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    camPopModelMtx(dList);
    camera = camGetPtr();
    for (i = 0; i < 20; i++) {
        particle = &pool->particles[i];
        if (object->trans.y <= particle->y) {
            dx = particle->x - camera->trans.x;
            dy = particle->y - camera->trans.y;
            dz = particle->z - camera->trans.z;
            transform.rotationY = Arctanf(dx, dz);
            transform.rotationX = Arctanf(-dy, sqrtf(dx * dx + dz * dz)) + 0x8000;
            transform.rotationZ = 0;
            transform.scale = particle->scale;
            transform.x = particle->x;
            transform.y = particle->y;
            transform.z = particle->z;
            camPushModelMtx(dList, context, &transform, 1.0f, 0.0f);
            texDPTextureX(dList, object->resource[1], 0x10, 0);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, pool->alpha);
            gSPVertexJFG((*dList)++, gO38ParticleVertices, 4, 0);
            gSPPolygon((*dList)++, gO38ParticleTriangles, 2, 1);
            gDPPipeSync((*dList)++);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
            camPopModelMtx(dList);
        }
    }
}
