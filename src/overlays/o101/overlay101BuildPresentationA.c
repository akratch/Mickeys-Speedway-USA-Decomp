#include "PR/ultratypes.h"

typedef struct O101PresARoot {
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
} O101PresARoot;

typedef struct O101PresANode32 {
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
} O101PresANode32;

typedef struct O101PresANode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresANode20;

typedef struct O101PresANode24 {
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
} O101PresANode24;

typedef struct O101PresAInputs {
    u8 pad000[0x12C];
    void *data12C;
    u8 *text130;
} O101PresAInputs;

extern O101PresARoot D_0;
extern s32 gO101PresAOrderCount;
extern void *gO101PresAOrderSlots[];
extern s32 gO101PresANode32Count;
extern O101PresANode32 D_340[];
extern s32 gO101PresANode20Count;
extern O101PresANode20 gO101PresANodes20[];
extern s32 gO101PresANode24Count;
extern O101PresANode24 D_540[];
extern O101PresAInputs gO101PresAInputs;
extern u8 gO101PresAAssetD2C;
extern u8 gO101PresAFinalObject32B8;

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */
extern void *texLoadSprite(s32 key, void *source);
extern void *func_80036DD0(s32 key, O101PresANode20 *node);
extern void overlay101Reset(void *value);
extern s32 overlay101ByteLength(u8 *text);

/* Matched 2026-10-02 (lane x-o101), from 136 masked words (130 on D) at
 * delta 0, by discarding the inherited shape after decoding the relocation
 * table and writing the body in the matched tail builders' form: one symbol
 * per object (the A/B alias pairs were each one address), one-name counter
 * subscripts with each creator result stored straight into its node, the
 * calls at their real arities (the "dim colour in a2" and the four-argument
 * closing call were the leftover a1-a3 of a one-argument overlay101Reset),
 * and the opacity read back from the stored length. The four presentation
 * builders are one function with different constants. The last two orders
 * were the second root header's data, childType, child and the node-24
 * childType before child. */
void overlay101BuildPresentationA(void) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101PresAAssetD2C; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101PresAOrderSlots[gO101PresAOrderCount] = &D_0.chainType; gO101PresAOrderCount = gO101PresAOrderCount + 1;

    D_340[gO101PresANode32Count].x = 0xF2; D_340[gO101PresANode32Count].y = 0x14E; D_340[gO101PresANode32Count].value10 = 0; D_340[gO101PresANode32Count].color12 = 0xFF; D_340[gO101PresANode32Count].color13 = 0; D_340[gO101PresANode32Count].value18 = 0; D_340[gO101PresANode32Count].scale = 1.0f; D_340[gO101PresANode32Count].value14 = 0.0f; D_340[gO101PresANode32Count].handle = texLoadSprite(0x91, NULL); D_340[gO101PresANode32Count].previousType = D_0.chainType; D_340[gO101PresANode32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101PresANode32Count]; gO101PresANode32Count = gO101PresANode32Count + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x50; D_0.value4A = 0xA0; D_0.value4C = 0xAE; D_0.width44 = 0x18; D_0.height48 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = gO101PresAInputs.data12C; D_0.childType = 0; D_0.child = NULL; gO101PresAOrderSlots[gO101PresAOrderCount] = &D_0.childType; gO101PresAOrderCount = gO101PresAOrderCount + 1;

    gO101PresANodes20[gO101PresANode20Count].x = 0x10; gO101PresANodes20[gO101PresANode20Count].y = 0x16; gO101PresANodes20[gO101PresANode20Count].scale = 1.0f; gO101PresANodes20[gO101PresANode20Count].handle = func_80036DD0(0x15, &gO101PresANodes20[gO101PresANode20Count]); gO101PresANodes20[gO101PresANode20Count].previousType = D_0.childType; gO101PresANodes20[gO101PresANode20Count].previous = D_0.child; D_0.childType = 1; D_0.child = &gO101PresANodes20[gO101PresANode20Count]; gO101PresANode20Count = gO101PresANode20Count + 1;

    D_540[gO101PresANode24Count].x = 0x50; D_540[gO101PresANode24Count].y = 0x9C; D_540[gO101PresANode24Count].length = overlay101ByteLength(gO101PresAInputs.text130); D_540[gO101PresANode24Count].opacity = (s8)(s32)((f32)(u32)D_540[gO101PresANode24Count].length * (f32)(s32)1); D_540[gO101PresANode24Count].mode = 2; D_540[gO101PresANode24Count].color0 = 0xFF; D_540[gO101PresANode24Count].color1 = 0xC0; D_540[gO101PresANode24Count].color2 = 0xC0; D_540[gO101PresANode24Count].color3 = 0xFF; D_540[gO101PresANode24Count].kind = 4; D_540[gO101PresANode24Count].text = gO101PresAInputs.text130; D_540[gO101PresANode24Count].previousType = D_0.childType; D_540[gO101PresANode24Count].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101PresANode24Count]; gO101PresANode24Count = gO101PresANode24Count + 1;

    overlay101Reset(&gO101PresAFinalObject32B8);
}
