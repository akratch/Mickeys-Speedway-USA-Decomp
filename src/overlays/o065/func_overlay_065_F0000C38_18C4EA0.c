#include "PR/ultratypes.h"
#include "n_audio/gbi.h"

typedef struct Overlay65TrailRecord {
    f32 x[9];
    f32 y[9];
    f32 z[9];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 floorY;
    u8 active;
    u8 red;
    u8 green;
    u8 blue;
} Overlay65TrailRecord;

typedef struct Overlay65TrailVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay65TrailVertex;

typedef struct Overlay65TrailCamera {
    s16 angle;
    u8 pad02[10];
    f32 x;
    f32 y;
    f32 z;
} Overlay65TrailCamera;

#define O65_RECORD(base) ((Overlay65TrailRecord *)(base))

extern u8 D_0[];
extern s16 D_1900;
extern f32 D_214;
extern f32 D_2978;
extern f32 D_297C;
extern Overlay65TrailVertex *D_2988;
extern u8 D_800000C0[];

/*
 * Tier B: Overlay 65 runtime SYMBOL records distinguish the camera getter,
 * active-camera setter, view setup, random range, sine, cosine and render-state
 * calls below; their zero-offset assembly carrier is not a shared callee.
 * The spawn call is a same-overlay JUMP to the adjacent record initializer.
 * LOCAL records bind D_1900, D_2978, D_297C and D_2988 to BSS, D_214 and
 * the triangle command address to data. The record pool starts at BSS zero.
 */
extern Overlay65TrailCamera *func_80021970(s32 index);
extern void camSetNo(s32 index);
extern void camSetView(Gfx **commands, Mtx **matrices);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void texDPTextureX(Gfx **commands, s32 texture,
                          s32 flags, s32 parameter);
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
    Overlay65TrailCamera *camera;
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
    record = O65_RECORD(D_0);
    for (recordIndex = 0; recordIndex != 50; recordIndex++) {
        if (O65_RECORD(record)->active != 0) {
            for (updateIndex = 0; updateIndex < updateRate; updateIndex++) {
                O65_RECORD(record)->x[0] += O65_RECORD(record)->velocityX;
                O65_RECORD(record)->y[0] += O65_RECORD(record)->velocityY;
                O65_RECORD(record)->z[0] += O65_RECORD(record)->velocityZ;
                for (pointIndex = 1; pointIndex < 9; pointIndex++) {
                    O65_RECORD(record)->x[pointIndex] +=
                        (O65_RECORD(record)->x[pointIndex - 1] -
                         O65_RECORD(record)->x[pointIndex]) *
                        0.25f;
                    O65_RECORD(record)->y[pointIndex] +=
                        (O65_RECORD(record)->y[pointIndex - 1] -
                         O65_RECORD(record)->y[pointIndex]) *
                        0.25f;
                    O65_RECORD(record)->z[pointIndex] +=
                        (O65_RECORD(record)->z[pointIndex - 1] -
                         O65_RECORD(record)->z[pointIndex]) *
                        0.25f;
                }
                for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                    O65_RECORD(record)->x[pointIndex] += D_2978;
                    O65_RECORD(record)->z[pointIndex] += D_297C;
                }
                O65_RECORD(record)->velocityY -= D_214;
                if (O65_RECORD(record)->y[0] < O65_RECORD(record)->floorY) {
                    O65_RECORD(record)->active = 0;
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
                D_2988->x = O65_RECORD(record)->x[pointIndex] - 3.0f;
                D_2988->y = O65_RECORD(record)->y[pointIndex];
                D_2988->z = O65_RECORD(record)->z[pointIndex] - 3.0f;
                D_2988->red = O65_RECORD(record)->red;
                D_2988->green = O65_RECORD(record)->green;
                D_2988->blue = O65_RECORD(record)->blue;
                D_2988->alpha = 0xFF;
                D_2988++;
                D_2988->x = O65_RECORD(record)->x[pointIndex] + 3.0f;
                D_2988->y = O65_RECORD(record)->y[pointIndex];
                D_2988->z = O65_RECORD(record)->z[pointIndex] + 3.0f;
                D_2988->red = O65_RECORD(record)->red;
                D_2988->green = O65_RECORD(record)->green;
                D_2988->blue = O65_RECORD(record)->blue;
                D_2988->alpha = 0xFF;
                D_2988++;
            }
        }
        record++;
    }
    *commandPtr = commands;
    *matrixPtr = matrices;
}
