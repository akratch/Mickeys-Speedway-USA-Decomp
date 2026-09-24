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

extern O101TailRoot gO101TailRoot;
extern s32 gO101TailOrderCount;
extern void *gO101TailOrderSlots[];
extern s32 gO101TailNode32Count;
extern O101TailNode32 gO101TailNodes32[];
extern s32 gO101TailNode24Count;
extern s32 gO101TailNode24CountReload1;
extern s32 gO101TailNode24CountReload2;
extern O101TailNode24 gO101TailNodes24[];
extern O101TailNode24 gO101TailNodes24Reload2[];
extern O101TailInputs gO101TailInputs;
extern u8 gO101TailAssetD7C;
extern u8 gO101TailFinalObject3B78;

/* Relocation metadata has not yet proved the real names. The creator and
 * finalizer are deliberately distinct despite the same encoded zero target. */
extern void *o101TailCreatorReloc(s32 key, void *source, s32 index);
extern void o101TailFinalizerReloc(void *object);
extern s32 overlay101ByteLength(u8 *text);

/* Exact overlay 101 body at +0xA6BC. */
#ifdef NON_MATCHING
void overlay101TailA6BC(s32 base, s32 positionalA1, s32 workIndex) {
    register void *handle;
    s32 length;

    /* Dead second parameter. The empty test stops IDO storing it to its incoming home. */
    if (positionalA1 != 0) {
    }
    gO101TailRoot.kind = 4; gO101TailRoot.width2E = 0x140; gO101TailRoot.height30 = 0xF0; gO101TailRoot.asset34 = &gO101TailAssetD7C; gO101TailRoot.color32 = 0xFF; gO101TailRoot.color33 = 0xFF; gO101TailRoot.value26 = 0; gO101TailRoot.value28 = 0; gO101TailRoot.value2A = 0; gO101TailRoot.value2C = 0; gO101TailRoot.chainType = 0; gO101TailRoot.chain = NULL; gO101TailOrderSlots[gO101TailOrderCount] = &gO101TailRoot.chainType; gO101TailOrderCount = gO101TailOrderCount + 1;

    gO101TailNodes32[gO101TailNode32Count].x = 0xA0; gO101TailNodes32[gO101TailNode32Count].y = 0x78; gO101TailNodes32[gO101TailNode32Count].value10 = 0xE38; gO101TailNodes32[gO101TailNode32Count].color12 = 0; gO101TailNodes32[gO101TailNode32Count].color13 = 0; gO101TailNodes32[gO101TailNode32Count].value18 = 0; gO101TailNodes32[gO101TailNode32Count].scale = 5.0f; gO101TailNodes32[gO101TailNode32Count].value14 = 0.0f; handle = o101TailCreatorReloc(base + 0xA2, NULL, workIndex); gO101TailNodes32[gO101TailNode32Count].handle = handle; gO101TailNodes32[gO101TailNode32Count].previousType = gO101TailRoot.chainType; gO101TailNodes32[gO101TailNode32Count].previous = gO101TailRoot.chain; gO101TailRoot.chainType = 2; gO101TailRoot.chain = &gO101TailNodes32[gO101TailNode32Count]; length = gO101TailNode32Count; gO101TailNode32Count = length + 1;

    gO101TailNodes32[gO101TailNode32Count].x = 0xA0; gO101TailNodes32[gO101TailNode32Count].y = 0x78; gO101TailNodes32[gO101TailNode32Count].value10 = 0x71C; gO101TailNodes32[gO101TailNode32Count].color12 = 0; gO101TailNodes32[gO101TailNode32Count].color13 = 0; gO101TailNodes32[gO101TailNode32Count].value18 = 0; gO101TailNodes32[gO101TailNode32Count].scale = 5.0f; gO101TailNodes32[gO101TailNode32Count].value14 = 0.0f; handle = o101TailCreatorReloc(base + 0xA2, NULL, length); gO101TailNodes32[gO101TailNode32Count].handle = handle; gO101TailNodes32[gO101TailNode32Count].previousType = gO101TailRoot.chainType; gO101TailNodes32[gO101TailNode32Count].previous = gO101TailRoot.chain; gO101TailRoot.chainType = 2; gO101TailRoot.chain = &gO101TailNodes32[gO101TailNode32Count]; length = gO101TailNode32Count; gO101TailNode32Count = length + 1;

    gO101TailNodes32[gO101TailNode32Count].x = 0xA0; gO101TailNodes32[gO101TailNode32Count].y = 0x78; gO101TailNodes32[gO101TailNode32Count].value10 = 0; gO101TailNodes32[gO101TailNode32Count].color12 = 0; gO101TailNodes32[gO101TailNode32Count].color13 = 0; gO101TailNodes32[gO101TailNode32Count].value18 = 0; gO101TailNodes32[gO101TailNode32Count].scale = 5.0f; gO101TailNodes32[gO101TailNode32Count].value14 = 0.0f; handle = o101TailCreatorReloc(base + 0xA2, NULL, length); gO101TailNodes32[gO101TailNode32Count].handle = handle; gO101TailNodes32[gO101TailNode32Count].previousType = gO101TailRoot.chainType; gO101TailNodes32[gO101TailNode32Count].previous = gO101TailRoot.chain; gO101TailRoot.chainType = 2; gO101TailRoot.chain = &gO101TailNodes32[gO101TailNode32Count]; gO101TailNode32Count = gO101TailNode32Count + 1;

    gO101TailRoot.x42 = 0x20; gO101TailRoot.y46 = 0x3C; gO101TailRoot.height48 = 0x50; gO101TailRoot.value4A = 0xC8; gO101TailRoot.value4C = 0x2A; gO101TailRoot.width44 = 0x18; gO101TailRoot.mode40 = 0; gO101TailRoot.color4E = 0xFF; gO101TailRoot.color4F = 0xFF; gO101TailRoot.childType = 0; gO101TailRoot.child = NULL; gO101TailRoot.data50 = gO101TailInputs.data14C; gO101TailOrderSlots[gO101TailOrderCount] = &gO101TailRoot.childType; gO101TailOrderCount = gO101TailOrderCount + 1;

    gO101TailNodes24[gO101TailNode24Count].x = 0x64; gO101TailNodes24[gO101TailNode24Count].y = 0x10; length = overlay101ByteLength(gO101TailInputs.text150); gO101TailNodes24[gO101TailNode24Count].length = (u8)length; gO101TailNodes24[gO101TailNode24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailNodes24[gO101TailNode24Count].mode = 2; gO101TailNodes24[gO101TailNode24Count].text = gO101TailInputs.text150; gO101TailNodes24[gO101TailNode24Count].color0 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color1 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color2 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color3 = 0xFF; gO101TailNodes24[gO101TailNode24Count].kind = 4; gO101TailNodes24[gO101TailNode24Count].previousType = gO101TailRoot.childType; gO101TailNodes24[gO101TailNode24Count].previous = gO101TailRoot.child; gO101TailRoot.child = &gO101TailNodes24[gO101TailNode24Count]; gO101TailRoot.childType = 3; gO101TailNode24Count = gO101TailNode24Count + 1;

    gO101TailNodes24[gO101TailNode24Count].x = 0x64; gO101TailNodes24[gO101TailNode24Count].y = 0x1A; length = overlay101ByteLength(gO101TailInputs.text154); gO101TailNodes24[gO101TailNode24Count].length = (u8)length; gO101TailNodes24[gO101TailNode24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailNodes24[gO101TailNode24Count].mode = 2; gO101TailNodes24[gO101TailNode24Count].text = gO101TailInputs.text154; gO101TailNodes24[gO101TailNode24Count].color0 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color1 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color2 = 0xFF; gO101TailNodes24[gO101TailNode24Count].color3 = 0xFF; gO101TailNodes24[gO101TailNode24Count].kind = 4; gO101TailNodes24[gO101TailNode24Count].previousType = gO101TailRoot.childType; gO101TailNodes24[gO101TailNode24Count].previous = gO101TailRoot.child; gO101TailRoot.child = &gO101TailNodes24[gO101TailNode24Count]; gO101TailRoot.childType = 3; gO101TailNode24Count = gO101TailNode24Count + 1;

    o101TailFinalizerReloc(&gO101TailFinalObject3B78);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailA6BC/func_overlay_101_F000A6BC_18E5EDC.s")
#endif

/* PLATEAU-HANDOFF:overlay101TailA6BC:start
 * symbol: overlay101TailA6BC
 * score: 53 differing words
 * frame: 0x40
 * relocations: 44
 * first-mismatch: +0x0
 * summary: 53 words at size delta 0. Frame 0x40 vs 0x38 with the same slots. Second node-24 row and an unused 8-byte frame remain.
 * PLATEAU-HANDOFF:overlay101TailA6BC:end
 */
