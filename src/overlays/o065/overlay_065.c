#include "overlays/overlay_065.h"


#include "PR/ultratypes.h"



extern O65Particle D_1908[150];
extern O65Color D_1C0[7];
extern O65Vec3f D_1D8[4];
extern void *D_208;
extern s32 D_20C;
extern s32 D_210;
extern f32 D_2970;
extern f32 D_2974;
extern f32 D_2978;
extern f32 D_297C;
extern O65Vertex *D_2980[];
extern O65Vertex *D_2988;
extern O65Triangle D_80000000[12];

extern void texDPTextureX(Gfx **, TextureFrameHeader *, s32, s32);
extern O65Camera *func_80021970(s32);
extern void camSetNo(s32);
extern void camSetView(Gfx **, Mtx **);
extern s32 mathRnd(s32, s32);
extern f32 func_8002A8BC(s32);
extern f32 func_8002A8C0(s32);
extern s32 func_8001398C(f32, f32, s32, f32 ***);
extern void pointListRPY(s32, s16 *, O65Vec3f *, O65Vec3f *);
extern void func_overlay_065_F0000C38_18C4EA0(Gfx **, Mtx **, s32);

/*
 * PROVENANCE: packet macros adapted from the Jet Force Gemini decompilation
 * (include/PR/gbi.h gDma1p, include/PR/mbi.h _SHIFTL, include/f3ddkr.h
 * gSPVertexJFG and gSPPolygon, include/PR/os_convert.h OS_PHYSICAL_TO_K0), a
 * permitted source under docs/CLEANROOM.md; same adaptation as overlay 58.
 */
