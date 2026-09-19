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

#define O65_MODE D_20C
#define O65_INPUT D_208
#define O65_CAMERA_X D_2970
#define O65_CAMERA_Z D_2974
#define O65_DELTA_X D_2978
#define O65_DELTA_Z D_297C
#define O65_BUFFER_TABLE D_2980

/*
 * Size closed at 720 words / frame 0xF8: load *arg1 while a1 is live, then
 * write transformed points through a word-index cursor ((s32 *)point + i,
 * i += 3) reused on groundIndex. Colour cannot close the remaining residual.
 */
#ifdef NON_MATCHING
void overlay65UpdateParticles(O65Command **arg0, s32 *arg1,
                                        s32 arg2) {
    O65Command *commands;
    O65Vertex *batchStart;
    s32 cursor;
    volatile s32 spawnCount;
    O65Vec3f transformed[4];
    f32 **ground;
    O65Camera *camera;
    O65Particle *particle;
    s32 groundCount;
    s32 groundIndex;
    s32 particleIndex;
    s32 remaining;
    s16 radius;
    f32 lateralX;
    f32 lateralZ;
    f32 sinAngle;
    f32 cosAngle;
    u8 *colors;
    register u8 alpha;
    register O65Vec3f *point;

    commands = *arg0;
    cursor = *arg1;
    particle = D_1908;
    spawnCount = 4;
    remaining = 6;
    D_2988 = O65_BUFFER_TABLE[D_210];
    D_210 ^= 1;
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
    point = transformed;
    alpha = 0xFF;

    do {
        if (particle->active != 0) {
            particle->y -= arg2 * 4;
            if (particle->y < particle->floorY) {
                particle->active = 0;
            } else {
                particle->dx += particle->ddx * arg2;
                particle->dy += particle->ddy * arg2;
                particle->dz += particle->ddz * arg2;
                particle->angle += particle->angleStep * arg2;
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
            particle->active = 1;
            particle->b = colors[2];
            spawnCount--;
        }

        if (particle->active != 0) {
            radius = (s16)(o65Cos(particle->angle) * 50.0f);
            o65Transform(4, &particle->dx, D_1D8, point);
            remaining--;
            groundIndex = 3;
#define O65_WRITE_VTX(vtx) \
                D_2988->x = (s16)((vtx)->x + (f32)(particle->x + radius)); \
                D_2988->y = (s16)((vtx)->y + (f32)particle->y); \
                D_2988->z = (s16)((vtx)->z + (f32)(particle->z + radius)); \
                D_2988->r = particle->r; \
                D_2988->g = particle->g; \
                D_2988->b = particle->b; \
                D_2988->a = alpha; \
                D_2988++
            O65_WRITE_VTX(point);
            O65_WRITE_VTX((O65Vec3f *)((s32 *)point + groundIndex));
            groundIndex += 3;
            O65_WRITE_VTX((O65Vec3f *)((s32 *)point + groundIndex));
            groundIndex += 3;
            O65_WRITE_VTX((O65Vec3f *)((s32 *)point + groundIndex));
#undef O65_WRITE_VTX

            if (remaining == 0) {
                commands->w0 = 0x040000F8U |
                    (((((u32)batchStart + 0x80000000U) & 6U) | 0xC0U) << 16);
                commands->w1 = (u32)batchStart + 0x80000000U;
                commands++;
                commands->w0 = 0x05B100C0U;
                commands->w1 = (u32)&D_80000000;
                commands++;
                remaining = 6;
                batchStart = D_2988;
            }
        }
        particleIndex++;
        particle++;
    } while (particleIndex != 150);

    if (remaining != 6) {
        s32 used = 6 - remaining;
        s32 vertexCount = used * 4;
        commands->w0 = 0x04000000U |
            (((vertexCount * 8 | (((u32)batchStart + 0x80000000U) & 6U)) & 0xFFU) << 16) |
            ((used * 40 + 8) & 0xFFFF);
        commands->w1 = (u32)batchStart + 0x80000000U;
        commands++;
        commands->w0 = 0x05000000U |
            (((((used * 2 - 1) * 16) | 1) & 0xFFU) << 16) |
            ((used * 2 * 16) & 0xFFFF);
        commands->w1 = (u32)&D_80000000;
        commands++;
    }

    *arg0 = commands;
    *arg1 = cursor;
    func_overlay_065_F0000C38_18C4EA0(arg0, arg1, arg2);
    O65_MODE = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o065/overlay65UpdateParticles/func_overlay_065_F0000080_18C42E8.s")
#endif

/* PLATEAU-HANDOFF:overlay65UpdateParticles:start
 * symbol: overlay65UpdateParticles
 * score: 458/720 words
 * frame: 0xF8
 * relocations: 64
 * first-mismatch: +0x50
 * summary: Size closed 0 at frame 0xF8. Colour 267 probes floor 395 on one c8 radius. Cursor still one-behind. overlay65Initialize identities untouched.
 * PLATEAU-HANDOFF:overlay65UpdateParticles:end
 */
