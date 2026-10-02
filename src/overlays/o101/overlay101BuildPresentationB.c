#include "PR/ultratypes.h"

typedef struct O101PresBRoot {
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
} O101PresBRoot;

typedef struct O101PresBNode32 {
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
} O101PresBNode32;

typedef struct O101PresBNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresBNode20;

typedef struct O101PresBNode24 {
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
} O101PresBNode24;

typedef struct O101PresBInputs {
    u8 pad000[0x134];
    void *data134;
    u8 *text138;
} O101PresBInputs;

extern O101PresBRoot gO101PresBRoot;
extern s32 gO101PresBOrderCount;
extern void *gO101PresBOrderSlots[];
extern s32 gO101PresBNode32Count;
extern O101PresBNode32 gO101PresBNodes32[];
extern s32 gO101PresBNode20Count;
extern O101PresBNode20 gO101PresBNodes20[];
extern s32 gO101PresBNode24Count;
extern O101PresBNode24 gO101PresBNodes24[];
extern O101PresBInputs gO101PresBInputs;
extern u8 gO101PresBAssetD40;
extern u8 gO101PresBFinalObject34E8;

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */
extern void *o101PresBCreatorReloc(s32 key, void *source);
extern void *o101PresBCreateCompactReloc(s32 key, O101PresBNode20 *node);
extern void o101PresBResetReloc(void *value);
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
void overlay101BuildPresentationB(void) {
    gO101PresBRoot.kind = 4; gO101PresBRoot.width2E = 0x140; gO101PresBRoot.height30 = 0xF0; gO101PresBRoot.asset34 = &gO101PresBAssetD40; gO101PresBRoot.color32 = 0xFF; gO101PresBRoot.color33 = 0xFF; gO101PresBRoot.value26 = 0; gO101PresBRoot.value28 = 0; gO101PresBRoot.value2A = 0; gO101PresBRoot.value2C = 0; gO101PresBRoot.chainType = 0; gO101PresBRoot.chain = NULL; gO101PresBOrderSlots[gO101PresBOrderCount] = &gO101PresBRoot.chainType; gO101PresBOrderCount = gO101PresBOrderCount + 1;

    gO101PresBNodes32[gO101PresBNode32Count].x = 0xF2; gO101PresBNodes32[gO101PresBNode32Count].y = 0x14E; gO101PresBNodes32[gO101PresBNode32Count].value10 = 0; gO101PresBNodes32[gO101PresBNode32Count].color12 = 0xFF; gO101PresBNodes32[gO101PresBNode32Count].color13 = 0; gO101PresBNodes32[gO101PresBNode32Count].value18 = 0; gO101PresBNodes32[gO101PresBNode32Count].scale = 1.0f; gO101PresBNodes32[gO101PresBNode32Count].value14 = 0.0f; gO101PresBNodes32[gO101PresBNode32Count].handle = o101PresBCreatorReloc(0x91, NULL); gO101PresBNodes32[gO101PresBNode32Count].previousType = gO101PresBRoot.chainType; gO101PresBNodes32[gO101PresBNode32Count].previous = gO101PresBRoot.chain; gO101PresBRoot.chainType = 2; gO101PresBRoot.chain = &gO101PresBNodes32[gO101PresBNode32Count]; gO101PresBNode32Count = gO101PresBNode32Count + 1;

    gO101PresBRoot.x42 = 0x20; gO101PresBRoot.y46 = 0x50; gO101PresBRoot.value4A = 0xA0; gO101PresBRoot.value4C = 0xAE; gO101PresBRoot.width44 = 0x18; gO101PresBRoot.height48 = 0x18; gO101PresBRoot.mode40 = 0; gO101PresBRoot.color4E = 0xFF; gO101PresBRoot.color4F = 0xFF; gO101PresBRoot.data50 = gO101PresBInputs.data134; gO101PresBRoot.childType = 0; gO101PresBRoot.child = NULL; gO101PresBOrderSlots[gO101PresBOrderCount] = &gO101PresBRoot.childType; gO101PresBOrderCount = gO101PresBOrderCount + 1;

    gO101PresBNodes20[gO101PresBNode20Count].x = 0x10; gO101PresBNodes20[gO101PresBNode20Count].y = 0x16; gO101PresBNodes20[gO101PresBNode20Count].scale = 1.0f; gO101PresBNodes20[gO101PresBNode20Count].handle = o101PresBCreateCompactReloc(0x16, &gO101PresBNodes20[gO101PresBNode20Count]); gO101PresBNodes20[gO101PresBNode20Count].previousType = gO101PresBRoot.childType; gO101PresBNodes20[gO101PresBNode20Count].previous = gO101PresBRoot.child; gO101PresBRoot.childType = 1; gO101PresBRoot.child = &gO101PresBNodes20[gO101PresBNode20Count]; gO101PresBNode20Count = gO101PresBNode20Count + 1;

    gO101PresBNodes24[gO101PresBNode24Count].x = 0x50; gO101PresBNodes24[gO101PresBNode24Count].y = 0x9C; gO101PresBNodes24[gO101PresBNode24Count].length = overlay101ByteLength(gO101PresBInputs.text138); gO101PresBNodes24[gO101PresBNode24Count].opacity = (s8)(s32)((f32)(u32)gO101PresBNodes24[gO101PresBNode24Count].length * (f32)(s32)1); gO101PresBNodes24[gO101PresBNode24Count].mode = 2; gO101PresBNodes24[gO101PresBNode24Count].color0 = 0xC0; gO101PresBNodes24[gO101PresBNode24Count].color1 = 0xFF; gO101PresBNodes24[gO101PresBNode24Count].color2 = 0xC0; gO101PresBNodes24[gO101PresBNode24Count].color3 = 0xFF; gO101PresBNodes24[gO101PresBNode24Count].kind = 4; gO101PresBNodes24[gO101PresBNode24Count].text = gO101PresBInputs.text138; gO101PresBNodes24[gO101PresBNode24Count].previousType = gO101PresBRoot.childType; gO101PresBNodes24[gO101PresBNode24Count].previous = gO101PresBRoot.child; gO101PresBRoot.childType = 3; gO101PresBRoot.child = &gO101PresBNodes24[gO101PresBNode24Count]; gO101PresBNode24Count = gO101PresBNode24Count + 1;

    o101PresBResetReloc(&gO101PresBFinalObject34E8);
}
