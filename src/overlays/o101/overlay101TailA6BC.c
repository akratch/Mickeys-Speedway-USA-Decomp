#include "PR/ultratypes.h"

typedef struct O101TailRoot {
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
} O101TailRoot;

typedef struct O101TailNode32 {
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
} O101TailNode32;

typedef struct O101TailNode24 {
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
} O101TailNode24;

typedef struct O101TailInputs {
    u8 pad000[0x14C];
    void *data14C;
    u8 *text150;
    u8 *text154;
} O101TailInputs;

extern O101TailRoot D_0;
extern s32 gO101TailOrderCount;
extern void *gO101TailOrderSlots[];
extern s32 gO101TailNode32Count;
extern O101TailNode32 D_340[];
extern s32 gO101TailNode24Count;
extern O101TailNode24 D_540[];
extern O101TailInputs gO101TailInputs;
extern u8 gO101TailAssetD7C;
extern u8 gO101TailFinalObject3B78;
/* All three sprite nodes are created by one resident routine (one SYMBOL
 * target in the relocation table, the same one overlay101TailAB4C calls),
 * and it takes two arguments: the a2 the shipped calls carry is the node
 * count's leftover register, not an argument. The finalizer is a
 * same-module SYMBOL record, so it is a placeholder too. */
extern void *texLoadSprite(s32 key, void *source);
extern void overlay101Reset(void *object);
extern s32 overlay101ByteLength(u8 *text);

/* Matched 2026-10-02 (lane x-o101), 53 -> 0 at delta 0, by copying the
 * matched sibling overlay101TailAB4C's shape: the creator called with its
 * real two arguments and the function taking only `base` (53 -> 8; the old
 * third argument and the dead second parameter were the leftover a2), the
 * root header stored kind, width2E, height30 (8 -> 2), and no handle or
 * length local: the creator result is stored straight into the node and the
 * opacity reads the stored length back (2 -> 0, the frame 0x40 -> 0x38, since
 * each declared local had held a frame cell above the base + 0xA2 spill). */
void overlay101TailA6BC(s32 base) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101TailAssetD7C; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailOrderSlots[gO101TailOrderCount] = &D_0.chainType; gO101TailOrderCount = gO101TailOrderCount + 1;

    D_340[gO101TailNode32Count].x = 0xA0; D_340[gO101TailNode32Count].y = 0x78; D_340[gO101TailNode32Count].value10 = 0xE38; D_340[gO101TailNode32Count].color12 = 0; D_340[gO101TailNode32Count].color13 = 0; D_340[gO101TailNode32Count].value18 = 0; D_340[gO101TailNode32Count].scale = 5.0f; D_340[gO101TailNode32Count].value14 = 0.0f; D_340[gO101TailNode32Count].handle = texLoadSprite(base + 0xA2, NULL); D_340[gO101TailNode32Count].previousType = D_0.chainType; D_340[gO101TailNode32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101TailNode32Count]; gO101TailNode32Count = gO101TailNode32Count + 1;

    D_340[gO101TailNode32Count].x = 0xA0; D_340[gO101TailNode32Count].y = 0x78; D_340[gO101TailNode32Count].value10 = 0x71C; D_340[gO101TailNode32Count].color12 = 0; D_340[gO101TailNode32Count].color13 = 0; D_340[gO101TailNode32Count].value18 = 0; D_340[gO101TailNode32Count].scale = 5.0f; D_340[gO101TailNode32Count].value14 = 0.0f; D_340[gO101TailNode32Count].handle = texLoadSprite(base + 0xA2, NULL); D_340[gO101TailNode32Count].previousType = D_0.chainType; D_340[gO101TailNode32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101TailNode32Count]; gO101TailNode32Count = gO101TailNode32Count + 1;

    D_340[gO101TailNode32Count].x = 0xA0; D_340[gO101TailNode32Count].y = 0x78; D_340[gO101TailNode32Count].value10 = 0; D_340[gO101TailNode32Count].color12 = 0; D_340[gO101TailNode32Count].color13 = 0; D_340[gO101TailNode32Count].value18 = 0; D_340[gO101TailNode32Count].scale = 5.0f; D_340[gO101TailNode32Count].value14 = 0.0f; D_340[gO101TailNode32Count].handle = texLoadSprite(base + 0xA2, NULL); D_340[gO101TailNode32Count].previousType = D_0.chainType; D_340[gO101TailNode32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101TailNode32Count]; gO101TailNode32Count = gO101TailNode32Count + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x3C; D_0.height48 = 0x50; D_0.value4A = 0xC8; D_0.value4C = 0x2A; D_0.width44 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailInputs.data14C; gO101TailOrderSlots[gO101TailOrderCount] = &D_0.childType; gO101TailOrderCount = gO101TailOrderCount + 1;

    D_540[gO101TailNode24Count].x = 0x64; D_540[gO101TailNode24Count].y = 0x10; D_540[gO101TailNode24Count].length = overlay101ByteLength(gO101TailInputs.text150); D_540[gO101TailNode24Count].opacity = (s8)(s32)((f32)(u32)D_540[gO101TailNode24Count].length * (f32)(s32)1); D_540[gO101TailNode24Count].mode = 2; D_540[gO101TailNode24Count].color0 = 0xFF; D_540[gO101TailNode24Count].color1 = 0xFF; D_540[gO101TailNode24Count].color2 = 0xFF; D_540[gO101TailNode24Count].color3 = 0xFF; D_540[gO101TailNode24Count].kind = 4; D_540[gO101TailNode24Count].text = gO101TailInputs.text150; D_540[gO101TailNode24Count].previousType = D_0.childType; D_540[gO101TailNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailNode24Count]; gO101TailNode24Count = gO101TailNode24Count + 1;

    D_540[gO101TailNode24Count].x = 0x64; D_540[gO101TailNode24Count].y = 0x1A; D_540[gO101TailNode24Count].length = overlay101ByteLength(gO101TailInputs.text154); D_540[gO101TailNode24Count].opacity = (s8)(s32)((f32)(u32)D_540[gO101TailNode24Count].length * (f32)(s32)1); D_540[gO101TailNode24Count].mode = 2; D_540[gO101TailNode24Count].color0 = 0xFF; D_540[gO101TailNode24Count].color1 = 0xFF; D_540[gO101TailNode24Count].color2 = 0xFF; D_540[gO101TailNode24Count].color3 = 0xFF; D_540[gO101TailNode24Count].kind = 4; D_540[gO101TailNode24Count].text = gO101TailInputs.text154; D_540[gO101TailNode24Count].previousType = D_0.childType; D_540[gO101TailNode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailNode24Count]; gO101TailNode24Count = gO101TailNode24Count + 1;

    overlay101Reset(&gO101TailFinalObject3B78);
}
