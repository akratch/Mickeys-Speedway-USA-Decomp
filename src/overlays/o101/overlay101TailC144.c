#include "PR/ultratypes.h"

typedef struct O101TailC144Root {
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
} O101TailC144Root;

typedef struct O101TailC144Node32 {
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
} O101TailC144Node32;

typedef struct O101TailC144Node24 {
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
} O101TailC144Node24;

typedef struct O101TailC144Inputs {
    u8 pad000[0x84];
    void *data084;
    u8 pad088[0x128];
    u8 *text1B0;
    u8 *text1B4;
    u8 *text1B8;
    u8 *text1BC;
    u8 *text1C0;
} O101TailC144Inputs;

extern O101TailC144Root gO101TailC144Root;
extern s32 gO101TailC144OrderCount;
extern void *gO101TailC144OrderSlots[];
extern s32 gO101TailC144Node32Count;
extern O101TailC144Node32 gO101TailC144Nodes32[];
extern s32 gO101TailC144Node24Count;
extern O101TailC144Node24 gO101TailC144Nodes24[];
extern O101TailC144Inputs gO101TailC144Inputs;
extern u8 gO101TailC144AssetDCC;
extern u8 gO101TailC144FinalObject4A90;

extern void *texLoadSprite(s32 assetId, s32 flags);
extern void overlay101Reset(void *value);
extern s8 func_overlay_101_F000CEA8_18E86C8(void *text);

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative. */
/* 246 -> 209 -> 178 -> 0. The last step, lane p10-o101 2026-09-12, is L145 and
 * it also voids the three levers this comment used to record.
 * L145 -- DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX,
 *   `node32`, `node24`, `nodeIndex`, `orderIndex` and the `orderCount` pointer
 *   alike, and spell every store through the array subscript of the counter
 *   global itself. A uopt live range is formed per IR name (L131), so the
 *   subscript written on both sides of the call is ONE range spanning it and
 *   both occurrences take the caller-saved colour the shipped code uses; the
 *   counter read becomes its own range on the callee-saved register the index
 *   local used to hold; and the pre-call read lands in a ring temporary, which
 *   a local can never be, because a local is a symbol (L130). The counter bump
 *   moves to the end of its group as a consequence, since the stores now read
 *   the counter rather than a captured address. 178 -> 36 at size delta 0.
 * L146 -- every order recorded before that edit is void, and all three of the
 *   ones this comment used to carry were. The text row wanted its text store
 *   just after the opacity store, ahead of mode, from 17,481 constrained tail
 *   orders; on this shape it wants `mode` first and `text` second, which is
 *   the single swap that takes 36 -> 0. The second root header wanted height48
 *   before value4A from 18,996 head orders; on this shape the declared field
 *   order is the floor and the climb moves nothing. The extra physical line
 *   inside rows 1..4 is likewise gone: every group is one line again.
 * A four-group dependence-respecting climb -- the text row, both root headers
 * and the sprite node, with the call held as a barrier -- reaches 0 and a
 * second round is flat.
 * The promotion blocker this comment used to name, the gO101TailC144OrderCount
 * relocation identity, is resolved by the ROM: 361 of 361 words byte-exact and
 * gmake verify unchanged. */
