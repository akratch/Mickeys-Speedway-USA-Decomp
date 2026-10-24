#include "PR/ultratypes.h"

/* Tier B: texAnimateSprite reads and updates signed flags and float frames.
 * These three independent channels start enabled, ping-ponging and looping. */
enum {
    O57_ANIMATION_ENABLED = 1,
    O57_ANIMATION_PING_PONG = 4,
    O57_ANIMATION_LOOP = 8
};

s32 o57TitleAnimationFlags = O57_ANIMATION_ENABLED |
    O57_ANIMATION_PING_PONG | O57_ANIMATION_LOOP;
s32 o57GridAnimationFlags = O57_ANIMATION_ENABLED |
    O57_ANIMATION_PING_PONG | O57_ANIMATION_LOOP;
s32 o57PanelAnimationFlags = O57_ANIMATION_ENABLED |
    O57_ANIMATION_PING_PONG | O57_ANIMATION_LOOP;

f32 o57TitleTextureFrame = 0.0f;
f32 o57PanelTextureFrame = 0.0f;
f32 o57GridTextureFrame = 0.0f;

/* The selection update treats 255 as the absent-selection value. */
s32 o57PrimarySelection = 255;
s32 o57SecondarySelection = 255;
