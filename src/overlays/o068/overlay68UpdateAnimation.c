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
 * PLATEAU: L160 named keyframes base plus local index, generated subscripts.
 * Object takes t2 unforced. Exact 1424 size, frame 0x78, 15 relocations.
 * 214/356 positional words, first at +0x34. Residual is structural around
 * the duration loop. Break-arm keyframes reload and keyframeIndex store
 * are load-bearing for size. L109/L100 did not recover the missing words.
 *
 */
#ifdef NON_MATCHING
void overlay68UpdateAnimation(Overlay68Object *object, s32 updateRate) {
    s32 atStart;
    f32 tangentX;
    f32 tangentZ;
    Overlay68Keyframe *keyframes;
    Overlay68Keyframe *before;
    Overlay68Keyframe *after;
    Overlay68Keyframe *afterAfter;
    Overlay68ObjectState *state;
    Overlay68Animation *animation;
    s32 index;
    s16 angle;
    s32 direction;
    s32 opacity;
    s32 animationOpacity;

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
            state->opacity = animationOpacity;

            state->elapsed += updateRate;
            keyframes = animation->keyframes;
            index = state->keyframeIndex;
            while (state->elapsed >= (s32)keyframes[index].duration) {
                state->elapsed -= keyframes[index].duration;
                index++;
                if (index >= animation->keyframeCount) {
                    index = animation->keyframeCount - 1;
                    keyframes = animation->keyframes;
                    state->keyframeIndex = index;
                    state->elapsed = 0;
                    state->active = 0;
                    break;
                }
            }
            state->keyframeIndex = index;

            if (keyframes[index].duration != 0) {
                state->fraction = (f32)state->elapsed /
                    (f32)keyframes[index].duration;
            } else {
                state->fraction = 0.0f;
            }

            before = keyframes + index;
            after = keyframes + index;
            afterAfter = keyframes + index;
            if (index > 0) {
                before = keyframes + (index - 1);
            }
            atStart = index < 1;
            if (index < animation->keyframeCount - 1) {
                after = keyframes + (index + 1);
            }
            angle = index < animation->keyframeCount - 2
                ? (afterAfter = keyframes + (index + 2), before->red)
                : before->red;

            object->red = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, angle << 8, keyframes[index].red << 8,
                after->red << 8, afterAfter->red << 8, 1, 0, atStart);
            object->green = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, before->green << 8, keyframes[index].green << 8,
                after->green << 8, afterAfter->green << 8, 1, 0, atStart);
            object->blue = (s16)(s32)func_overlay_068_F0000650_18C77B0(
                state->fraction, before->blue << 8, keyframes[index].blue << 8,
                after->blue << 8, afterAfter->blue << 8, 1, 0, atStart);
            object->x = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->x, keyframes[index].x, after->x,
                afterAfter->x, 0, &tangentX, atStart);
            object->y = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->y, keyframes[index].y, after->y,
                afterAfter->y, 0, 0, atStart);
            object->z = func_overlay_068_F0000650_18C77B0(
                state->fraction, before->z, keyframes[index].z, after->z,
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
 * score: 214/356 words
 * frame: 0x78
 * relocations: 15
 * first-mismatch: +0x34
 * summary: L160 named keyframes base puts object in t2 unforced at size 0 frame 0x78. Remaining blocker is duration-loop structure.
 * PLATEAU-HANDOFF:overlay68UpdateAnimation:end
 */
