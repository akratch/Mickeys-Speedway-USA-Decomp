#include "PR/ultratypes.h"

typedef struct Overlay17Transform {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay17Transform;

typedef struct Overlay17ChainHead {
    s16 count;
    u8 selectedBuffer;
    u8 dirty;
    Overlay17Transform *material;
    f32 x;
    f32 y;
    f32 z;
    f32 oldX;
    f32 oldY;
    f32 oldZ;
    f32 radius;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    Overlay17Transform *transform;
} Overlay17ChainHead;

extern void overlay17TransformReloc(
    s32 mode, Overlay17Transform *transform, f32 *source, f32 *destination);
extern f32 overlay17SqrtReloc(f32 value);

/* Plateau (2026-08-25): exact-size 0x318, 133 words differ from +0x24;
 * reverse pointer fill closed the frame from 0x60 to the target's 0x58.
 * The flag lattice was neutral; the 40-minute permuter reached score 3035. */
#ifdef NON_MATCHING
void overlay17CalculateEndpoints(
    Overlay17ChainHead *chain, f32 *outX0, f32 *outY0, f32 *outZ0,
    f32 *outX1, f32 *outY1, f32 *outZ1) {
    s32 index;
    f32 deltaZ;
    f32 scale;
    volatile f32 lengthSquared;
    f32 points[6];
    Overlay17Transform *transform;
    f32 *point;
    f32 deltaX;

    if (chain != 0) {
        transform = chain->transform;
        if (transform != 0) {
            points[0] = chain->x * transform->scale;
            points[1] = chain->y * transform->scale;
            points[2] = chain->z * transform->scale;
            overlay17TransformReloc(1, transform, points, points);
            points[0] += transform->x;
            points[1] += transform->y;
            points[2] += transform->z;

            if (chain->dirty == 0) {
                deltaX = points[0] - chain->oldX;
                deltaZ = points[2] - chain->oldZ;
            } else {
                deltaX = 0.0f;
                deltaZ = 0.0f;
            }
            lengthSquared = (deltaX * deltaX) + (deltaZ * deltaZ);
            scale = lengthSquared;
            if (lengthSquared > 0.0f) {
                scale = (chain->radius * transform->scale) /
                        overlay17SqrtReloc(lengthSquared);
            }
            deltaX *= scale;
            deltaZ *= scale;
            chain->oldX = points[0];
            chain->oldY = points[1];
            chain->dirty = 0;
            chain->oldZ = points[2];
            points[3] = points[0] + deltaZ;
            points[0] -= deltaZ;
            points[4] = points[1];
            points[5] = points[2] - deltaX;
            points[2] += deltaX;
        } else {
            points[0] = chain->x - chain->radius;
            points[1] = chain->y;
            points[2] = chain->z;
            points[3] = chain->x + chain->radius;
            points[4] = chain->y;
            points[5] = chain->z;
            if (transform != 0) {
                overlay17TransformReloc(2, transform, points, points);
                point = points;
                do {
                    point[0] = (point[0] * transform->scale) + transform->x;
                    point[1] = (point[1] * transform->scale) + transform->y;
                    point[2] = (point[2] * transform->scale) + transform->z;
                } while ((point += 3) < &points[6]);
            }
        }
    } else {
        point = &points[5];
        index = 5;
        do {
            *point-- = 0.0f;
        } while (index--);
    }

    *outX0 = points[0];
    *outY0 = points[1];
    *outZ0 = points[2];
    *outX1 = points[3];
    *outY1 = points[4];
    *outZ1 = points[5];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o017/overlay17CalculateEndpoints/func_overlay_017_F0000000_18739B8.s")
#endif

/* PLATEAU-HANDOFF:overlay17CalculateEndpoints:start
 * symbol: overlay17CalculateEndpoints
 * score: 121 differing words
 * frame: 0x58
 * relocations: 3
 * first-mismatch: +0x24
 * summary: Declaration order moves points to 0x30: 133 to 121. Volatile lengthSquared reloads where the target stores once and keeps f18 live.
 * PLATEAU-HANDOFF:overlay17CalculateEndpoints:end
 */
