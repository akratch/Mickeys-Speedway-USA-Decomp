#include "PR/ultratypes.h"

typedef struct O65Particle {
    s16 x;
    s16 y;
    s16 z;
    s16 floorY;
    s16 dx;
    s16 dy;
    s16 dz;
    s16 ddx;
    s16 ddy;
    s16 ddz;
    s16 angle;
    s16 angleStep;
    u8 r;
    u8 g;
    u8 b;
    u8 active;
} O65Particle;

typedef struct O65Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O65Vertex;

typedef struct O65Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O65Vec3f;

typedef struct O65Camera {
    s16 angle;
    u8 pad02[10];
    f32 x;
    f32 y;
    f32 z;
} O65Camera;

typedef struct O65Command {
    u32 w0;
    u32 w1;
} O65Command;

extern O65Particle D_1908[150];
extern u8 D_1C0[];
extern O65Vec3f D_1D8[];
extern void *D_208;
extern s32 D_20C;
extern s32 D_210;
extern f32 D_2970;
extern f32 D_2974;
extern f32 D_2978;
extern f32 D_297C;
extern O65Vertex *D_2980[];
extern O65Vertex *D_2988;
extern u8 D_80000000[];

extern void o65BeginDraw(O65Command **, void *, s32, void *);
extern O65Camera *o65GetCamera(s32);
extern void o65PrepareCamera(s32);
extern void o65LoadCursor(O65Command **, s32 *);
extern s32 o65RandomRange(s32, s32);
extern f32 o65Sin(s16);
extern f32 o65Cos(s16);
extern s32 o65FindGround(f32, f32, s32, f32 ***);
extern void o65Transform(s32, s16 *, O65Vec3f *, O65Vec3f *);
extern void func_overlay_065_F0000C38_18C4EA0(O65Command **, s32 *, s32);

/*
 * PROVENANCE: packet macros adapted from the Jet Force Gemini decompilation
 * (include/PR/gbi.h gDma1p, include/PR/mbi.h _SHIFTL, include/f3ddkr.h
 * gSPVertexJFG and gSPPolygon, include/PR/os_convert.h OS_PHYSICAL_TO_K0), a
 * permitted source under docs/CLEANROOM.md; same adaptation as overlay 58.
 */
#define O65_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O65_DMA1P(pkt, c, s, l, p) { O65Command *_g = (O65Command *)(pkt); _g->w0 = (O65_SHIFTL((c), 24, 8) | O65_SHIFTL((p), 16, 8) | O65_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O65_VERTEX(pkt, v, n, v0) O65_DMA1P(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O65_POLYGON(dl, ptr, numTris, texEnabled) { O65Command *_g = (O65Command *)(dl); _g->w0 = O65_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O65_SHIFTL(5, 24, 8) | O65_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O65_K0(x) (void *)(((u32)(x)+0x80000000))

#define O65_MODE D_20C
#define O65_INPUT D_208
#define O65_CAMERA_X D_2970
#define O65_CAMERA_Z D_2974
#define O65_DELTA_X D_2978
#define O65_DELTA_Z D_297C
#define O65_BUFFER_TABLE D_2980

/*
 * Matched (2026-10-02). The four vertex writes are one counted loop over
 * the transformed points (uopt unrolls it, which is what made the vertex
 * block a depth-2 region and put radius and the point base in saved
 * registers); the batch flush and tail use the packet macros above; the
 * working set is one local per role with no carrier copies of constants.
 */
