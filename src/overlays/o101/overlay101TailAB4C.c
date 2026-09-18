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

extern O101TailAB4CRoot gO101TailAB4CRoot;
extern s32 gO101TailAB4COrderCount;
extern void *gO101TailAB4COrderSlots[];
extern s32 gO101TailAB4CNode32Count;
extern O101TailAB4CNode32 gO101TailAB4CNodes32[];
extern s32 gO101TailAB4CNode24Count;
extern O101TailAB4CNode24 gO101TailAB4CNodes24[];
extern O101TailAB4CInputs gO101TailAB4CInputs;
extern u8 gO101TailAB4CAssetD90;
extern u8 gO101TailAB4CFinalObject3F14;

/* Old-style declarations avoid asserting unproved formal arity while calls
 * preserve each site's three-register physical image. */
extern void *o101TailAB4CCreator1Reloc();
extern void *o101TailAB4CCreator2Reloc();
extern void *o101TailAB4CCreator3Reloc();
extern void o101TailAB4CFinalizerReloc(void *object);
extern s32 overlay101ByteLength(u8 *text);

/* Workbench p5 read this as `structure-mismatch` at 636 differing words from
 * +0x4, with the candidate 665 instructions against 638 and overrunning its
 * 2,552-byte owner by 108 bytes. Lane p10-o101, 2026-09-12, retired most of
 * that as a carrier problem, using the shape that closed six other builders in
 * this overlay on the same day.
 * L145 -- `nodeIndex`, `orderIndex`, `node32`, `node24`, and the three pool
 *   carriers `node24CountPtr`, `node24Pool` and `node24Stride` are all gone;
 *   every store is spelled through the array subscript of the counter global
 *   itself, so the subscript written on both sides of each call is ONE IR name
 *   (L131) and the pre-call counter read lands in a ring temporary, which a
 *   local can never be (L130). The bump moves to the end of its group.
 *   This overlay's counters have ONE name each here, so the lever applies
 *   whole-body, unlike the BA34 tail where the order counter has two.
 * L144/L103 -- the `opacityScale` local is gone too; the multiplier is the
 *   `(f32)(s32)1` literal in place, as in every matched sibling.
 * Lane w13-o101, 2026-09-18: 579 masked at delta -8 -> 499 at delta 0.
 * L103 -- sprite nodes 2 and 3 were CSE'ing node 1's `0.0f`, so the two
 *   `mtc1 $zero` the target emits for those nodes were missing. Distinct
 *   spellings `0.00f` and `0.000f` split the pool entries (L103) and close
 *   the size. `(f32)(s32)0` on both nodes only reached delta -4 (the cvt
 *   overshoots); `(f32)(s32)1` for node 1's scale regresses to -8.
 * L149 -- `u8 length` without a second `& 0xFF` is byte-identical to the
 *   inherited `s32` plus mask; the per-row addiu-3 is not a folded extra
 *   draw. draw_census stays 19 GP draws per text row (21 on the first).
 * L154 -- `pool[count]` vs `*(Node24 *)((u8 *)pool + count * 0x18)` is
 *   byte-identical; the array form is kept as the matched-sibling spelling.
 * The remaining per-row addiu-3 pair is the same `childType = 3` constant
 *   materialized late (ours) vs with mode=2 and kind=4 (target). Hoisting
 *   the chain stores ahead of mode is 470 positional / 154 structural
 *   against 499 / 74 -- worse aligned. child then childType, and
 *   `(s32)(u8)3`, are inert. Integer census is 24 percent coherent across
 *   33 windows (not a ring phase). FP is one clean six-cycle at 98 percent.
 * NEXT LEVER: get constant 3 demanded in the post-call materialization
 *   window with 2 and 4, without moving the previousType copy. */
