#include "PR/ultratypes.h"

typedef struct Root {
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
    u8 pad54[0x130];
    void *text184;
    void *text188;
    void *text18C;
    void *text190;
    void *text194;
} Root;

typedef struct Node32 {
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
} Node32;

typedef struct Node24 {
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
    void *text;
} Node24;

typedef struct Inputs {
    u8 pad000[0x184];
    void *data184;
    void *text188;
    void *text18C;
    void *text190;
    void *text194;
} Inputs;

extern Root D_0;
extern Inputs D_INPUT;
extern void *D_1C;
extern void *D_38;
extern void *D_DA4;
extern void *D_4358;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1CC;
extern s32 D_1D0;
extern Node32 D_340[];
extern Node24 D_540[];

extern void *texLoadSprite(s32 assetId, s32 flags);
extern void overlay101Reset(void *value);
extern s32 func_overlay_101_F000CEA8_18E86C8(void *);

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C TU;
 * this consumer uses that owner for the root and node pools. */
/* 158 -> 0, lane p10-o101 2026-09-12, on the lever that closed the rest of
 * this overlay's builder family, and the residual was never four separate
 * problems -- it was one carrier.
 * L145 -- DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX.
 *   `index`, `node32` and `node24` are gone and every store is spelled through
 *   the array subscript of the counter global itself. A uopt live range is
 *   formed per IR name (L131), so the subscript written on both sides of the
 *   call is ONE range spanning it, and both occurrences take the caller-saved
 *   colour the shipped code uses; the counter read becomes its own range on the
 *   callee-saved register the index local used to hold; and the pre-call read
 *   lands in a ring temporary, which a local can never be (L130). The bump
 *   moves to the end of its group, since the stores now read the counter
 *   rather than a captured address. 158 -> 2 at size delta 0.
 * L146 -- every field order this file carried was an optimum of the carrier
 *   shape and none of them survives it. All four text rows take the declared
 *   field order now, one physical line each, including the two that had been
 *   expanded one statement per line and the one that carried a scrambled
 *   colour order; the two root headers likewise. The only order that is not
 *   the declared one is in the sprite node, where color13 precedes value18,
 *   and that single swap is the last 2 words. A dependence-respecting climb
 *   over all four groups with the call held as a barrier reaches 0 and a
 *   second round is flat. */
void func_overlay_101_F000B544_18E6D64(void) {
    s32 length;
    void *handle;

    D_0.kind = 4; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.width2E = 0x140; D_0.value2C = 0; D_0.chainType = 0; D_0.height30 = 0xF0; D_0.asset34 = &D_DA4; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 = D_1C4 + 1;

    D_340[D_1CC].x = 0x108; D_340[D_1CC].scale = 1.0f; D_340[D_1CC].value10 = 0; D_340[D_1CC].color12 = 0xFF; D_340[D_1CC].color13 = 0; D_340[D_1CC].value18 = 0; D_340[D_1CC].y = 0x154; D_340[D_1CC].value14 = 0.0f; handle = texLoadSprite(0x91, 0); D_340[D_1CC].previousType = D_0.chainType; D_340[D_1CC].previous = D_0.chain; D_340[D_1CC].handle = handle; D_0.chainType = 2; D_0.chain = &D_340[D_1CC]; D_1CC = D_1CC + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x1C; D_0.height48 = 0x50; D_0.value4A = 0xD8; D_0.value4C = 0x42; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = 0; D_0.data50 = D_INPUT.data184; D_1C0[D_1C4] = &D_38; D_1C4 = D_1C4 + 1;

    D_540[D_1D0].x = 0x6C; D_540[D_1D0].y = 0x10; length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text188); D_540[D_1D0].length = (u8)length; D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].mode = 2; D_540[D_1D0].color0 = 0xFF; D_540[D_1D0].color1 = 0x80; D_540[D_1D0].color2 = 0; D_540[D_1D0].color3 = 0xFF; D_540[D_1D0].kind = 4; D_540[D_1D0].text = D_INPUT.text188; D_540[D_1D0].previousType = D_0.childType; D_540[D_1D0].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0]; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].x = 0x6C; D_540[D_1D0].y = 0x1E; length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text18C); D_540[D_1D0].length = (u8)length; D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].mode = 2; D_540[D_1D0].color0 = 0xFF; D_540[D_1D0].color1 = 0xFF; D_540[D_1D0].color2 = 0; D_540[D_1D0].color3 = 0xFF; D_540[D_1D0].kind = 4; D_540[D_1D0].text = D_INPUT.text18C; D_540[D_1D0].previousType = D_0.childType; D_540[D_1D0].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0]; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].x = 0x6C; D_540[D_1D0].y = 0x28; length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text190); D_540[D_1D0].length = (u8)length; D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].mode = 2; D_540[D_1D0].color0 = 0xFF; D_540[D_1D0].color1 = 0xFF; D_540[D_1D0].color2 = 0; D_540[D_1D0].color3 = 0xFF; D_540[D_1D0].kind = 4; D_540[D_1D0].text = D_INPUT.text190; D_540[D_1D0].previousType = D_0.childType; D_540[D_1D0].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0]; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].x = 0x6C; D_540[D_1D0].y = 0x32; length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text194); D_540[D_1D0].length = (u8)length; D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].mode = 2; D_540[D_1D0].color0 = 0xFF; D_540[D_1D0].color1 = 0xFF; D_540[D_1D0].color2 = 0; D_540[D_1D0].color3 = 0xFF; D_540[D_1D0].kind = 4; D_540[D_1D0].text = D_INPUT.text194; D_540[D_1D0].previousType = D_0.childType; D_540[D_1D0].previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0]; D_1D0 = D_1D0 + 1;

    overlay101Reset(&D_4358);
}

