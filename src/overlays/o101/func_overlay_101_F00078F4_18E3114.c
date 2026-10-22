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
    u8 pad00[0xF8];
    void *dataF8;
    void *textFC;
    void *text100;
    void *text104;
} Inputs;

extern Root D_0;
extern Inputs D_INPUT;
/* The builder flags word sits at the head of the same shared block. It has
 * to be its own symbol rather than a field of D_0 or D_INPUT: the shipped
 * code materialises a separate base register for it, and spelling it as a
 * field lets uopt share one of theirs and drops five instructions. The
 * per-overlay name is required -- overlay 55 already publishes a D_F4 at a
 * different offset, and a shared generic name silently mis-relocates it. */
extern u32 D_800D3128;
extern void *D_1C;
extern void *D_38;
extern void *D_CDC;
extern void *D_2608;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1CC;
extern s32 D_1D0;
extern Node32 D_340[];
extern Node24 D_540[];

extern void *texLoadSprite(s32 assetId, s32 flags);
extern void overlay101Reset(void *value);
extern s8 func_overlay_101_F000CEA8_18E86C8(void *);

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C
 * TU; this consumer uses that owner for the root and node pools. */
/* Matched by deleting every local that held an element address or an element
 * index, and spelling all six store groups -- the chain node, the eight image
 * nodes and the three text rows -- through the array subscript of the counter
 * global itself.
 *
 * A uopt live range is formed per IR name (L131), so `&D_340[D_1CC]` written
 * on both sides of a call is ONE range whose hull spans the call, and a
 * pointer or index LOCAL is a symbol, which is coloured from the procedure's
 * table or given a stack home and is never a ring temporary.  That is what
 * held the earlier shapes apart from the shipped code in two ways at once.
 * Writing `D_340[D_1CC].field` instead gives, per element:
 *   - one range for the address expression, pre-call and post-call, so both
 *     occurrences take the one caller-saved colour the shipped code uses;
 *   - one range for the counter load, spanning all three rows, which is what
 *     takes the callee-saved register the earlier shapes gave to the address;
 *   - and the pre-call read of the counter lands in a ring temporary instead
 *     of a coloured symbol, which is the ring draw (L129) the index-local
 *     shapes were missing -- they ran a FIFO position behind from the head of
 *     the text region onward.
 * Measured on this function: index locals everywhere 179, one row reverted to
 * the direct read 155, all rows direct but the post-call stores still through
 * a shared index local 165, no locals at all 98.
 *
 * The remaining 98 were two statement orders, both of which had to be
 * re-climbed because a statement-order optimum is not portable across a shape
 * change (L59: each group is one physical line, so as1's `lineno` tie is
 * retired and the raw ready-list order supplies the reversal):
 *   - the image node's post-call group wants the handle store FIRST, ahead of
 *     the two old-link reads: 98 -> 26.  The earlier 630-order exhaustion was
 *     an optimum of the pointer-local shape and does not survive this one.
 *   - the second root header's nine geometry stores want width, height, mode,
 *     x, colour, colour, y, then the two values: 26 -> 0.
 *
 * All four siblings in this cluster take the identical edit at 0 words. */
void func_overlay_101_F00078F4_18E3114(void) {
    void *handle;
    s32 length;
    s32 ownerIndex;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_CDC; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_1C; D_1C4 = ownerIndex + 1;

    D_340[D_1CC].x = 0x4E; D_340[D_1CC].y = 0x14E; D_340[D_1CC].value10 = 0; D_340[D_1CC].color12 = 0xFF; D_340[D_1CC].color13 = 0; D_340[D_1CC].value18 = 0; D_340[D_1CC].scale = 1.0f; D_340[D_1CC].value14 = 0.0f; handle = texLoadSprite(0x92, 0); D_340[D_1CC].handle = handle; D_340[D_1CC].previousType = D_0.chainType; D_340[D_1CC].previous = D_0.chain; D_0.chain = &D_340[D_1CC]; D_0.chainType = 2; D_1CC = D_1CC + 1;

    D_0.width44 = 0x18; D_0.height48 = 0x14; D_0.mode40 = 0; D_0.x42 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.y46 = 0x20; D_0.value4A = 0x100; D_0.value4C = 0xB4; D_0.data50 = D_INPUT.dataF8; D_0.childType = 0; D_0.child = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_38; D_1C4 = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId) \
    D_340[D_1CC].x = (nodeX); \
    D_340[D_1CC].y = (nodeY); \
    D_340[D_1CC].scale = 1.0f; \
    D_340[D_1CC].value10 = 0; \
    D_340[D_1CC].color12 = 0xFF; \
    D_340[D_1CC].color13 = 0; \
    D_340[D_1CC].value14 = 0.0f; \
    D_340[D_1CC].value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[D_1CC].handle = handle; \
    D_340[D_1CC].previousType = D_0.childType; \
    D_0.childType = 2; \
    D_340[D_1CC].previous = D_0.child; \
    D_0.child = &D_340[D_1CC]; \
    D_1CC = D_1CC + 1

    if (((D_800D3128 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x98);
    }
    if (((D_800D3128 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x9A);
    }
    if (((D_800D3128 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9E);
    }
    if (((D_800D3128 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY) \
    D_540[D_1D0].x = 0x80; \
    D_540[D_1D0].y = (rowY); \
    length = func_overlay_101_F000CEA8_18E86C8(field); \
    D_540[D_1D0].length = (u8)length; \
    D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].mode = 2; \
    D_540[D_1D0].color0 = 0; \
    D_540[D_1D0].color1 = 0; \
    D_540[D_1D0].color2 = 0; \
    D_540[D_1D0].color3 = 0; \
    D_540[D_1D0].kind = 4; \
    D_540[D_1D0].text = (field); \
    D_540[D_1D0].previousType = D_0.childType; \
    D_540[D_1D0].previous = D_0.child; \
    D_0.childType = 3; \
    D_0.child = &D_540[D_1D0]; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.textFC, 0x92);
    ADD_TEXT_ROW(D_INPUT.text100, 0x9C);
    ADD_TEXT_ROW(D_INPUT.text104, 0xA6);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_2608);
}

