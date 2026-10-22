#include "PR/ultratypes.h"

typedef struct O57Tail2C28Smooth {
    u8 pad00[0xC];
    s16 x;
    s16 y;
} O57Tail2C28Smooth;

typedef struct O57Tail2C28Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} O57Tail2C28Position;

extern O57Tail2C28Smooth gO57Tail2C28Record21C;
extern O57Tail2C28Smooth gO57Tail2C28Record23C;
extern O57Tail2C28Position *gO57Tail2C28Reference14C;
extern f32 gO57Tail2C28DistanceLimit104;
extern s32 gO57Tail2C28QueryWord180;
extern s32 gO57Tail2C28Timer118;
extern s32 gO57Tail2C28Completion50C;

extern void camSetNo(s32 value);
extern O57Tail2C28Position *camGetPtr(void);
extern void func_800498FC(s32 type, u32 scaleBits, u32 valueBits,
                                 s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void *overlay41IsUnitScale(s32 value);
extern void mainChangeLevel(s32 type, s32 arg1, s32 arg2, s32 mode,
                                  s32 arg4, s32 arg5);
extern void overlay84Mark(void);

/* Overlay 57 text +0x2C28..+0x2F48. Exact: 200 words, frame 0x30, 24/24
 * relocation identities.
 *
 * Same two smoothing loops as overlay57EaseAndLatch and the same fix: no s32
 * locals, s16 fields read and written in place, horizontal before vertical. */
void overlay57SmoothAndCheckDistance(s32 smoothingSteps) {
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    O57Tail2C28Position *position;
    O57Tail2C28Position *reference;

    i = 0;
    if (smoothingSteps > 0) {
        do {
            i++;
            gO57Tail2C28Record21C.x = (s16)(gO57Tail2C28Record21C.x + ((0x17C - gO57Tail2C28Record21C.x) >> 3));
            gO57Tail2C28Record21C.y = (s16)(gO57Tail2C28Record21C.y + ((0xBE - gO57Tail2C28Record21C.y) >> 3));
        } while (i != smoothingSteps);
    }

    i = 0;
    if (smoothingSteps > 0) {
        do {
            i++;
            gO57Tail2C28Record23C.x = (s16)(gO57Tail2C28Record23C.x + ((-0x46 - gO57Tail2C28Record23C.x) >> 3));
            gO57Tail2C28Record23C.y = (s16)(gO57Tail2C28Record23C.y + ((0xBE - gO57Tail2C28Record23C.y) >> 3));
        } while (i != smoothingSteps);
    }

    camSetNo(0);
    position = camGetPtr();
    reference = gO57Tail2C28Reference14C;
    if (reference != NULL) {
        dx = reference->x - position->x;
        dy = reference->y - position->y;
        dz = reference->z - position->z;
        if (((dx * dx) + (dy * dy)) + (dz * dz) <
            gO57Tail2C28DistanceLimit104) {
            func_800498FC(4, 0x3EAE147B, 0xBF800000,
                                 0, 0, 0, 0);
        }
    }

    if (overlay41IsUnitScale(gO57Tail2C28QueryWord180) != NULL) {
        gO57Tail2C28Timer118 = 0x11;
        mainChangeLevel(0x1D, 0, 0, 9, 1, 0);
        gO57Tail2C28Completion50C = 1;
        overlay84Mark();
    }
}
