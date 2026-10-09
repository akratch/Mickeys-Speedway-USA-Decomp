#include "PR/ultratypes.h"

typedef struct O101RootGroup {
    s32 childType;
    void *child;
    u8 mode;
    u8 pad09;
    s16 x;
    s16 width;
    s16 y;
    s16 height;
    s16 value12;
    s16 value14;
    u8 color16;
    u8 color17;
    void *data18;
} O101RootGroup;

typedef struct O101Node32_69E8 {
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
} O101Node32_69E8;

typedef struct O101Node20_69E8 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_69E8;

typedef struct O101Node24_69E8 {
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
} O101Node24_69E8;

typedef struct O101Inputs69E8 {
    u8 pad000[0xC8];
    void *dataC8;
    u8 *textCC;
    u8 *textD0;
    void *dataD4;
    void *dataD8;
    u8 *textDC;
    u8 *textE0;
    void *dataE4;
    u8 *textE8;
    u8 *textEC;
    void *dataF0;
    u8 *textF4;
} O101Inputs69E8;

/* Tier B: runtime LOCAL relocation pairs and ROM-exact cleanup/draw
 * siblings identify the order records, order list, resource arrays and
 * their three counters. The text-node pool and input block are separate
 * objects; equal encoded addends never establish an alias. */
extern O101RootGroup gOverlay101OrderEntries[];
extern O101Inputs69E8 D_INPUT;
extern O101RootGroup D_1C;
extern O101RootGroup D_38;
extern O101RootGroup D_54;
extern O101RootGroup D_70;
extern O101RootGroup D_8C;
extern O101RootGroup D_A8;
extern u8 D_CC8[];
extern u8 D_1D2C[];
extern void *gOverlay101Order[];
extern s32 gOverlay101OrderCount;
extern s32 gOverlay101Resource20Count;
extern s32 gOverlay101Resource32Count;
extern s32 D_1D0;
extern O101Node20_69E8 gOverlay101Resources20[];
extern O101Node32_69E8 gOverlay101Resources32[];
extern O101Node24_69E8 D_540[];

/* Tier B: this function's runtime relocation records resolve ten sprite
 * loads to resident texLoadSprite, four screen loads to func_80036DD0,
 * seven local calls to overlay101ByteLength, and the final call to
 * overlay101Reset. The matched local callees establish their prototypes. */
extern void *texLoadSprite(s32 assetId, s32 flags);
extern s32 *func_80036DD0(s32 screenIndex);
extern s32 overlay101ByteLength(u8 *text);
/* The tail call is a SYMBOL relocation record, not a JUMP one: the shipped
 * word carries the 0xF0000000 addend rather than offset >> 2, so it must go
 * through the generated surface name even though the callee is defined in
 * this same overlay. Naming the in-module definition scores 0 masked words
 * and still builds a different ROM, by exactly this one word. */
extern void overlay101ResetReloc(void *value);

/*
 * Mickey-local reconstruction from this overlay's extracted function and
 * the typed node builders immediately before and after it. The pinned DKR
 * v77/v80 and JFG overlay scans report no donor for overlay 101.
 *
 * 613 masked words -> 0, lane p10-o101 2026-09-12, on three edits and no
 * new identity. The shape is the one that closed the rest of this overlay's
 * builder family, and the previous handoff's structural reading -- an
 * exchanged saved-register pair plus a scratch ring one slot behind from
 * +0x88 -- was a consequence of the carrier, not a separate problem.
 *
 * L145. DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX
 *   and spell each store through the array subscript of the counter global
 *   itself. `index`, `orderIndex`, `node20`, `node24` and `node32` are all
 *   gone. A uopt live range is formed per IR name (L131), so the subscript
 *   written on both sides of a call is ONE range spanning the call and both
 *   occurrences take the caller-saved colour the shipped code uses; the
 *   counter read becomes its own range and takes the callee-saved register
 *   the index local held; and the pre-call read lands in a ring temporary,
 *   which a local can never be (L130, a local is a symbol). Worth 613 -> 13
 *   at size delta 0 on its own. The four builder blocks that were written out
 *   longhand are folded back into their macros as part of it, so every group
 *   is one physical line again and as1's `lineno` tie-break is retired (L59,
 *   L132); on the local-carrier shape that same folding measured 623, ten
 *   words WORSE, which is L146 exactly -- an order or a layout measured
 *   before a shape change says nothing after it.
 * L146, twice more, and both directions were already written down here as
 *   settled. The first sprite node wants x BEFORE y: the comment this one
 *   replaces recorded the opposite, measured on the carrier shape, and it was
 *   right there and wrong here. 13 -> 9. The group header wants `mode` FIRST,
 *   ahead of the geometry stores, where it used to sit seventh; sweeping it
 *   across the header reads 0, 0, 6, 7, 9, 9, 9, 16, 21, 26, 31, 31 from
 *   first to last, and only groups 2, 4 and 5 were mis-scheduled at the old
 *   position. 9 -> 0.
 *
 * Tier B identities are unchanged from the reconstruction: the runtime table
 * resolves ten sprite loads to texLoadSprite, four screen loads to
 * func_80036DD0, seven local calls to overlay101ByteLength and the tail call
 * to overlay101Reset; ROM-exact overlay101Cleanup and overlay101DrawSlots
 * witness the order records, the order list and count, and the two resource
 * arrays with their counters. 963 of 963 words byte-exact, frame 0x50,
 * 110 of 110 relocations aligned.
 */
