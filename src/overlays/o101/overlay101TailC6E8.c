#include "PR/ultratypes.h"

typedef struct O101TailC6E8Root {
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad09;
    s16 value0A;
    s16 value0C;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 color0;
    u8 color1;
    void *asset;
} O101TailC6E8Root;

typedef struct O101TailC6E8Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color0;
    u8 color1;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailC6E8Node32;

typedef struct O101TailC6E8Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101TailC6E8Node20;

extern s32 D_8007BF44;
extern u8 D_8007BF3C[];
extern s32 gO101TailC6E8PresentationActive;
extern s32 gO101TailC6E8PresentationDone;
extern O101TailC6E8Root D_0;
extern s32 gO101TailC6E8OrderCount;
extern O101TailC6E8Root *gO101TailC6E8OrderSlots[];
extern s32 gO101TailC6E8Node32Count;
extern O101TailC6E8Node32 D_340[];
extern s32 gO101TailC6E8Node20Count;
extern O101TailC6E8Node20 gO101TailC6E8Nodes20[];
extern u8 gO101TailC6E8AssetDF0;
extern void *gO101TailC6E8Handle1D4;
extern void *gO101TailC6E8Handle1F4;
extern void *gO101TailC6E8Handle33C;
extern void *gO101TailC6E8Handle338;

/* Every call in this function is a SYMBOL relocation record, so each callee
 * is a placeholder: the shipped word carries the 0xF0000000 addend. Named by
 * the target each record decodes to (overlay 101 offsets, or one resident
 * routine per name). The selector's 25-case switch table is the retained
 * rodata at rodata-relative +0xEAC; mk/overlays.mk rebinds the compiler's
 * private copy onto it. */
extern s32 levelGetNumber(void);
extern void overlay101Reset(void *value);
extern void func_overlay_101_F0003A58_18DF278(void);
extern void func_overlay_101_F000512C_18E094C(void);
extern void func_overlay_101_F000571C_18E0F3C(void);
extern void func_overlay_101_F0005E08_18E1628(void);
extern void func_overlay_101_F00063F8_18E1C18(void);
extern void func_overlay_101_F00069E8_18E2208(void);
extern void func_overlay_101_F00078F4_18E3114(void);
extern void func_overlay_101_F0008128_18E3948(void);
extern void func_overlay_101_F000895C_18E417C(void);
extern void func_overlay_101_F0009190_18E49B0(void);
extern void overlay101BuildPresentationA(void);
extern void overlay101BuildPresentationC(void);
extern void overlay101BuildPresentationB(void);
extern void overlay101BuildPresentationD(void);
extern void overlay101TailA6BC(s32 base);
extern void func_overlay_101_F000AB4C_18E636C(void);
extern void func_overlay_101_F000B544_18E6D64(void);
extern void func_overlay_101_F000BA34_18E7254(s32 variant);
extern void func_overlay_101_F000C144_18E7964(void);
extern void *texLoadSprite(s32 key, void *source);
extern void *func_80036DD0(s32 key, O101TailC6E8Node20 *node);
extern void *texLoadTexture(s32 key);
extern void *mathRnd(s32 a, s32 b);
extern void amTunePlay(s32 key);
extern void func_800015F8(void);

/* Matched 2026-10-02 (lane x-o101), 106 -> 0 at delta 0, by rewriting the
 * inherited shape in the matched siblings' (overlay101TailAB4C/A6BC) form
 * after decoding the relocation table: the selector is the queue's first
 * byte (one symbol, not two), the queue shift is a plain indexed for loop
 * whose own guard is the second count test, every node row is a one-name
 * counter subscript with the creator result stored straight into the node,
 * and the sprite creators take two arguments (the a2/a3 the shipped calls
 * carry are the root and order-count addresses left over). The first
 * candidate in this shape scored 274 at +8; dropping the redundant inner
 * count test closed it to 0. */
