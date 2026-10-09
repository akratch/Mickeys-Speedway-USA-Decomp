#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct O38Transform {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} O38Transform;

typedef struct O38Particle {
    f32 scale;
    f32 x, y, z;
    f32 dx, dy, dz;
} O38Particle;

typedef struct O38Pool {
    s32 count;
    s32 alpha;
    O38Particle particles[20];
} O38Pool;

typedef struct O38Object {
    O38Transform trans;
    u8 pad18[0x4C];
    O38Pool *pool;
    void **resource;
} O38Object;

typedef struct O38Camera {
    O38Transform trans;
} O38Camera;

/* Overlay 38's own vertex and triangle data, reached through the module's
 * LOCAL relocation records. */
extern u8 gO38ObjectVertices[];
extern u8 gO38ObjectTriangles[];
extern u8 gO38ParticleVertices[];
extern u8 gO38ParticleTriangles[];

/* Resident callees, reached through SYMBOL records: camPushModelMtx,
 * texDPTextureX, camPopModelMtx, camGetPtr, Arctanf and sqrtf. */
extern void o38PushMatrixReloc(Gfx **dList, s32 context,
                               O38Transform *transform, f32 scale, f32 offset);
extern void o38DrawTextureReloc(Gfx **dList, void *texture, s32 flags,
                                s32 arg3);
extern void o38PopMatrixReloc(Gfx **dList);
extern O38Camera *o38GetCameraReloc(void);
extern s16 o38ArctanReloc(f32 y, f32 x);
extern f32 o38SqrtReloc(f32 value);

/* PROVENANCE: the two packet macros are Jet Force Gemini's gSPVertexJFG and
 * gSPPolygon (include/f3ddkr.h in its public decompilation, a permitted
 * source under docs/CLEANROOM.md), copied for the vertex-load and
 * triangle-list commands this overlay emits. */
#define gSPVertexJFG(pkt, v, n, v0) \
    gDma1p(pkt, G_VTX, v, ((((n) << 3) + ((n) << 1))) + 8, \
           ((n)) << 3 | (((u32)(v) & 6)) | (v0))
#define gSPPolygon(dl, ptr, numTris, texEnabled) { \
    Gfx *_g = (Gfx *)(dl); \
    _g->words.w0 = _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | \
                   _SHIFTL(5, 24, 8) | _SHIFTL(((numTris) * 16), 0, 16); \
    _g->words.w1 = (unsigned int)(ptr); \
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
void func_overlay_038_F000047C_188618C(Gfx **dList, s32 context, O38Object *object) {
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
    o38PushMatrixReloc(dList, context, &transform, 1.0f, 0.0f);
    o38DrawTextureReloc(dList, object->resource[0], 0x10, 0);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, pool->alpha);
    gSPVertexJFG((*dList)++, gO38ObjectVertices, 4, 0);
    gSPPolygon((*dList)++, gO38ObjectTriangles, 2, 1);
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    o38PopMatrixReloc(dList);
    camera = o38GetCameraReloc();
    for (i = 0; i < 20; i++) {
        particle = &pool->particles[i];
        if (object->trans.y <= particle->y) {
            dx = particle->x - camera->trans.x;
            dy = particle->y - camera->trans.y;
            dz = particle->z - camera->trans.z;
            transform.rotationY = o38ArctanReloc(dx, dz);
            transform.rotationX = o38ArctanReloc(-dy, o38SqrtReloc(dx * dx + dz * dz)) + 0x8000;
            transform.rotationZ = 0;
            transform.scale = particle->scale;
            transform.x = particle->x;
            transform.y = particle->y;
            transform.z = particle->z;
            o38PushMatrixReloc(dList, context, &transform, 1.0f, 0.0f);
            o38DrawTextureReloc(dList, object->resource[1], 0x10, 0);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, pool->alpha);
            gSPVertexJFG((*dList)++, gO38ParticleVertices, 4, 0);
            gSPPolygon((*dList)++, gO38ParticleTriangles, 2, 1);
            gDPPipeSync((*dList)++);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
            o38PopMatrixReloc(dList);
        }
    }
}
