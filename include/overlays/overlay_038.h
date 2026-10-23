#ifndef OVERLAY_038_H
#define OVERLAY_038_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/*
 * Overlay 38: one translation unit (ADR 0006), a particle burst effect: spawn,
 * per-tick update and draw. The per-function views collapse into one object
 * layout that starts with the engine's 0x18-byte transform; the spawn's
 * descriptor angle seeds the transform's yaw and the update grows its scale.
 * PROVENANCE: Mickey-derived; no external C is adapted in this module except
 * the two packet macros below.
 */

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
    f32 x;
    f32 y;
    f32 z;
    f32 dx;
    f32 dy;
    f32 dz;
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

typedef struct O38Descriptor {
    u8 pad00[0xA];
    s16 angle;
} O38Descriptor;

typedef struct O38DirectionInput {
    u8 pad00[0x4];
    s16 angles[2];
} O38DirectionInput;

/* Overlay 38's own vertex and triangle data, reached through the module's
 * LOCAL relocation records. */
typedef struct O38Vertex {
    s16 x, y, z;
    u8 red, green, blue, alpha;
} O38Vertex;

typedef struct O38UV {
    s16 u, v;
} O38UV;

typedef struct O38Triangle {
    u8 flags, firstIndex, secondIndex, thirdIndex;
    O38UV coordinates[3];
} O38Triangle;

extern O38Vertex gO38ObjectVertices[4];
extern O38Triangle gO38ObjectTriangles[2];
extern O38Vertex gO38ParticleVertices[4];
extern O38Triangle gO38ParticleTriangles[2];

extern s32 mathRnd(s32 minimum, s32 maximum);
extern void mathOneFloatPY(s16 *rotation, f32 *vector);
extern void func_80006EA0(O38Object *object);
extern void camPushModelMtx(Gfx **dList, void *context,
                            O38Transform *transform, f32 scale, f32 offset);
extern void texDPTextureX(Gfx **dList, void *texture, s32 flags, s32 arg3);
extern void camPopModelMtx(Gfx **dList);
extern O38Camera *camGetPtr(void);
extern s32 Arctanf(f32 y, f32 x);
extern f32 sqrtf(f32 value);

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

void func_overlay_038_F0000000_1885D10(O38Object *object,
                                       O38Descriptor *descriptor);
void func_overlay_038_F0000154_1885E64(O38Object *object, s32 ticks);
void func_overlay_038_F000047C_188618C(Gfx **dList, void *context,
                                       O38Object *object);

#endif