void func_overlay_101_F000C6E8_18E7F08(void) {
    u8 selector;
    s32 i;

    if (levelGetNumber() == 0x12) {
        overlay101Reset(NULL);
    } else {
        selector = D_8007BF3C[0];
        if (D_8007BF44 > 0) {
            D_8007BF44--;
            for (i = 0; i < D_8007BF44; i++) {
                D_8007BF3C[i] = D_8007BF3C[i + 1];
            }
        }

        switch (selector) {
            case 0:  func_overlay_101_F0003A58_18DF278(); break;
            case 1:  func_overlay_101_F000512C_18E094C(); break;
            case 2:  func_overlay_101_F000571C_18E0F3C(); break;
            case 3:  func_overlay_101_F0005E08_18E1628(); break;
            case 4:  func_overlay_101_F00063F8_18E1C18(); break;
            case 5:  func_overlay_101_F00069E8_18E2208(); break;
            case 6:  func_overlay_101_F00078F4_18E3114(); break;
            case 7:  func_overlay_101_F0008128_18E3948(); break;
            case 8:  func_overlay_101_F000895C_18E417C(); break;
            case 9:  func_overlay_101_F0009190_18E49B0(); break;
            case 10: overlay101BuildPresentationA(); break;
            case 11: overlay101BuildPresentationC(); break;
            case 12: overlay101BuildPresentationB(); break;
            case 13: overlay101BuildPresentationD(); break;
            case 14: overlay101TailA6BC(0); break;
            case 15: overlay101TailA6BC(1); break;
            case 16: overlay101TailA6BC(2); break;
            case 17: overlay101TailA6BC(3); break;
            case 18: overlay101TailA6BC(4); break;
            case 19: overlay101TailA6BC(5); break;
            case 20: func_overlay_101_F000AB4C_18E636C(); break;
            case 21: func_overlay_101_F000B544_18E6D64(); break;
            case 22: func_overlay_101_F000BA34_18E7254(0); break;
            case 23: func_overlay_101_F000BA34_18E7254(1); break;
            case 24: func_overlay_101_F000C144_18E7964(); break;
            default: overlay101Reset(NULL); break;
        }

        if (gO101TailC6E8PresentationActive != 0) {
            D_0.kind = 2; D_0.x = -0x20; D_0.y = -0x18; D_0.width = 0x180; D_0.height = 0x120; D_0.asset = &gO101TailC6E8AssetDF0; D_0.color0 = 0xFF; D_0.color1 = 0xFF; D_0.value0A = 0; D_0.value0C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailC6E8OrderSlots[gO101TailC6E8OrderCount] = &D_0; gO101TailC6E8OrderCount = gO101TailC6E8OrderCount + 1;

            D_340[gO101TailC6E8Node32Count].x = 0x35; D_340[gO101TailC6E8Node32Count].y = 0xE0; D_340[gO101TailC6E8Node32Count].value10 = 0; D_340[gO101TailC6E8Node32Count].color0 = 0xFF; D_340[gO101TailC6E8Node32Count].color1 = 0; D_340[gO101TailC6E8Node32Count].value18 = 0; D_340[gO101TailC6E8Node32Count].scale = 1.0f; D_340[gO101TailC6E8Node32Count].value14 = 0.0f; D_340[gO101TailC6E8Node32Count].handle = texLoadSprite(0x8F, NULL); D_340[gO101TailC6E8Node32Count].previousType = D_0.chainType; D_340[gO101TailC6E8Node32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101TailC6E8Node32Count]; gO101TailC6E8Node32Count = gO101TailC6E8Node32Count + 1;

            D_340[gO101TailC6E8Node32Count].x = 0xC0; D_340[gO101TailC6E8Node32Count].y = 0xE0; D_340[gO101TailC6E8Node32Count].value10 = 0; D_340[gO101TailC6E8Node32Count].color0 = 0xFF; D_340[gO101TailC6E8Node32Count].color1 = 0; D_340[gO101TailC6E8Node32Count].value18 = 0; D_340[gO101TailC6E8Node32Count].scale = 1.0f; D_340[gO101TailC6E8Node32Count].value14 = 0.0f; D_340[gO101TailC6E8Node32Count].handle = texLoadSprite(0x90, NULL); D_340[gO101TailC6E8Node32Count].previousType = D_0.chainType; D_340[gO101TailC6E8Node32Count].previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[gO101TailC6E8Node32Count]; gO101TailC6E8Node32Count = gO101TailC6E8Node32Count + 1;

            gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].x = 0x20; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].y = 0x18; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].scale = 1.0f; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].handle = func_80036DD0(1, &gO101TailC6E8Nodes20[gO101TailC6E8Node20Count]); gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].previousType = D_0.chainType; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].previous = D_0.chain; D_0.chainType = 1; D_0.chain = &gO101TailC6E8Nodes20[gO101TailC6E8Node20Count]; gO101TailC6E8Node20Count = gO101TailC6E8Node20Count + 1;

            gO101TailC6E8Handle1D4 = texLoadTexture(0x5F1);
            gO101TailC6E8Handle1F4 = texLoadTexture(0x5F2);
        }

        gO101TailC6E8Handle33C = texLoadTexture(0x58);
        gO101TailC6E8Handle338 = mathRnd(0, 0x278D00);
    }

    gO101TailC6E8PresentationDone = 0;
    amTunePlay(0x27);
    func_800015F8();
}
