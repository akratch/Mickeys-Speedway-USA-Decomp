#include "PR/ultratypes.h"

typedef struct Overlay74UpdateState {
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74UpdateState;

typedef struct Overlay74UpdateObject {
    s16 angle;
    u8 pad02[4];
    s16 flags;
    u8 pad08[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay74UpdateState *state;
} Overlay74UpdateObject;

typedef struct Overlay74HitState {
    s8 strength;
} Overlay74HitState;

typedef struct Overlay74HitObject {
    u8 pad00[0x10];
    f32 y;
    u8 pad14[0x50];
    Overlay74HitState *state;
} Overlay74HitObject;

/* Bits 26..23 of the word (bits 10..7 of its leading halfword) are a 4-bit
 * channel mask. The target reads it through the 32-bit word and writes it
 * through a halfword, which is what IDO emits for a u16 bitfield. */
typedef struct Overlay74Flags {
    u16 pad : 5;
    u16 channelMask : 4;
    u16 rest : 7;
} Overlay74Flags;

extern Overlay74Flags D_800D3128;

/* Runtime identities: func_8005776C, func_800291B4, amSndPlay, func_8003A680.
 * Resident ROM and matched callers authenticate the floating-point z ABI. */
s32 func_8005776C(f32 x, f32 y, f32 z, f32 strength, s32 enabled,
                        Overlay74HitObject **results);
void func_800291B4(void);
void amSndPlay(u16 soundId, void **handle);
void func_8003A680(s32 count);

void overlay74Update(Overlay74UpdateObject *object, s32 amount) {
    Overlay74HitObject *results[13];
    f32 delta;
    Overlay74UpdateState *state;
    Overlay74HitObject *hitObject;

    if (!(object->flags & 0x400)) {
        object->angle += amount << 8;
        state = object->state;
        if (func_8005776C(object->x, object->y, object->z,
                                (f32)state->strength, 1, results) != 0) {
            hitObject = results[0];
            amount = (s32)hitObject->state;
            delta = hitObject->y - object->y;
            if ((((Overlay74HitState *)amount)->strength == 0) &&
                ((f32)state->minimum < delta) &&
                (delta < (f32)state->maximum)) {
                object->flags |= 0x400;
                D_800D3128.channelMask |= 1 << state->channel;
                func_800291B4();
                amSndPlay(0x27C, 0);

                object = (Overlay74UpdateObject *)5;
                state = (Overlay74UpdateState *)D_800D3128.channelMask;
                hitObject = (Overlay74HitObject *)8;
                do {
                    if ((s32)state & (s32)hitObject) {
                        object = (Overlay74UpdateObject *)((s32)object + 1);
                    }
                    hitObject = (Overlay74HitObject *)((s32)hitObject >> 1);
                } while (hitObject != NULL);
                if ((s32)object >= 6) {
                    func_8003A680((s32)object);
                }
            }
        }
    }
}
