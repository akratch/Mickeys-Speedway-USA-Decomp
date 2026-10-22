#include "PR/ultratypes.h"

typedef struct O57Ease28B4Point {
    u8 pad00[0x0C];
    s16 x;
    s16 y;
} O57Ease28B4Point;

typedef struct O57Ease28B4Transform {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} O57Ease28B4Transform;

typedef struct O57Ease28B4Primary {
    u8 pad00[8];
    O57Ease28B4Transform *source;
} O57Ease28B4Primary;

typedef struct O57Ease28B4Secondary {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[9];
    O57Ease28B4Transform *target;
} O57Ease28B4Secondary;

extern O57Ease28B4Point gO57Ease28B4Record21C;
extern O57Ease28B4Point gO57Ease28B4Record23C;
extern s32 gO57Ease28B4State144;
extern s32 gO57Ease28B4State118;
extern s32 gO57Ease28B4State180;
extern u8 gO57Ease28B4Id183;
extern s32 gO57Ease28B4Latch50C;
extern u32 D_800D31B8;

extern O57Ease28B4Primary *func_800508B4(u8 id);
extern void amSndPlay(s32 command, s32 argument);
extern O57Ease28B4Primary *animseqStopPath(u8 id);
extern O57Ease28B4Secondary *o57Ease28B4LookupSecondaryReloc(s32 id);
extern void func_8005055C(s32 id);
extern void animseqStartPath(s32 id);
extern void mainTitlePageInit(s32 argument);

/* Overlay 57 text +0x28B4..+0x2C28. Exact: 221 words, frame 0x28, 36/36
 * relocation identities.
 *
 * The plateau here was an s32 local per halfword field. Each one is its own
 * uopt web, and the two of them moved the record pointer's colour off v0 and
 * rotated the block-local temp ring behind it -- 86 of the 92 masked words
 * were that one substitution and its knock-on. Reading and writing the s16
 * fields in place removes the webs entirely; the horizontal statement is
 * written before the vertical one because the pair's load order follows it. */
void overlay57EaseAndLatch(s32 updateRate) {
    O57Ease28B4Primary *primary;
    O57Ease28B4Secondary *secondary;
    O57Ease28B4Transform *target;
    u32 flags;
    s32 i;

    gO57Ease28B4State144 = 0;

    i = 0;
    if (updateRate > 0) {
        do {
            i++;
            gO57Ease28B4Record21C.x = (s16)(gO57Ease28B4Record21C.x + ((0x104 - gO57Ease28B4Record21C.x) >> 3));
            gO57Ease28B4Record21C.y = (s16)(gO57Ease28B4Record21C.y + ((0xBE - gO57Ease28B4Record21C.y) >> 3));
        } while (i != updateRate);
    }

    i = 0;
    if (updateRate > 0) {
        do {
            i++;
            gO57Ease28B4Record23C.x = (s16)(gO57Ease28B4Record23C.x + ((0x32 - gO57Ease28B4Record23C.x) >> 3));
            gO57Ease28B4Record23C.y = (s16)(gO57Ease28B4Record23C.y + ((0xBE - gO57Ease28B4Record23C.y) >> 3));
        } while (i != updateRate);
    }

    flags = D_800D31B8;
    if (((flags & 0x9000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        primary = func_800508B4(gO57Ease28B4Id183);
        if (primary == NULL) {
            return;
        }
        amSndPlay(0xC, 0);
        (void)animseqStopPath(gO57Ease28B4Id183);
        secondary = o57Ease28B4LookupSecondaryReloc(0x29);
        if (secondary != NULL) {
            target = secondary->target;
            target->x = primary->source->x;
            target->y = primary->source->y;
            target->z = primary->source->z;
            target->angleX = primary->source->angleX;
            target->angleY = primary->source->angleY;
            target->angleZ = primary->source->angleZ;
            func_8005055C(0x29);
            animseqStartPath(0x29);
            gO57Ease28B4State180 = 0x29;
            secondary->flags |= 2;
        }
        gO57Ease28B4Latch50C = 1;
        gO57Ease28B4State118 = 14;
        return;
    }

    if (((flags & 0x4000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        amSndPlay(0xD, 0);
        mainTitlePageInit(1);
        gO57Ease28B4Latch50C = 1;
    }
}