void func_overlay_101_F00069E8_18E2208(void) {
    register s32 commonY;
    s32 length;
    void *handle;

#define INIT_GROUP(group, px, pwidth, py, pheight, value0, value1, input, owner) \
    gOverlay101OrderEntries[1 + group].mode = 0; \
    gOverlay101OrderEntries[1 + group].x = (px); \
    gOverlay101OrderEntries[1 + group].width = (pwidth); \
    gOverlay101OrderEntries[1 + group].y = (py); \
    gOverlay101OrderEntries[1 + group].height = (pheight); \
    gOverlay101OrderEntries[1 + group].value12 = (value0); \
    gOverlay101OrderEntries[1 + group].value14 = (value1); \
    gOverlay101OrderEntries[1 + group].color16 = 0xFF; \
    gOverlay101OrderEntries[1 + group].color17 = 0xFF; \
    gOverlay101OrderEntries[1 + group].childType = 0; \
    gOverlay101OrderEntries[1 + group].child = 0; \
    gOverlay101OrderEntries[1 + group].data18 = D_INPUT.input; \
    gOverlay101Order[gOverlay101OrderCount] = &(owner); \
    gOverlay101OrderCount = gOverlay101OrderCount + 1

#define ADD_NODE32(group, nodeX, nodeY, nodeScale, nodeValue, nodeColor, imageId) \
    gOverlay101Resources32[gOverlay101Resource32Count].x = (nodeX); \
    gOverlay101Resources32[gOverlay101Resource32Count].y = (nodeY); \
    gOverlay101Resources32[gOverlay101Resource32Count].scale = (nodeScale); \
    gOverlay101Resources32[gOverlay101Resource32Count].value10 = (nodeValue); \
    gOverlay101Resources32[gOverlay101Resource32Count].color12 = (nodeColor); \
    gOverlay101Resources32[gOverlay101Resource32Count].color13 = 0; \
    gOverlay101Resources32[gOverlay101Resource32Count].value14 = 0.0f; \
    gOverlay101Resources32[gOverlay101Resource32Count].value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    gOverlay101Resources32[gOverlay101Resource32Count].previousType = gOverlay101OrderEntries[1 + group].childType; \
    gOverlay101Resources32[gOverlay101Resource32Count].previous = gOverlay101OrderEntries[1 + group].child; \
    gOverlay101Resources32[gOverlay101Resource32Count].handle = handle; \
    gOverlay101OrderEntries[1 + group].childType = 2; \
    gOverlay101OrderEntries[1 + group].child = &gOverlay101Resources32[gOverlay101Resource32Count]; \
    gOverlay101Resource32Count = gOverlay101Resource32Count + 1

#define ADD_NODE20(group, nodeX, nodeY, imageId) \
    gOverlay101Resources20[gOverlay101Resource20Count].x = (nodeX); \
    gOverlay101Resources20[gOverlay101Resource20Count].y = (nodeY); \
    gOverlay101Resources20[gOverlay101Resource20Count].scale = 1.0f; \
    handle = func_80036DD0(imageId); \
    gOverlay101Resources20[gOverlay101Resource20Count].previousType = gOverlay101OrderEntries[1 + group].childType; \
    gOverlay101Resources20[gOverlay101Resource20Count].previous = gOverlay101OrderEntries[1 + group].child; \
    gOverlay101Resources20[gOverlay101Resource20Count].handle = handle; \
    gOverlay101OrderEntries[1 + group].childType = 1; \
    gOverlay101OrderEntries[1 + group].child = &gOverlay101Resources20[gOverlay101Resource20Count]; \
    gOverlay101Resource20Count = gOverlay101Resource20Count + 1

#define ADD_TEXT(group, input, textX, textY) \
    D_540[D_1D0].x = (textX); \
    D_540[D_1D0].y = (textY); \
    length = overlay101ByteLength(D_INPUT.input); \
    D_540[D_1D0].length = (u8)length; \
    D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].mode = 2; \
    D_540[D_1D0].color0 = 0xFF; \
    D_540[D_1D0].color1 = 0xFF; \
    D_540[D_1D0].color2 = 0xFF; \
    D_540[D_1D0].color3 = 0xFF; \
    D_540[D_1D0].kind = 4; \
    D_540[D_1D0].text = D_INPUT.input; \
    D_540[D_1D0].previousType = gOverlay101OrderEntries[1 + group].childType; \
    D_540[D_1D0].previous = gOverlay101OrderEntries[1 + group].child; \
    gOverlay101OrderEntries[1 + group].childType = 3; \
    gOverlay101OrderEntries[1 + group].child = &D_540[D_1D0]; \
    D_1D0 = D_1D0 + 1

