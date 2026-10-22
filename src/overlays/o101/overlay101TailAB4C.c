#include "PR/ultratypes.h"

typedef struct O101TailAB4CRoot {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101TailAB4CRoot;

typedef struct O101TailAB4CNode32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailAB4CNode32;

typedef struct O101TailAB4CNode24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailAB4CNode24;

typedef struct O101TailAB4CInputs {
    u8 pad000[0x158];
    void *data158;
    u8 *text15C;
    u8 *text160;
    u8 *text164;
    u8 *text168;
    u8 *text16C;
    u8 *text170;
    u8 *text174;
    u8 *text178;
    u8 *text17C;
    u8 *text180;
} O101TailAB4CInputs;

extern O101TailAB4CRoot D_0;
extern s32 gO101TailAB4COrderCount;
extern void *gO101TailAB4COrderSlots[];
extern s32 gO101TailAB4CNode32Count;
extern O101TailAB4CNode32 D_340[];
extern s32 gO101TailAB4CNode24Count;
extern O101TailAB4CNode24 D_540[];
extern O101TailAB4CInputs gO101TailAB4CInputs;
extern u8 gO101TailAB4CAssetD90;
extern u8 gO101TailAB4CFinalObject3F14;

/* All three sprite nodes are created by the same resident routine (one
 * SYMBOL target in the relocation table), and it takes two arguments: the
 * third register the shipped calls carry is a leftover, not an argument. */
extern void *texLoadSprite(s32 key, void *source);
extern void overlay101Reset(void *object);
extern s32 overlay101ByteLength(u8 *text);

/* Matched 2026-10-02 (lane q-ovl10), 499 -> 0 at delta 0. Over the
 * one-name counter-subscript shape of lanes p10/w13 (L145/L131), three
 * edits: the creator called with its real two arguments (499 -> 100, the
 * old three-argument image was a leftover a2), value14 = 0.0f on all three
 * sprite nodes as ONE pool web with only the scales split by spelling
 * (L103; 100 -> 13), and the line order kind before width2E in the root
 * header and value14 before value18 in nodes 2 and 3 (13 -> 0). */
void func_overlay_101_F000AB4C_18E636C(void) {
    void *handle;
    u8 length;

    D_0.height30 = 0xF0; D_0.kind = 4; D_0.width2E = 0x140; D_0.asset34 = &gO101TailAB4CAssetD90; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &D_0.chainType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    D_340[gO101TailAB4CNode32Count].x = 0xF0; D_340[gO101TailAB4CNode32Count].y = 0x154; D_340[gO101TailAB4CNode32Count].value10 = 0; D_340[gO101TailAB4CNode32Count].color12 = 0xFF; D_340[gO101TailAB4CNode32Count].color13 = 0; D_340[gO101TailAB4CNode32Count].value18 = 0; D_340[gO101TailAB4CNode32Count].scale = 1.0f; D_340[gO101TailAB4CNode32Count].value14 = 0.0f; handle = texLoadSprite(0x91, NULL); D_340[gO101TailAB4CNode32Count].previousType = D_0.chainType; D_340[gO101TailAB4CNode32Count].previous = D_0.chain; D_340[gO101TailAB4CNode32Count].handle = handle; D_0.chainType = 2; D_0.chain = &D_340[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    D_340[gO101TailAB4CNode32Count].x = 0x8C; D_340[gO101TailAB4CNode32Count].y = 0x88; D_340[gO101TailAB4CNode32Count].value10 = 0; D_340[gO101TailAB4CNode32Count].color12 = 0; D_340[gO101TailAB4CNode32Count].color13 = 0; D_340[gO101TailAB4CNode32Count].value14 = 0.0f; D_340[gO101TailAB4CNode32Count].value18 = 0; D_340[gO101TailAB4CNode32Count].scale = 0.00f; handle = texLoadSprite(0xA8, NULL); D_340[gO101TailAB4CNode32Count].previousType = D_0.chainType; D_340[gO101TailAB4CNode32Count].previous = D_0.chain; D_340[gO101TailAB4CNode32Count].handle = handle; D_0.chainType = 2; D_0.chain = &D_340[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    D_340[gO101TailAB4CNode32Count].x = 0xB4; D_340[gO101TailAB4CNode32Count].y = 0x88; D_340[gO101TailAB4CNode32Count].value10 = 0; D_340[gO101TailAB4CNode32Count].color12 = 0; D_340[gO101TailAB4CNode32Count].color13 = 0; D_340[gO101TailAB4CNode32Count].value14 = 0.0f; D_340[gO101TailAB4CNode32Count].value18 = 0; D_340[gO101TailAB4CNode32Count].scale = 0.000f; handle = texLoadSprite(0xA8, NULL); D_340[gO101TailAB4CNode32Count].previousType = D_0.chainType; D_340[gO101TailAB4CNode32Count].previous = D_0.chain; D_340[gO101TailAB4CNode32Count].handle = handle; D_0.chainType = 2; D_0.chain = &D_340[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x28; D_0.height48 = 0x18; D_0.value4A = 0xF0; D_0.value4C = 0xAA; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailAB4CInputs.data158; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &D_0.childType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x10; length = overlay101ByteLength(gO101TailAB4CInputs.text15C); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0x80; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text15C; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x1E; length = overlay101ByteLength(gO101TailAB4CInputs.text160); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text160; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x28; length = overlay101ByteLength(gO101TailAB4CInputs.text164); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text164; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x36; length = overlay101ByteLength(gO101TailAB4CInputs.text168); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text168; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x40; length = overlay101ByteLength(gO101TailAB4CInputs.text16C); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text16C; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x4A; length = overlay101ByteLength(gO101TailAB4CInputs.text170); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text170; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x54; length = overlay101ByteLength(gO101TailAB4CInputs.text174); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text174; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x84; length = overlay101ByteLength(gO101TailAB4CInputs.text178); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text178; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x8E; length = overlay101ByteLength(gO101TailAB4CInputs.text17C); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text17C; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].x = 0x78; D_540[gO101TailAB4CNode24Count].y = 0x98; length = overlay101ByteLength(gO101TailAB4CInputs.text180); D_540[gO101TailAB4CNode24Count].length = (u8)length; D_540[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].mode = 2; D_540[gO101TailAB4CNode24Count].color0 = 0xFF; D_540[gO101TailAB4CNode24Count].color1 = 0xFF; D_540[gO101TailAB4CNode24Count].color2 = 0; D_540[gO101TailAB4CNode24Count].color3 = 0; D_540[gO101TailAB4CNode24Count].kind = 4; D_540[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text180; D_540[gO101TailAB4CNode24Count].previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    overlay101Reset(&gO101TailAB4CFinalObject3F14);
}