void overlay65UpdateParticles(O65Command **commandList, s32 *cursorPtr,
                                        s32 ticks) {
    O65Command *commands;
    O65Vertex *batchStart;
    s32 cursor;
    O65Camera *camera;
    O65Particle *particle;
    s32 groundCount;
    s32 groundIndex;
    s32 particleIndex;
    s32 spawnCount;
    s32 remaining;
    s16 radius;
    O65Vec3f transformed[4];
    f32 lateralX;
    f32 lateralZ;
    f32 sinAngle;
    f32 cosAngle;
    f32 **ground;
    u8 *colors;

    commands = *commandList;
    cursor = *cursorPtr;
    D_2988 = O65_BUFFER_TABLE[D_210];
    D_210 ^= 1;
    particle = D_1908;
    spawnCount = 4;
    remaining = 6;
    o65BeginDraw(&commands, O65_INPUT, 3, 0);
    camera = o65GetCamera(0);
    o65PrepareCamera(0);
    o65LoadCursor(&commands, &cursor);
    particleIndex = 0;
    batchStart = D_2988;

    if (O65_MODE != 0) {
        O65_DELTA_X = (camera->x - O65_CAMERA_X) * 0.75f;
        O65_DELTA_Z = (camera->z - O65_CAMERA_Z) * 0.75f;
    } else {
        O65_DELTA_X = 0.0f;
        O65_DELTA_Z = 0.0f;
    }
    O65_CAMERA_X = camera->x;
    O65_CAMERA_Z = camera->z;

    do {
        if (particle->active != 0) {
            particle->y -= ticks * 4;
            if (particle->y < particle->floorY) {
                particle->active = 0;
            } else {
                particle->dx += particle->ddx * ticks;
                particle->dy += particle->ddy * ticks;
                particle->dz += particle->ddz * ticks;
                particle->angle += particle->angleStep * ticks;
                particle->x = (s16)((f32)particle->x + O65_DELTA_X);
                particle->z = (s16)((f32)particle->z + O65_DELTA_Z);
            }
        }

        if ((spawnCount != 0) && (particle->active == 0)) {
            particle->y = (s16)((f32)o65RandomRange(200, 250) + camera->y);
            lateralX = (f32)o65RandomRange(-500, 500);
            lateralZ = (f32)o65RandomRange(-500, 500);
            sinAngle = o65Sin(camera->angle);
            cosAngle = o65Cos(camera->angle);
            particle->x = (s16)((camera->x + lateralX * sinAngle) -
                                lateralZ * cosAngle);
            particle->z = (s16)(camera->z + lateralZ * sinAngle +
                                lateralX * cosAngle);
            particle->dx = o65RandomRange(-0x7FFF, 0x8000);
            particle->dy = o65RandomRange(-0x7FFF, 0x8000);
            particle->dz = o65RandomRange(-0x7FFF, 0x8000);
            particle->ddx = o65RandomRange(-0x300, 0x300);
            particle->ddy = o65RandomRange(-0x300, 0x300);
            particle->ddz = o65RandomRange(-0x300, 0x300);
            particle->angle = o65RandomRange(-0x7FFF, 0x8000);
            particle->angleStep = o65RandomRange(-0x400, 0x400);

            groundCount = o65FindGround((f32)particle->x, (f32)particle->z,
                                        0x1000, &ground);
            if (groundCount != 0) {
                particle->floorY = particle->y - 10000;
                for (groundIndex = 0; groundIndex < groundCount; groundIndex++) {
                    if ((*ground[groundIndex] < (f32)particle->y) &&
                        ((f32)particle->floorY < *ground[groundIndex])) {
                        particle->floorY = (s16)*ground[groundIndex];
                    }
                }
            } else {
                particle->floorY = (s16)(camera->y - 300.0f);
            }

            colors = &D_1C0[o65RandomRange(0, 6) * 3];
            particle->r = colors[0];
            particle->g = colors[1];
            particle->b = colors[2];
            particle->active = 1;
            spawnCount--;
        }

        if (particle->active != 0) {
            radius = (s16)(o65Cos(particle->angle) * 50.0f);
            o65Transform(4, &particle->dx, D_1D8, transformed);
            remaining--;
            groundIndex = 0;
            for (groundCount = 4; groundCount != 0; groundCount--) {
                D_2988->x = (s16)(((O65Vec3f *)((s32 *)transformed + groundIndex))->x + (f32)(particle->x + radius));
                D_2988->y = (s16)(((O65Vec3f *)((s32 *)transformed + groundIndex))->y + (f32)particle->y);
                D_2988->z = (s16)(((O65Vec3f *)((s32 *)transformed + groundIndex))->z + (f32)(particle->z + radius));
                D_2988->r = particle->r;
                D_2988->g = particle->g;
                D_2988->b = particle->b;
                D_2988->a = 0xFF;
                D_2988++;
                groundIndex += 3;
            }

            if (remaining == 0) {
                O65_VERTEX(commands++, O65_K0(batchStart), 24, 0);
                O65_POLYGON(commands++, D_80000000, 12, 1);
                remaining = 6;
                batchStart = D_2988;
            }
        }
        particleIndex++;
        particle++;
    } while (particleIndex != 150);

    if (remaining != 6) {
        O65_VERTEX(commands++, O65_K0(batchStart), (6 - remaining) * 4, 0);
        O65_POLYGON(commands++, D_80000000, (6 - remaining) * 2, 1);
    }

    *commandList = commands;
    *cursorPtr = cursor;
    func_overlay_065_F0000C38_18C4EA0(commandList, cursorPtr, ticks);
    O65_MODE = 1;
}