#define INIT_GROUP0() \
    gOverlay101OrderEntries[1 + 0].mode = 4; \
    gOverlay101OrderEntries[1 + 0].value12 = 0x140; \
    gOverlay101OrderEntries[1 + 0].value14 = 0xF0; \
    gOverlay101OrderEntries[1 + 0].data18 = &D_CC8; \
    gOverlay101OrderEntries[1 + 0].color16 = 0xFF; \
    gOverlay101OrderEntries[1 + 0].color17 = 0xFF; \
    gOverlay101OrderEntries[1 + 0].x = 0; \
    gOverlay101OrderEntries[1 + 0].width = 0; \
    gOverlay101OrderEntries[1 + 0].y = 0; \
    gOverlay101OrderEntries[1 + 0].height = 0; \
    gOverlay101OrderEntries[1 + 0].childType = 0; \
    gOverlay101OrderEntries[1 + 0].child = 0; \
    gOverlay101Order[gOverlay101OrderCount] = &D_1C; \
    gOverlay101OrderCount = gOverlay101OrderCount + 1

    INIT_GROUP0();
    ADD_NODE32(0, 0xF2, 0x14E, 1.0f, 0, 0xFF, 0x91);
    commonY = 0x78;
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x94);

    INIT_GROUP(1, 0x20, 0x18, 0x30, 0x10, 0xE0, 0xC0, dataC8, D_38);
    ADD_NODE20(1, 0xC, 0xE, 0xD);
    ADD_TEXT(1, textCC, 0x70, 0xA8);
    ADD_TEXT(1, textD0, 0x70, 0xB2);

    INIT_GROUP(2, 0x20, 0x40, 0x62, 0x18, 0x7C, 0x84, dataD4, D_54);
    ADD_NODE20(2, 0x16, 0x1E, 0x10);

    INIT_GROUP(3, 0x20, 0x68, 0x40, 0xA0, 0xC0, 0x26, dataD8, D_70);
    ADD_TEXT(3, textDC, 0x60, 0xE);
    ADD_TEXT(3, textE0, 0x60, 0x18);

    INIT_GROUP(4, 0x20, 0x90, 0x2F, 0x10, 0xE2, 0xC0, dataE4, D_8C);
    ADD_NODE20(4, 0xD, 0xE, 0x14);
    ADD_TEXT(4, textE8, 0x71, 0xA8);
    ADD_TEXT(4, textEC, 0x71, 0xB2);

    INIT_GROUP(5, 0x90, -0x50, 0x30, 0x30, 0xE0, 0x8C, dataF0, D_A8);
    ADD_NODE20(5, 0xC, 0x12, 0xC);
    ADD_TEXT(5, textF4, 0x70, 0x7B);

    overlay101ResetReloc(&D_1D2C);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
#undef INIT_GROUP0
}