#define O65_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O65_DMA1P(pkt, c, s, l, p) { Gfx *_g = (Gfx *)(pkt); _g->words.w0 = (O65_SHIFTL((c), 24, 8) | O65_SHIFTL((p), 16, 8) | O65_SHIFTL((l), 0, 16)); _g->words.w1 = (unsigned int)(s); }
#define O65_VERTEX(pkt, v, n, v0) O65_DMA1P(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O65_POLYGON(dl, ptr, numTris, texEnabled) { Gfx *_g = (Gfx *)(dl); _g->words.w0 = O65_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O65_SHIFTL(5, 24, 8) | O65_SHIFTL(((numTris)*16), 0, 16); _g->words.w1 = (unsigned int)(ptr); }
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
void overlay65UpdateParticles(Gfx **commandList, Mtx **cursorPtr,
                                        s32 ticks) {
    Gfx *commands;
    O65Vertex *batchStart;
    Mtx *cursor;
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
    const O65Color *colors;

    commands = *commandList;
    cursor = *cursorPtr;
    D_2988 = O65_BUFFER_TABLE[D_210];
    D_210 ^= 1;
    particle = D_1908;
    spawnCount = 4;
    remaining = 6;
    texDPTextureX(&commands, O65_INPUT, 3, 0);
    camera = func_80021970(0);
    camSetNo(0);
    camSetView(&commands, &cursor);
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
            particle->y = (s16)((f32)mathRnd(200, 250) + camera->y);
            lateralX = (f32)mathRnd(-500, 500);
            lateralZ = (f32)mathRnd(-500, 500);
            sinAngle = func_8002A8BC(camera->angle);
            cosAngle = func_8002A8C0(camera->angle);
            particle->x = (s16)((camera->x + lateralX * sinAngle) -
                                lateralZ * cosAngle);
            particle->z = (s16)(camera->z + lateralZ * sinAngle +
                                lateralX * cosAngle);
            particle->dx = mathRnd(-0x7FFF, 0x8000);
            particle->dy = mathRnd(-0x7FFF, 0x8000);
            particle->dz = mathRnd(-0x7FFF, 0x8000);
            particle->ddx = mathRnd(-0x300, 0x300);
            particle->ddy = mathRnd(-0x300, 0x300);
            particle->ddz = mathRnd(-0x300, 0x300);
            particle->angle = mathRnd(-0x7FFF, 0x8000);
            particle->angleStep = mathRnd(-0x400, 0x400);

            groundCount = func_8001398C((f32)particle->x, (f32)particle->z,
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

            colors = &D_1C0[mathRnd(0, 6)];
            particle->r = colors->red;
            particle->g = colors->green;
            particle->b = colors->blue;
            particle->active = 1;
            spawnCount--;
        }

        if (particle->active != 0) {
            radius = (s16)(func_8002A8C0(particle->angle) * 50.0f);
            pointListRPY(4, &particle->dx, D_1D8, transformed);
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



/* Generic double-release wrapper only; no exact DKR donor. */
extern TextureHeader *gOverlay65Resource208;
extern void *gOverlay65Resource2980;
extern void texFreeTexture(TextureHeader *texture);
extern void mmFree(void *data);

void overlay65Release(void) {
    texFreeTexture(gOverlay65Resource208);
    mmFree(gOverlay65Resource2980);
}





/* Tier B: the LOCAL records address the 50 trail active fields in BSS. */
void overlay65ResetSlots(void) {
    register s32 index;

    for (index = 0; index < 50; index++) {
        D_0[index].active = 0;
    }
    D_1900 = 1;
}







extern f32 D_214;
extern f32 D_2978;
extern f32 D_297C;
extern O65Vertex *D_2988;
extern O65Triangle D_800000C0[16];

/*
 * Tier B: Overlay 65 runtime SYMBOL records distinguish the camera getter,
 * active-camera setter, view setup, random range, sine, cosine and render-state
 * calls below; their zero-offset assembly carrier is not a shared callee.
 * The spawn call is a same-overlay JUMP to the adjacent record initializer.
 * LOCAL records bind D_1900, D_2978, D_297C and D_2988 to BSS, D_214 and
 * the triangle command address to data. The record pool starts at BSS zero.
 */







extern void func_overlay_065_F0001A14_18C5C7C(f32 x, f32 y, f32 z);

/*
 * Tier D (target CFG): render each trail active at entry, including its final
 * update below the floor. The spawn's last random call precedes camera Y/Z
 * loads; preserve that sequencing across the call.
 */
void func_overlay_065_F0000C38_18C4EA0(Gfx **commandPtr,
                                       Mtx **matrixPtr, s32 updateRate) {
    s32 recordIndex;
    s32 updateIndex;
    s32 pointIndex;
    f32 randomX;
    f32 randomZ;
    f32 sinAngle;
    f32 cosAngle;
    f32 spawnX;
    f32 spawnY;
    f32 spawnZ;
    O65Camera *camera;
    Overlay65TrailRecord *record;
    Gfx *commands;
    Mtx *matrices;

    commands = *commandPtr;
    matrices = *matrixPtr;
    camera = func_80021970(0);
    camSetNo(0);
    camSetView(&commands, &matrices);

    if (D_1900 > 0) {
        D_1900 -= updateRate;
    } else {
        randomX = (f32)mathRnd(-500, 500);
        randomZ = (f32)mathRnd(-500, 500);
        sinAngle = func_8002A8BC(camera->angle);
        cosAngle = func_8002A8C0(camera->angle);
        spawnX = (camera->x + (randomX * sinAngle)) -
                 (randomZ * cosAngle);
        spawnY = (f32)mathRnd(-100, -50) + camera->y;
        spawnZ = camera->z + (randomZ * sinAngle) +
                 (randomX * cosAngle);
        func_overlay_065_F0001A14_18C5C7C(
            spawnX, spawnY, spawnZ);
        D_1900 = 1;
    }

    texDPTextureX(&commands, 0, 1, 0);
    record = D_0;
    for (recordIndex = 0; recordIndex != 50; recordIndex++) {
        if (record->active != 0) {
            for (updateIndex = 0; updateIndex < updateRate; updateIndex++) {
                record->x[0] += record->velocityX;
                record->y[0] += record->velocityY;
                record->z[0] += record->velocityZ;
                for (pointIndex = 1; pointIndex < 9; pointIndex++) {
                    record->x[pointIndex] +=
                        (record->x[pointIndex - 1] -
                         record->x[pointIndex]) *
                        0.25f;
                    record->y[pointIndex] +=
                        (record->y[pointIndex - 1] -
                         record->y[pointIndex]) *
                        0.25f;
                    record->z[pointIndex] +=
                        (record->z[pointIndex - 1] -
                         record->z[pointIndex]) *
                        0.25f;
                }
                for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                    record->x[pointIndex] += D_2978;
                    record->z[pointIndex] += D_297C;
                }
                record->velocityY -= D_214;
                if (record->y[0] < record->floorY) {
                    record->active = 0;
                }
            }

            {
                u32 *packet = (u32 *)commands++; packet[0] = ((((((u32)D_2988 + 0x80000000U) & 6U) | 0x90U) & 0xFFU) << 16) | 0x04000000U | 0xBCU;
                packet[1] = (u32)D_2988 + 0x80000000U;
            }
            {
                u32 *packet = (u32 *)commands++;

                packet[1] = (u32)D_800000C0;
                packet[0] = 0x05F10100U;
            }

            for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                D_2988->x = record->x[pointIndex] - 3.0f;
                D_2988->y = record->y[pointIndex];
                D_2988->z = record->z[pointIndex] - 3.0f;
                D_2988->r = record->red;
                D_2988->g = record->green;
                D_2988->b = record->blue;
                D_2988->a = 0xFF;
                D_2988++;
                D_2988->x = record->x[pointIndex] + 3.0f;
                D_2988->y = record->y[pointIndex];
                D_2988->z = record->z[pointIndex] + 3.0f;
                D_2988->r = record->red;
                D_2988->g = record->green;
                D_2988->b = record->blue;
                D_2988->a = 0xFF;
                D_2988++;
            }
        }
        record++;
    }
    *commandPtr = commands;
    *matrixPtr = matrices;
}



#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))


extern O65Color D_1C0[7];
extern s32 D_218;
extern s32 D_21C;



void func_overlay_065_F0001A14_18C5C7C(f32 baseX, f32 baseY, f32 baseZ) {
    O65Camera *camera;
    Overlay65TrailRecord *record;
    s32 i;
    s32 j;
    void *point;
    const O65Color *colors;

    camera = func_80021970(0);
    record = (Overlay65TrailRecord *)D_0;
    for (i = 0; i < 50; i++, record++) {
        j = 0;
        point = record;
        if (record->active == 0) {
            record->active = 1;
            for (;;) {
                f32 value = (f32)j;
                j++;
                point = (u8 *)point + 4;
                FIELD(point, f32, -4) = value + baseX;
                FIELD(point, f32, 0x20) = value + baseY;
                FIELD(point, f32, 0x44) = value + baseZ;
                if (j < 9) {
                    continue;
                }
                break;
            }
            record->velocityX = (f32)mathRnd(-D_218, D_218);
            record->velocityY = (f32)mathRnd(D_21C, D_21C + 5);
            record->velocityZ = (f32)mathRnd(-D_218, D_218);
            colors = &D_1C0[mathRnd(0, 6)];
            record->red = colors->red;
            record->green = colors->green;
            record->blue = colors->blue;
            record->floorY = camera->y - 200.0f;
            i = 50;
        }
    }
}
