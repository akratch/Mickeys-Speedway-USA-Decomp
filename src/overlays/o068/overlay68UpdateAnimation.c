typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

typedef struct Overlay68Keyframe {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 duration;
} Overlay68Keyframe;

typedef struct Overlay68Animation {
    u8 pad00[8];
    s16 endAngle;
    s16 keyframeCount;
    Overlay68Keyframe *keyframes;
} Overlay68Animation;

typedef struct Overlay68ObjectState {
    Overlay68Animation *animation;
    f32 fraction;
    s16 keyframeIndex;
    s16 elapsed;
    s16 angle;
    u8 active;
    u8 opacity;
} Overlay68ObjectState;

typedef struct Overlay68Object {
    s16 red;
    s16 green;
    s16 blue;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 facingAngle;
    u8 pad30[0x0B];
    s8 direction;
    u8 pad3C[0x28];
    Overlay68ObjectState *state;
} Overlay68Object;

extern f32 func_overlay_068_F0000650_18C77B0(
    f32 fraction, s32 before, s32 current, s32 after, s32 afterAfter,
    s32 colorMode, f32 *tangentOut, s32 atStart);

/* The split assembly normalizes these five resident roles to one opaque call
 * symbol.  Distinct proxies preserve their independently proved physical
 * signatures while source shape is iterated. */
extern s16 overlay68Angle3Reloc(f32 x, f32 y, f32 z);
extern s16 overlay68Angle2Reloc(f32 x, f32 z);
extern s16 overlay68AngleDifferenceReloc(s32 from, s32 to);
extern void overlay68SetDirectionReloc(Overlay68Object *object, s32 direction,
    s32 arg2, f32 arg3);
extern void overlay68AdvanceObjectReloc(Overlay68Object *object, f32 scale,
    f32 updateRate);

/* The shipped overlay relocation table proves both loads resolve through its
 * reserved-BSS entry to resident D_800C947C.  This proxy remains zero-linked
 * in the overlay object; the retained runtime relocation table is authoritative. */
extern s32 gOverlay68GlobalFlagReloc;
#define OVERLAY68_GLOBAL_FLAG gOverlay68GlobalFlagReloc

/*
 * 2026-10-02 (lane x-sort), 180 -> 108 at size 0: the duration loop carries
 * the keyframe index in `index` (stored as index + 1, re-read into index in
 * the exit test), which makes one web of the loop index and the neighbour
 * index and gives state t2, current t1, index t0, animation a2 and the
 * stride a3 as the target has them.  The declarations follow the target's
 * home map (atStart 0x6C, tangents 0x60/0x5C, current 0x58, before 0x54,
 * after 0x4C, afterAfter 0x48, state 0x40) with two unused cells at the
 * bottom for the 0x78 frame.
 */
#ifdef NON_MATCHING
void overlay68UpdateAnimation(Overlay68Object *object, s32 updateRate) {
    s32 direction;
    s32 animationOpacity;
    s32 atStart;
    s16 angle;
    s32 opacity;
    f32 tangentX;
    f32 tangentZ;
    Overlay68Keyframe *current;
    Overlay68Keyframe *before;
    s32 index;
    Overlay68Keyframe *after;
    Overlay68Keyframe *afterAfter;
    Overlay68Animation *animation;
    Overlay68ObjectState *state;
    s32 unused1;
    s32 unused2;

    state = object->state;
    if (OVERLAY68_GLOBAL_FLAG != 0) {
        updateRate = 0;
    }

    if (state->active != 0) {
        animation = state->animation;
        if (animation != 0) {
            state->angle += updateRate;
            if (state->angle >= 0x3841) {
                state->angle = 0x3840;
            }

            if (state->angle >= 0x37C1) {
                opacity = 0x80 - ((state->angle - 0x37C0) << 1);
            } else {
                opacity = 0x80;
            }
            if (animation->endAngle < state->angle) {
                animationOpacity = 0x80 -
                    ((state->angle - animation->endAngle) << 1);
            } else {
                animationOpacity = 0x80;
            }
            if (opacity < animationOpacity) {
                animationOpacity = opacity;
            }
            if (animationOpacity < 0) {
                animationOpacity = 0;
            }
            state->opacity = animationOpacity & 0xFF;

            state->elapsed += updateRate;
            index = state->keyframeIndex;
            current = &animation->keyframes[index];
            while (state->elapsed >= current->duration) {
                state->elapsed -= current->duration;
                state->keyframeIndex = index + 1;
                current++;
                if ((index = state->keyframeIndex) >= animation->keyframeCount) {
                    state->keyframeIndex = animation->keyframeCount - 1;
                    state->elapsed = 0;
                    state->active = 0;
                    current = &animation->keyframes[state->keyframeIndex];
                    break;
                }
            }

            if (current->duration) {
                state->fraction = (f32)state->elapsed / (f32)current->duration;
            } else {
                state->fraction = 0.0f;
            }

            index = state->keyframeIndex;
            before = current;
            after = current;
            afterAfter = current;
            if (index > 0) {
                before = current - 1;
            }
            atStart = index < 1;
            if (index < animation->keyframeCount - 1) {
                after = current + 1;
            }
            angle = index < animation->keyframeCount - 2
                ? (afterAfter = current + 2, before->red)
                : before->red;

            object->red = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, angle << 8, current->red << 8,
                after->red << 8, afterAfter->red << 8, 1, 0, atStart);
            object->green = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, before->green << 8, current->green << 8,
                after->green << 8, afterAfter->green << 8, 1, 0, atStart);
            object->blue = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, before->blue << 8, current->blue << 8,
                after->blue << 8, afterAfter->blue << 8, 1, 0, atStart);
            object->x = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->x, current->x, after->x,
                afterAfter->x, 0, &tangentX, atStart);
            object->y = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->y, current->y, after->y,
                afterAfter->y, 0, 0, atStart);
            object->z = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->z, current->z, after->z,
                afterAfter->z, 0, &tangentZ, atStart);

            object->facingAngle = overlay68Angle3Reloc(object->x, object->y,
                object->z);
            if ((tangentX == 0.0f) && (tangentZ == 0.0f)) {
                angle = 0;
            } else {
                angle = overlay68Angle2Reloc(tangentX, tangentZ);
                angle = overlay68AngleDifferenceReloc(object->red,
                    angle - 0x8000);
            }

            if (OVERLAY68_GLOBAL_FLAG != 0) {
                direction = 0;
            } else if (angle < -0x2000) {
                direction = 7;
            } else if (angle < -0x800) {
                direction = 5;
            } else if (angle < -0x100) {
                direction = 3;
            } else if (angle >= 0x2001) {
                direction = 6;
            } else if (angle >= 0x801) {
                direction = 4;
            } else if (angle >= 0x101) {
                direction = 2;
            } else {
                direction = 1;
            }

            if (direction != object->direction) {
                overlay68SetDirectionReloc(object, direction, -1, 0.0f);
            }
            overlay68AdvanceObjectReloc(object, 0.04f, (f32)updateRate);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o068/overlay68UpdateAnimation/func_overlay_068_F000096C_18C7ACC.s")
#endif

/* PLATEAU-HANDOFF:overlay68UpdateAnimation:start
 * symbol: overlay68UpdateAnimation
 * score: 108/356 words
 * frame: 0x78
 * relocations: 15
 * first-mismatch: +0xD4
 * summary: Loop index carried in a local and target home order: state t2, index t0. Left: a one-draw ring shift at +0xD4 and the neighbour pointers' colours.
 * PLATEAU-HANDOFF:overlay68UpdateAnimation:end
 */
