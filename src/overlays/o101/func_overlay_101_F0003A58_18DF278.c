#include "PR/ultratypes.h"

typedef struct O101RootGroup3A58 {
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
} O101RootGroup3A58;

typedef struct O101Node32_3A58 {
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
} O101Node32_3A58;

typedef struct O101Node20_3A58 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_3A58;

typedef struct O101Node24_3A58 {
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
} O101Node24_3A58;

typedef struct O101Inputs3A58 {
    void *data0;
    void *text4;
    void *text8;
    void *dataC;
    void *text10;
    void *data14;
    void *data18;
    void *text1C;
    void *text20;
    void *text24;
    void *data28;
    void *text2C;
    void *data30;
    void *text34;
    void *data38;
    void *text3C;
    void *data40;
    void *text44;
    void *data48;
    void *text4C;
    void *text50;
    void *data54;
    void *text58;
    void *text5C;
    void *text60;
    void *text64;
} O101Inputs3A58;

/* Tier B: runtime LOCAL relocation pairs and the ROM-exact cleanup/draw
 * siblings identify the order records, the order list, the resource arrays
 * and their three counters. */
extern O101RootGroup3A58 gOverlay101OrderEntries[];
extern O101Inputs3A58 D_INPUT;
extern O101RootGroup3A58 D_1C;
extern O101RootGroup3A58 D_38;
extern O101RootGroup3A58 D_54;
extern O101RootGroup3A58 D_70;
extern O101RootGroup3A58 D_8C;
extern O101RootGroup3A58 D_A8;
extern O101RootGroup3A58 D_C4;
extern O101RootGroup3A58 D_E0;
extern O101RootGroup3A58 D_FC;
extern O101RootGroup3A58 D_118;
extern O101RootGroup3A58 D_134;
extern void *D_C64;
extern void *gOverlay101Order[];
extern s32 gOverlay101OrderCount;
extern s32 gOverlay101Resource20Count;
extern s32 gOverlay101Resource32Count;
extern s32 D_1D0;
extern O101Node20_3A58 gOverlay101Resources20[];
extern O101Node32_3A58 gOverlay101Resources32[];
extern O101Node24_3A58 D_540[];

/* Tier B: runtime relocation records distinguish the sprite and screen loaders
 * from the overlay-local reset call. Their retail callees establish arity. */
extern void *texLoadSprite(s32 assetId, s32 flags);
extern s32 *func_80036DD0(s32 screenIndex);
extern s32 overlay101ByteLength(u8 *text);
/* The tail call is a SYMBOL relocation record, not a JUMP one, so it goes
 * through the generated surface name even though the callee is defined in
 * this same overlay. */
extern void overlay101ResetReloc(void *value);
/* Tier B: initialized data +0x340, distinct from the BSS node pool. */
extern u8 D_o101ResetData340[];

/* Tier A: stock IDO with this TU's R4300 multiply-scheduler flag reproduces
 * the owned text. Array expressions preserve the builder family's shared
 * address webs; group mode is initialized before geometry. See the matching
 * handoff for the compiler-flag control and linked promotion evidence. */
void func_overlay_101_F0003A58_18DF278(void) {
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

#define ADD_TEXT(group, input, textX, textY, c0, c1, c2, c3) \
    D_540[D_1D0].x = (textX); \
    D_540[D_1D0].y = (textY); \
    length = overlay101ByteLength(D_INPUT.input); \
    D_540[D_1D0].length = (u8)length; \
    D_540[D_1D0].opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].mode = 2; \
    D_540[D_1D0].color0 = (c0); \
    D_540[D_1D0].color1 = (c1); \
    D_540[D_1D0].color2 = (c2); \
    D_540[D_1D0].color3 = (c3); \
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
    gOverlay101OrderEntries[1 + 0].data18 = &D_C64; \
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
    ADD_NODE32(0, 242, 334, 1.0f, 0, 0xFF, 145);

    INIT_GROUP(1, 32, 24, 48, 36, 224, 168, data0, D_38);
    ADD_TEXT(1, text4, 112, 144, 255, 255, 255, 255);
    ADD_TEXT(1, text8, 112, 154, 255, 255, 255, 255);
    ADD_NODE20(1, -100, 14, 4);

    INIT_GROUP(2, 32, 104, 80, 57, 160, 126, dataC, D_54);
    ADD_TEXT(2, text10, 80, 112, 255, 255, 255, 255);
    ADD_NODE20(2, 32, 14, 11);

    INIT_GROUP(3, 32, 64, 58, 24, 204, 192, data14, D_70);
    ADD_NODE20(3, 6, 14, 6);

    INIT_GROUP(4, 40, -132, 40, -132, 240, 132, data18, D_8C);
    ADD_TEXT(4, text1C, 120, 96, 255, 255, 255, 255);
    ADD_TEXT(4, text20, 120, 106, 255, 255, 255, 255);
    ADD_TEXT(4, text24, 120, 116, 255, 255, 255, 255);
    ADD_NODE20(4, 80, 14, 14);

    INIT_GROUP(5, -64, -12, 64, 16, 96, 104, data28, D_A8);
    ADD_TEXT(5, text2C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(5, 8, 13, 3);

    INIT_GROUP(6, 320, -12, 160, 16, 96, 104, data30, D_C4);
    ADD_TEXT(6, text34, 48, 90, 192, 192, 255, 255);
    ADD_NODE20(6, 8, 13, 5);

    INIT_GROUP(7, 320, 240, 160, 120, 96, 104, data38, D_E0);
    ADD_TEXT(7, text3C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(7, 8, 13, 18);

    INIT_GROUP(8, -64, 240, 64, 120, 96, 104, data40, D_FC);
    ADD_TEXT(8, text44, 48, 90, 255, 255, 192, 255);
    ADD_NODE20(8, 8, 13, 8);

    INIT_GROUP(9, 76, -162, 76, -162, 172, 162, data48, D_118);
    ADD_TEXT(9, text4C, 86, 138, 192, 255, 192, 255);
    ADD_TEXT(9, text50, 86, 148, 192, 255, 192, 255);
    ADD_NODE20(9, 6, 14, 10);

    INIT_GROUP(10, 32, 144, 32, 91, 192, 62, data54, D_134);
    ADD_TEXT(10, text58, 96, 14, 0, 0, 0, 0);
    ADD_TEXT(10, text5C, 96, 24, 0, 0, 0, 0);
    ADD_TEXT(10, text60, 96, 34, 0, 0, 0, 0);
    ADD_TEXT(10, text64, 96, 48, 0, 0, 0, 0);

    overlay101ResetReloc(D_o101ResetData340);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
#undef INIT_GROUP0
}
