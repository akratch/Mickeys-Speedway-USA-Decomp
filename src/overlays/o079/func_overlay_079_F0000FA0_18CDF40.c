#include "PR/ultratypes.h"

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

typedef struct Overlay79Plane {
    Overlay79Vector normal;
    f32 constant;
    Overlay79Vector origin;
    f32 distance;
    u32 flags;
} Overlay79Plane;

typedef struct Overlay79CollisionState {
    u8 pad0[8];
    u32 flags;
} Overlay79CollisionState;

typedef struct Overlay79CollisionObject {
    u8 pad0[0x64];
    Overlay79CollisionState *state;
} Overlay79CollisionObject;

extern f32 sqrtf(f32 value);
extern s32 Arctanf(f32 y, f32 x);
extern f32 func_8002A8BC(s32 angle);

/*
 * PROVENANCE: copied from the shape of the matched resident sibling
 * func_800115E4 (src/main/track.c), the same collision response with one
 * branch fewer; no external function body is adapted. The plane offset is
 * computed inside the slide branch, the three literals are written at their
 * uses (the module's constant pool, two distinct 0.01f words), and the
 * declaration order places d, delta, u, v and w on the target's stack homes.
 */
void func_overlay_079_F0000FA0_18CDF40(
    void *unused, Overlay79Vector *pos, Overlay79Vector *vel,
    f32 radius, Overlay79Plane *plane, Overlay79CollisionObject *object) {
    f32 nx;
    f32 ny;
    f32 nz;
    f32 dx;
    f32 d;
    f32 dy;
    f32 dz;
    f32 len;
    f32 delta;
    f32 u;
    f32 v;
    f32 w;
    f32 value;
    f32 angle;
    Overlay79CollisionState *state;

    nx = plane->normal.x;
    ny = plane->normal.y;
    nz = plane->normal.z;
    state = object->state;
    d = plane->constant;
    if ((0.707f <= ny) || (plane->flags & 0x10000000)) {
        u = vel->z * ny;
        v = -(vel->z * nx) + (nz * vel->x);
        w = -(vel->x * ny);
        dx = (v * nz) - (w * ny);
        dy = (w * nx) - (u * nz);
        dz = (u * ny) - (v * nx);
        len = (dx * dx) + (dy * dy) + (dz * dz);
        if (0.1f < len) {
            len = sqrtf(len);
            dx /= len;
            dy /= len;
            dz /= len;
            len = radius - plane->distance;
            pos->x = plane->origin.x + (len * dx);
            pos->y = plane->origin.y + (len * dy);
            pos->z = plane->origin.z + (len * dz);
        } else {
            pos->y = (-((pos->z * nz) + (nx * pos->x) + d) / ny) + 0.01f;
        }
        state->flags |= 2;
    } else {
        value = (pos->z * nz) + ((nx * pos->x) + (ny * pos->y)) + d;
        len = 0.01f - value;
        delta = len;
        u = pos->x + (len * nx);
        v = pos->y + (len * ny);
        w = pos->z + (len * nz);
        dx = pos->x - u;
        dy = pos->y - v;
        dz = pos->z - w;
        len = sqrtf((dx * dx) + (dz * dz));
        angle = func_8002A8BC((s16) Arctanf(dy, len));
        if (angle != 0.0f) {
            value = delta / angle;
            len = sqrtf((nx * nx) + (nz * nz));
            pos->x += value * (nx / len);
            pos->z += value * (nz / len);
        } else {
            pos->x = u;
            pos->y = v;
            pos->z = w;
        }
        state->flags |= 4;
    }
}