void func_overlay_101_F000C144_18E7964(void) {
    void *handle;
    s32 length;

    gO101TailC144Root.width2E = 0x140; gO101TailC144Root.height30 = 0xF0; gO101TailC144Root.asset34 = &gO101TailC144AssetDCC; gO101TailC144Root.kind = 4; gO101TailC144Root.color32 = 0xFF; gO101TailC144Root.color33 = 0xFF; gO101TailC144Root.value26 = 0; gO101TailC144Root.value28 = 0; gO101TailC144Root.value2A = 0; gO101TailC144Root.value2C = 0; gO101TailC144Root.chainType = 0; gO101TailC144Root.chain = NULL; gO101TailC144OrderSlots[gO101TailC144OrderCount] = &gO101TailC144Root.chainType; gO101TailC144OrderCount = gO101TailC144OrderCount + 1;

    gO101TailC144Nodes32[gO101TailC144Node32Count].x = 0xF2; gO101TailC144Nodes32[gO101TailC144Node32Count].y = 0x14E; gO101TailC144Nodes32[gO101TailC144Node32Count].value10 = 0; gO101TailC144Nodes32[gO101TailC144Node32Count].color12 = 0xFF; gO101TailC144Nodes32[gO101TailC144Node32Count].color13 = 0; gO101TailC144Nodes32[gO101TailC144Node32Count].value18 = 0; gO101TailC144Nodes32[gO101TailC144Node32Count].scale = 1.0f; gO101TailC144Nodes32[gO101TailC144Node32Count].value14 = 0.0f; handle = texLoadSprite(0x93, 0); gO101TailC144Nodes32[gO101TailC144Node32Count].previousType = gO101TailC144Root.chainType; gO101TailC144Nodes32[gO101TailC144Node32Count].previous = gO101TailC144Root.chain; gO101TailC144Root.chainType = 2; gO101TailC144Root.chain = &gO101TailC144Nodes32[gO101TailC144Node32Count]; gO101TailC144Nodes32[gO101TailC144Node32Count].handle = handle; gO101TailC144Node32Count = gO101TailC144Node32Count + 1;

    gO101TailC144Root.height48 = 0x51; gO101TailC144Root.value4A = 0xCC; gO101TailC144Root.value4C = 0x4E; gO101TailC144Root.width44 = 0x18; gO101TailC144Root.x42 = 0x20; gO101TailC144Root.y46 = 0x20; gO101TailC144Root.color4E = 0xFF; gO101TailC144Root.color4F = 0xFF; gO101TailC144Root.mode40 = 0; gO101TailC144Root.childType = 0; gO101TailC144Root.child = NULL; gO101TailC144Root.data50 = gO101TailC144Inputs.data084; gO101TailC144OrderSlots[gO101TailC144OrderCount] = &gO101TailC144Root.childType; gO101TailC144OrderCount = gO101TailC144OrderCount + 1;

    gO101TailC144Nodes24[gO101TailC144Node24Count].x = 0x66; gO101TailC144Nodes24[gO101TailC144Node24Count].y = 0x10; length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B0); gO101TailC144Nodes24[gO101TailC144Node24Count].length = (u8)length; gO101TailC144Nodes24[gO101TailC144Node24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailC144Nodes24[gO101TailC144Node24Count].mode = 2; gO101TailC144Nodes24[gO101TailC144Node24Count].text = gO101TailC144Inputs.text1B0; gO101TailC144Nodes24[gO101TailC144Node24Count].color0 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color1 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color2 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color3 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].kind = 4; gO101TailC144Nodes24[gO101TailC144Node24Count].previousType = gO101TailC144Root.childType; gO101TailC144Nodes24[gO101TailC144Node24Count].previous = gO101TailC144Root.child; gO101TailC144Root.child = &gO101TailC144Nodes24[gO101TailC144Node24Count]; gO101TailC144Root.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    gO101TailC144Nodes24[gO101TailC144Node24Count].x = 0x66; gO101TailC144Nodes24[gO101TailC144Node24Count].y = 0x1E; length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B4); gO101TailC144Nodes24[gO101TailC144Node24Count].length = (u8)length; gO101TailC144Nodes24[gO101TailC144Node24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailC144Nodes24[gO101TailC144Node24Count].mode = 2; gO101TailC144Nodes24[gO101TailC144Node24Count].text = gO101TailC144Inputs.text1B4; gO101TailC144Nodes24[gO101TailC144Node24Count].color0 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color1 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color2 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color3 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].kind = 4; gO101TailC144Nodes24[gO101TailC144Node24Count].previousType = gO101TailC144Root.childType; gO101TailC144Nodes24[gO101TailC144Node24Count].previous = gO101TailC144Root.child; gO101TailC144Root.child = &gO101TailC144Nodes24[gO101TailC144Node24Count]; gO101TailC144Root.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    gO101TailC144Nodes24[gO101TailC144Node24Count].x = 0x66; gO101TailC144Nodes24[gO101TailC144Node24Count].y = 0x28; length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B8); gO101TailC144Nodes24[gO101TailC144Node24Count].length = (u8)length; gO101TailC144Nodes24[gO101TailC144Node24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailC144Nodes24[gO101TailC144Node24Count].mode = 2; gO101TailC144Nodes24[gO101TailC144Node24Count].text = gO101TailC144Inputs.text1B8; gO101TailC144Nodes24[gO101TailC144Node24Count].color0 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color1 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color2 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color3 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].kind = 4; gO101TailC144Nodes24[gO101TailC144Node24Count].previousType = gO101TailC144Root.childType; gO101TailC144Nodes24[gO101TailC144Node24Count].previous = gO101TailC144Root.child; gO101TailC144Root.child = &gO101TailC144Nodes24[gO101TailC144Node24Count]; gO101TailC144Root.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    gO101TailC144Nodes24[gO101TailC144Node24Count].x = 0x66; gO101TailC144Nodes24[gO101TailC144Node24Count].y = 0x36; length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1BC); gO101TailC144Nodes24[gO101TailC144Node24Count].length = (u8)length; gO101TailC144Nodes24[gO101TailC144Node24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailC144Nodes24[gO101TailC144Node24Count].mode = 2; gO101TailC144Nodes24[gO101TailC144Node24Count].text = gO101TailC144Inputs.text1BC; gO101TailC144Nodes24[gO101TailC144Node24Count].color0 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color1 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color2 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color3 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].kind = 4; gO101TailC144Nodes24[gO101TailC144Node24Count].previousType = gO101TailC144Root.childType; gO101TailC144Nodes24[gO101TailC144Node24Count].previous = gO101TailC144Root.child; gO101TailC144Root.child = &gO101TailC144Nodes24[gO101TailC144Node24Count]; gO101TailC144Root.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    gO101TailC144Nodes24[gO101TailC144Node24Count].x = 0x66; gO101TailC144Nodes24[gO101TailC144Node24Count].y = 0x40; length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1C0); gO101TailC144Nodes24[gO101TailC144Node24Count].length = (u8)length; gO101TailC144Nodes24[gO101TailC144Node24Count].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); gO101TailC144Nodes24[gO101TailC144Node24Count].mode = 2; gO101TailC144Nodes24[gO101TailC144Node24Count].text = gO101TailC144Inputs.text1C0; gO101TailC144Nodes24[gO101TailC144Node24Count].color0 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color1 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color2 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].color3 = 0; gO101TailC144Nodes24[gO101TailC144Node24Count].kind = 4; gO101TailC144Nodes24[gO101TailC144Node24Count].previousType = gO101TailC144Root.childType; gO101TailC144Nodes24[gO101TailC144Node24Count].previous = gO101TailC144Root.child; gO101TailC144Root.child = &gO101TailC144Nodes24[gO101TailC144Node24Count]; gO101TailC144Root.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    overlay101Reset(&gO101TailC144FinalObject4A90);
}

