#include "PR/ultratypes.h"

typedef struct O101PresCRoot {
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
} O101PresCRoot;

typedef struct O101PresCNode32 {
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
} O101PresCNode32;

typedef struct O101PresCNode20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101PresCNode20;

typedef struct O101PresCNode24 {
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
} O101PresCNode24;

typedef struct O101PresCInputs {
    u8 pad000[0x13C];
    void *data13C;
    u8 *text140;
} O101PresCInputs;

extern O101PresCRoot gO101PresCRoot;
extern s32 gO101PresCOrderCount;
extern void *gO101PresCOrderSlots[];
extern s32 gO101PresCNode32Count;
extern O101PresCNode32 gO101PresCNodes32[];
extern s32 gO101PresCNode20Count;
extern O101PresCNode20 gO101PresCNodes20[];
extern s32 gO101PresCNode24Count;
extern O101PresCNode24 gO101PresCNodes24[];
extern O101PresCInputs gO101PresCInputs;
extern u8 gO101PresCAssetD54;
extern u8 gO101PresCFinalObject3718;

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */
extern void *o101PresCCreatorReloc(s32 key, void *source);
extern void *o101PresCCreateCompactReloc(s32 key, O101PresCNode20 *node);
extern void o101PresCResetReloc(void *value);
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
void overlay101BuildPresentationC(void) {
    gO101PresCRoot.kind = 4; gO101PresCRoot.width2E = 0x140; gO101PresCRoot.height30 = 0xF0; gO101PresCRoot.asset34 = &gO101PresCAssetD54; gO101PresCRoot.color32 = 0xFF; gO101PresCRoot.color33 = 0xFF; gO101PresCRoot.value26 = 0; gO101PresCRoot.value28 = 0; gO101PresCRoot.value2A = 0; gO101PresCRoot.value2C = 0; gO101PresCRoot.chainType = 0; gO101PresCRoot.chain = NULL; gO101PresCOrderSlots[gO101PresCOrderCount] = &gO101PresCRoot.chainType; gO101PresCOrderCount = gO101PresCOrderCount + 1;

    gO101PresCNodes32[gO101PresCNode32Count].x = 0xF2; gO101PresCNodes32[gO101PresCNode32Count].y = 0x14E; gO101PresCNodes32[gO101PresCNode32Count].value10 = 0; gO101PresCNodes32[gO101PresCNode32Count].color12 = 0xFF; gO101PresCNodes32[gO101PresCNode32Count].color13 = 0; gO101PresCNodes32[gO101PresCNode32Count].value18 = 0; gO101PresCNodes32[gO101PresCNode32Count].scale = 1.0f; gO101PresCNodes32[gO101PresCNode32Count].value14 = 0.0f; gO101PresCNodes32[gO101PresCNode32Count].handle = o101PresCCreatorReloc(0x91, NULL); gO101PresCNodes32[gO101PresCNode32Count].previousType = gO101PresCRoot.chainType; gO101PresCNodes32[gO101PresCNode32Count].previous = gO101PresCRoot.chain; gO101PresCRoot.chainType = 2; gO101PresCRoot.chain = &gO101PresCNodes32[gO101PresCNode32Count]; gO101PresCNode32Count = gO101PresCNode32Count + 1;

    gO101PresCRoot.x42 = 0x20; gO101PresCRoot.y46 = 0x50; gO101PresCRoot.value4A = 0xA0; gO101PresCRoot.value4C = 0xAE; gO101PresCRoot.width44 = 0x18; gO101PresCRoot.height48 = 0x18; gO101PresCRoot.mode40 = 0; gO101PresCRoot.color4E = 0xFF; gO101PresCRoot.color4F = 0xFF; gO101PresCRoot.data50 = gO101PresCInputs.data13C; gO101PresCRoot.childType = 0; gO101PresCRoot.child = NULL; gO101PresCOrderSlots[gO101PresCOrderCount] = &gO101PresCRoot.childType; gO101PresCOrderCount = gO101PresCOrderCount + 1;

    gO101PresCNodes20[gO101PresCNode20Count].x = 0x10; gO101PresCNodes20[gO101PresCNode20Count].y = 0x16; gO101PresCNodes20[gO101PresCNode20Count].scale = 1.0f; gO101PresCNodes20[gO101PresCNode20Count].handle = o101PresCCreateCompactReloc(0x17, &gO101PresCNodes20[gO101PresCNode20Count]); gO101PresCNodes20[gO101PresCNode20Count].previousType = gO101PresCRoot.childType; gO101PresCNodes20[gO101PresCNode20Count].previous = gO101PresCRoot.child; gO101PresCRoot.childType = 1; gO101PresCRoot.child = &gO101PresCNodes20[gO101PresCNode20Count]; gO101PresCNode20Count = gO101PresCNode20Count + 1;

    gO101PresCNodes24[gO101PresCNode24Count].x = 0x50; gO101PresCNodes24[gO101PresCNode24Count].y = 0x9C; gO101PresCNodes24[gO101PresCNode24Count].length = overlay101ByteLength(gO101PresCInputs.text140); gO101PresCNodes24[gO101PresCNode24Count].opacity = (s8)(s32)((f32)(u32)gO101PresCNodes24[gO101PresCNode24Count].length * (f32)(s32)1); gO101PresCNodes24[gO101PresCNode24Count].mode = 2; gO101PresCNodes24[gO101PresCNode24Count].color0 = 0xC0; gO101PresCNodes24[gO101PresCNode24Count].color1 = 0xC0; gO101PresCNodes24[gO101PresCNode24Count].color2 = 0xFF; gO101PresCNodes24[gO101PresCNode24Count].color3 = 0xFF; gO101PresCNodes24[gO101PresCNode24Count].kind = 4; gO101PresCNodes24[gO101PresCNode24Count].text = gO101PresCInputs.text140; gO101PresCNodes24[gO101PresCNode24Count].previousType = gO101PresCRoot.childType; gO101PresCNodes24[gO101PresCNode24Count].previous = gO101PresCRoot.child; gO101PresCRoot.childType = 3; gO101PresCRoot.child = &gO101PresCNodes24[gO101PresCNode24Count]; gO101PresCNode24Count = gO101PresCNode24Count + 1;

    o101PresCResetReloc(&gO101PresCFinalObject3718);
}
