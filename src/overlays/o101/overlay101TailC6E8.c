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

extern s32 gO101TailC6E8QueueCount;
extern u8 gO101TailC6E8Queue[];
extern s32 gO101TailC6E8PresentationActive;
extern s32 gO101TailC6E8PresentationDone;
extern O101TailC6E8Root gO101TailC6E8Root;
extern s32 gO101TailC6E8OrderCount;
extern O101TailC6E8Root *gO101TailC6E8OrderSlots[];
extern s32 gO101TailC6E8Node32Count;
extern O101TailC6E8Node32 gO101TailC6E8Nodes32[];
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
extern s32 o101TailC6E8StateReloc(void);
extern void o101TailC6E8ResetReloc(void *value);
extern void o101TailC6E8Run3A58Reloc(void);
extern void o101TailC6E8Run512CReloc(void);
extern void o101TailC6E8Run571CReloc(void);
extern void o101TailC6E8Run5E08Reloc(void);
extern void o101TailC6E8Run63F8Reloc(void);
extern void o101TailC6E8Run69E8Reloc(void);
extern void o101TailC6E8Run78F4Reloc(void);
extern void o101TailC6E8Run8128Reloc(void);
extern void o101TailC6E8Run895CReloc(void);
extern void o101TailC6E8Run9190Reloc(void);
extern void o101TailC6E8PresentationAReloc(void);
extern void o101TailC6E8PresentationCReloc(void);
extern void o101TailC6E8PresentationBReloc(void);
extern void o101TailC6E8PresentationDReloc(void);
extern void o101TailC6E8TailA6BCReloc(s32 base);
extern void o101TailC6E8TailAB4CReloc(void);
extern void o101TailC6E8TailB544Reloc(void);
extern void o101TailC6E8TailBA34Reloc(s32 variant);
extern void o101TailC6E8TailC144Reloc(void);
extern void *o101TailC6E8CreatorReloc(s32 key, void *source);
extern void *o101TailC6E8CreateCompactReloc(s32 key, O101TailC6E8Node20 *node);
extern void *o101TailC6E8AcquireReloc(s32 key);
extern void *o101TailC6E8Acquire338Reloc(s32 a, s32 b);
extern void o101TailC6E8Tail27Reloc(s32 key);
extern void o101TailC6E8TailFinalReloc(void);

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

    if (o101TailC6E8StateReloc() == 0x12) {
        o101TailC6E8ResetReloc(NULL);
    } else {
        selector = gO101TailC6E8Queue[0];
        if (gO101TailC6E8QueueCount > 0) {
            gO101TailC6E8QueueCount--;
            for (i = 0; i < gO101TailC6E8QueueCount; i++) {
                gO101TailC6E8Queue[i] = gO101TailC6E8Queue[i + 1];
            }
        }

        switch (selector) {
            case 0:  o101TailC6E8Run3A58Reloc(); break;
            case 1:  o101TailC6E8Run512CReloc(); break;
            case 2:  o101TailC6E8Run571CReloc(); break;
            case 3:  o101TailC6E8Run5E08Reloc(); break;
            case 4:  o101TailC6E8Run63F8Reloc(); break;
            case 5:  o101TailC6E8Run69E8Reloc(); break;
            case 6:  o101TailC6E8Run78F4Reloc(); break;
            case 7:  o101TailC6E8Run8128Reloc(); break;
            case 8:  o101TailC6E8Run895CReloc(); break;
            case 9:  o101TailC6E8Run9190Reloc(); break;
            case 10: o101TailC6E8PresentationAReloc(); break;
            case 11: o101TailC6E8PresentationCReloc(); break;
            case 12: o101TailC6E8PresentationBReloc(); break;
            case 13: o101TailC6E8PresentationDReloc(); break;
            case 14: o101TailC6E8TailA6BCReloc(0); break;
            case 15: o101TailC6E8TailA6BCReloc(1); break;
            case 16: o101TailC6E8TailA6BCReloc(2); break;
            case 17: o101TailC6E8TailA6BCReloc(3); break;
            case 18: o101TailC6E8TailA6BCReloc(4); break;
            case 19: o101TailC6E8TailA6BCReloc(5); break;
            case 20: o101TailC6E8TailAB4CReloc(); break;
            case 21: o101TailC6E8TailB544Reloc(); break;
            case 22: o101TailC6E8TailBA34Reloc(0); break;
            case 23: o101TailC6E8TailBA34Reloc(1); break;
            case 24: o101TailC6E8TailC144Reloc(); break;
            default: o101TailC6E8ResetReloc(NULL); break;
        }

        if (gO101TailC6E8PresentationActive != 0) {
            gO101TailC6E8Root.kind = 2; gO101TailC6E8Root.x = -0x20; gO101TailC6E8Root.y = -0x18; gO101TailC6E8Root.width = 0x180; gO101TailC6E8Root.height = 0x120; gO101TailC6E8Root.asset = &gO101TailC6E8AssetDF0; gO101TailC6E8Root.color0 = 0xFF; gO101TailC6E8Root.color1 = 0xFF; gO101TailC6E8Root.value0A = 0; gO101TailC6E8Root.value0C = 0; gO101TailC6E8Root.chainType = 0; gO101TailC6E8Root.chain = NULL; gO101TailC6E8OrderSlots[gO101TailC6E8OrderCount] = &gO101TailC6E8Root; gO101TailC6E8OrderCount = gO101TailC6E8OrderCount + 1;

            gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].x = 0x35; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].y = 0xE0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value10 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].color0 = 0xFF; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].color1 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value18 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].scale = 1.0f; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value14 = 0.0f; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].handle = o101TailC6E8CreatorReloc(0x8F, NULL); gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].previousType = gO101TailC6E8Root.chainType; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].previous = gO101TailC6E8Root.chain; gO101TailC6E8Root.chainType = 2; gO101TailC6E8Root.chain = &gO101TailC6E8Nodes32[gO101TailC6E8Node32Count]; gO101TailC6E8Node32Count = gO101TailC6E8Node32Count + 1;

            gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].x = 0xC0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].y = 0xE0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value10 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].color0 = 0xFF; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].color1 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value18 = 0; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].scale = 1.0f; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].value14 = 0.0f; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].handle = o101TailC6E8CreatorReloc(0x90, NULL); gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].previousType = gO101TailC6E8Root.chainType; gO101TailC6E8Nodes32[gO101TailC6E8Node32Count].previous = gO101TailC6E8Root.chain; gO101TailC6E8Root.chainType = 2; gO101TailC6E8Root.chain = &gO101TailC6E8Nodes32[gO101TailC6E8Node32Count]; gO101TailC6E8Node32Count = gO101TailC6E8Node32Count + 1;

            gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].x = 0x20; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].y = 0x18; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].scale = 1.0f; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].handle = o101TailC6E8CreateCompactReloc(1, &gO101TailC6E8Nodes20[gO101TailC6E8Node20Count]); gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].previousType = gO101TailC6E8Root.chainType; gO101TailC6E8Nodes20[gO101TailC6E8Node20Count].previous = gO101TailC6E8Root.chain; gO101TailC6E8Root.chainType = 1; gO101TailC6E8Root.chain = &gO101TailC6E8Nodes20[gO101TailC6E8Node20Count]; gO101TailC6E8Node20Count = gO101TailC6E8Node20Count + 1;

            gO101TailC6E8Handle1D4 = o101TailC6E8AcquireReloc(0x5F1);
            gO101TailC6E8Handle1F4 = o101TailC6E8AcquireReloc(0x5F2);
        }

        gO101TailC6E8Handle33C = o101TailC6E8AcquireReloc(0x58);
        gO101TailC6E8Handle338 = o101TailC6E8Acquire338Reloc(0, 0x278D00);
    }

    gO101TailC6E8PresentationDone = 0;
    o101TailC6E8Tail27Reloc(0x27);
    o101TailC6E8TailFinalReloc();
}
