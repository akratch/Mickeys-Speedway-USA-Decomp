#include "PR/ultratypes.h"

typedef struct O101PresDRoot {
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
} O101PresDRoot;

typedef struct O101PresDNode32 {
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
} O101PresDNode32;

typedef struct O101PresDNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresDNode20;

typedef struct O101PresDNode24 {
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
} O101PresDNode24;

typedef struct O101PresDInputs {
    u8 pad000[0x144];
    void *data144;
    u8 *text148;
} O101PresDInputs;

extern O101PresDRoot gO101PresDRoot;
extern s32 gO101PresDOrderCount;
extern void *gO101PresDOrderSlots[];
extern s32 gO101PresDNode32Count;
extern O101PresDNode32 gO101PresDNodes32[];
extern s32 gO101PresDNode20Count;
extern O101PresDNode20 gO101PresDNodes20[];
extern s32 gO101PresDNode24Count;
extern O101PresDNode24 gO101PresDNodes24[];
extern O101PresDInputs gO101PresDInputs;
extern u8 gO101PresDAssetD68;
extern u8 gO101PresDFinalObject3948;

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */
extern void *o101PresDCreatorReloc(s32 key, void *source);
extern void *o101PresDCreateCompactReloc(s32 key, O101PresDNode20 *node);
extern void o101PresDResetReloc(void *value);
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
void overlay101BuildPresentationD(void) {
    gO101PresDRoot.kind = 4; gO101PresDRoot.width2E = 0x140; gO101PresDRoot.height30 = 0xF0; gO101PresDRoot.asset34 = &gO101PresDAssetD68; gO101PresDRoot.color32 = 0xFF; gO101PresDRoot.color33 = 0xFF; gO101PresDRoot.value26 = 0; gO101PresDRoot.value28 = 0; gO101PresDRoot.value2A = 0; gO101PresDRoot.value2C = 0; gO101PresDRoot.chainType = 0; gO101PresDRoot.chain = NULL; gO101PresDOrderSlots[gO101PresDOrderCount] = &gO101PresDRoot.chainType; gO101PresDOrderCount = gO101PresDOrderCount + 1;

    gO101PresDNodes32[gO101PresDNode32Count].x = 0xF2; gO101PresDNodes32[gO101PresDNode32Count].y = 0x14E; gO101PresDNodes32[gO101PresDNode32Count].value10 = 0; gO101PresDNodes32[gO101PresDNode32Count].color12 = 0xFF; gO101PresDNodes32[gO101PresDNode32Count].color13 = 0; gO101PresDNodes32[gO101PresDNode32Count].value18 = 0; gO101PresDNodes32[gO101PresDNode32Count].scale = 1.0f; gO101PresDNodes32[gO101PresDNode32Count].value14 = 0.0f; gO101PresDNodes32[gO101PresDNode32Count].handle = o101PresDCreatorReloc(0x91, NULL); gO101PresDNodes32[gO101PresDNode32Count].previousType = gO101PresDRoot.chainType; gO101PresDNodes32[gO101PresDNode32Count].previous = gO101PresDRoot.chain; gO101PresDRoot.chainType = 2; gO101PresDRoot.chain = &gO101PresDNodes32[gO101PresDNode32Count]; gO101PresDNode32Count = gO101PresDNode32Count + 1;

    gO101PresDRoot.x42 = 0x20; gO101PresDRoot.y46 = 0x50; gO101PresDRoot.value4A = 0xA0; gO101PresDRoot.value4C = 0xAE; gO101PresDRoot.width44 = 0x18; gO101PresDRoot.height48 = 0x18; gO101PresDRoot.mode40 = 0; gO101PresDRoot.color4E = 0xFF; gO101PresDRoot.color4F = 0xFF; gO101PresDRoot.data50 = gO101PresDInputs.data144; gO101PresDRoot.childType = 0; gO101PresDRoot.child = NULL; gO101PresDOrderSlots[gO101PresDOrderCount] = &gO101PresDRoot.childType; gO101PresDOrderCount = gO101PresDOrderCount + 1;

    gO101PresDNodes20[gO101PresDNode20Count].x = 0x10; gO101PresDNodes20[gO101PresDNode20Count].y = 0x16; gO101PresDNodes20[gO101PresDNode20Count].scale = 1.0f; gO101PresDNodes20[gO101PresDNode20Count].handle = o101PresDCreateCompactReloc(0x18, &gO101PresDNodes20[gO101PresDNode20Count]); gO101PresDNodes20[gO101PresDNode20Count].previousType = gO101PresDRoot.childType; gO101PresDNodes20[gO101PresDNode20Count].previous = gO101PresDRoot.child; gO101PresDRoot.childType = 1; gO101PresDRoot.child = &gO101PresDNodes20[gO101PresDNode20Count]; gO101PresDNode20Count = gO101PresDNode20Count + 1;

    gO101PresDNodes24[gO101PresDNode24Count].x = 0x50; gO101PresDNodes24[gO101PresDNode24Count].y = 0x9C; gO101PresDNodes24[gO101PresDNode24Count].length = overlay101ByteLength(gO101PresDInputs.text148); gO101PresDNodes24[gO101PresDNode24Count].opacity = (s8)(s32)((f32)(u32)gO101PresDNodes24[gO101PresDNode24Count].length * (f32)(s32)1); gO101PresDNodes24[gO101PresDNode24Count].mode = 2; gO101PresDNodes24[gO101PresDNode24Count].color0 = 0xFF; gO101PresDNodes24[gO101PresDNode24Count].color1 = 0xFF; gO101PresDNodes24[gO101PresDNode24Count].color2 = 0; gO101PresDNodes24[gO101PresDNode24Count].color3 = 0xFF; gO101PresDNodes24[gO101PresDNode24Count].kind = 4; gO101PresDNodes24[gO101PresDNode24Count].text = gO101PresDInputs.text148; gO101PresDNodes24[gO101PresDNode24Count].previousType = gO101PresDRoot.childType; gO101PresDNodes24[gO101PresDNode24Count].previous = gO101PresDRoot.child; gO101PresDRoot.childType = 3; gO101PresDRoot.child = &gO101PresDNodes24[gO101PresDNode24Count]; gO101PresDNode24Count = gO101PresDNode24Count + 1;

    o101PresDResetReloc(&gO101PresDFinalObject3948);
}