#ifdef NON_MATCHING
void func_overlay_101_F000AB4C_18E636C(void) {
    void *handle;
    u8 length;

    gO101TailAB4CRoot.height30 = 0xF0; gO101TailAB4CRoot.width2E = 0x140; gO101TailAB4CRoot.kind = 4; gO101TailAB4CRoot.asset34 = &gO101TailAB4CAssetD90; gO101TailAB4CRoot.color32 = 0xFF; gO101TailAB4CRoot.color33 = 0xFF; gO101TailAB4CRoot.value26 = 0; gO101TailAB4CRoot.value28 = 0; gO101TailAB4CRoot.value2A = 0; gO101TailAB4CRoot.value2C = 0; gO101TailAB4CRoot.chainType = 0; gO101TailAB4CRoot.chain = NULL; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &gO101TailAB4CRoot.chainType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    gO101TailAB4CNodes32[gO101TailAB4CNode32Count].x = 0xF0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].y = 0x154; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value10 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color12 = 0xFF; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color13 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value18 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].scale = 1.0f; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value14 = 0.0f; handle = o101TailAB4CCreator1Reloc(0x91, NULL, gO101TailAB4COrderCount); gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previousType = gO101TailAB4CRoot.chainType; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previous = gO101TailAB4CRoot.chain; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].handle = handle; gO101TailAB4CRoot.chainType = 2; gO101TailAB4CRoot.chain = &gO101TailAB4CNodes32[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    gO101TailAB4CNodes32[gO101TailAB4CNode32Count].x = 0x8C; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].y = 0x88; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value10 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color12 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color13 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value18 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].scale = 0.00f; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value14 = 0.00f; handle = o101TailAB4CCreator2Reloc(0xA8, NULL, gO101TailAB4CNodes32); gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previousType = gO101TailAB4CRoot.chainType; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previous = gO101TailAB4CRoot.chain; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].handle = handle; gO101TailAB4CRoot.chainType = 2; gO101TailAB4CRoot.chain = &gO101TailAB4CNodes32[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    gO101TailAB4CNodes32[gO101TailAB4CNode32Count].x = 0xB4; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].y = 0x88; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value10 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color12 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].color13 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value18 = 0; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].scale = 0.000f; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].value14 = 0.000f; handle = o101TailAB4CCreator3Reloc(0xA8, NULL, gO101TailAB4CNodes32); gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previousType = gO101TailAB4CRoot.chainType; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].previous = gO101TailAB4CRoot.chain; gO101TailAB4CNodes32[gO101TailAB4CNode32Count].handle = handle; gO101TailAB4CRoot.chainType = 2; gO101TailAB4CRoot.chain = &gO101TailAB4CNodes32[gO101TailAB4CNode32Count]; gO101TailAB4CNode32Count = gO101TailAB4CNode32Count + 1;

    gO101TailAB4CRoot.x42 = 0x20; gO101TailAB4CRoot.width44 = 0x18; gO101TailAB4CRoot.y46 = 0x28; gO101TailAB4CRoot.height48 = 0x18; gO101TailAB4CRoot.value4A = 0xF0; gO101TailAB4CRoot.value4C = 0xAA; gO101TailAB4CRoot.mode40 = 0; gO101TailAB4CRoot.color4E = 0xFF; gO101TailAB4CRoot.color4F = 0xFF; gO101TailAB4CRoot.childType = 0; gO101TailAB4CRoot.child = NULL; gO101TailAB4CRoot.data50 = gO101TailAB4CInputs.data158; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &gO101TailAB4CRoot.childType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x10; length = overlay101ByteLength(gO101TailAB4CInputs.text15C); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0x80; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text15C; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x1E; length = overlay101ByteLength(gO101TailAB4CInputs.text160); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text160; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x28; length = overlay101ByteLength(gO101TailAB4CInputs.text164); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text164; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x36; length = overlay101ByteLength(gO101TailAB4CInputs.text168); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text168; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x40; length = overlay101ByteLength(gO101TailAB4CInputs.text16C); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text16C; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x4A; length = overlay101ByteLength(gO101TailAB4CInputs.text170); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text170; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x54; length = overlay101ByteLength(gO101TailAB4CInputs.text174); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text174; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x84; length = overlay101ByteLength(gO101TailAB4CInputs.text178); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text178; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x8E; length = overlay101ByteLength(gO101TailAB4CInputs.text17C); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text17C; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    gO101TailAB4CNodes24[gO101TailAB4CNode24Count].x = 0x78; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].y = 0x98; length = overlay101ByteLength(gO101TailAB4CInputs.text180); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].length = (u8)length; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); gO101TailAB4CNodes24[gO101TailAB4CNode24Count].mode = 2; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color0 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color1 = 0xFF; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color2 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].color3 = 0; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].kind = 4; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].text = gO101TailAB4CInputs.text180; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previousType = gO101TailAB4CRoot.childType; gO101TailAB4CNodes24[gO101TailAB4CNode24Count].previous = gO101TailAB4CRoot.child; gO101TailAB4CRoot.childType = 3; gO101TailAB4CRoot.child = &gO101TailAB4CNodes24[gO101TailAB4CNode24Count]; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    o101TailAB4CFinalizerReloc(&gO101TailAB4CFinalObject3F14);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailAB4C/func_overlay_101_F000AB4C_18E636C.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_101_F000AB4C_18E636C:start
 * symbol: func_overlay_101_F000AB4C_18E636C
 * score: 499/638 words
 * frame: 0x48
 * relocations: 108
 * first-mismatch: +0x10
 * summary: 499 masked at size delta 0 after L103 split of the three 0.0f pool entries on sprite nodes; per-row addiu-3 is childType=3 materialized late, not a folded L149 draw. 237 exact / 340 naming / 5 imm / 74 structural.
 * PLATEAU-HANDOFF:func_overlay_101_F000AB4C_18E636C:end
 */
