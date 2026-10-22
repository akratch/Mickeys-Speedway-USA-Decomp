#include "PR/ultratypes.h"

typedef struct O57MiddleRenderItem {
    s32 type;
    u8 value04;
    u8 value05;
    u8 value06;
    u8 value07;
} O57MiddleRenderItem;

typedef struct O57MiddleInfo {
    u8 pad00[8];
    u8 value08;
    u8 value09;
    u8 value0A;
} O57MiddleInfo;

typedef struct O57MiddleChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 active;
    u8 pad2B[9];
} O57MiddleChoice;

typedef struct O57MiddleOutput {
    u8 value0;
    u8 pad01[3];
    u8 controller;
    u8 pad05[0x23];
} O57MiddleOutput;

typedef struct O57MiddleFlags {
    u32 pad0 : 13;
    u32 allUnlocked : 1;
    u32 pad14 : 1;
    u32 pending : 1;
    u32 low : 16;
} O57MiddleFlags;

typedef struct O57MiddleTextureNode {
    void *texture;
    void *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} O57MiddleTextureNode;

/* Tier B: calls and globals separated by overlay 57 runtime relocations.
 * Reserved data/BSS selectors remain separate from overlay-local storage. */
extern O57MiddleFlags D_800D3128;
extern s16 D_800D31BC, D_800D31BE;
extern s32 D_800D31B8, D_8007C1A0;
extern s16 D_8007C0E8[], D_8007C11C[];
extern char **D_8007C0B8;
extern void *D_800D3140, *D_800D3144;
extern void *D_800D31C8[];
extern O57MiddleChoice gO57MiddleChoices[];
extern s32 D_8007BF44, gO57MiddleData31E8, gO57MiddleData31EC;
extern s32 gO57MiddleData31F0, gO57MiddleData31F4;
extern u8 D_8007BEF8, D_8007BF0C;
extern s8 D_8007BEFC, D_8007BF04, D_8007BF08;
extern u16 gO57MiddleData31B4, gO57MiddleData31B8;
extern u8 gO57MiddleData3208, gO57MiddleData3214;
extern void *gO57MiddleItems[];
extern s32 gO57MiddleAlpha, gO57MiddlePreviousAlpha;
extern s32 gO57MiddleMode, gO57MiddleNextMode, gO57MiddleMoving;
extern s32 gO57MiddleFade, gO57MiddleFadeDelay;
extern s32 gO57MiddleSelection, gO57MiddlePreviousSelection;
extern s32 gO57MiddleState188, gO57MiddleState194;
extern s32 gO57MiddleUnlocked198, gO57MiddleUnlocked19C;
extern O57MiddleInfo gO57MiddleInfo;
extern s32 gO57MiddleStopList[], gO57MiddlePathIndices[], gO57MiddlePathList[];
extern O57MiddleOutput *gO57MiddleOutput;
extern s32 gO57MiddlePanelPosition, gO57MiddleTransition, gO57MiddleCanLeave;
extern s16 gO57MiddleColumns[];
extern char gO57MiddleEmptyFormat[], gO57MiddleResultFormat[];
extern O57MiddleTextureNode gO57MiddleCaption[], gO57MiddleBadge[];
extern char *gO57MiddleLabels[];
extern s16 gO57MiddleCharacterIds[];
extern u8 gO57MiddlePathId;

typedef struct O57MiddleRenderParameters {
    u8 pad00[0x10C];
    f32 position;
    f32 scale;
} O57MiddleRenderParameters;
extern O57MiddleRenderParameters D_800D3550;

extern void func_80000F94(u16 soundId, void **handle);
extern void func_80005548(u8 count);
extern void func_80022A50(void **displayList, void **matrices);
extern void func_80025444(s8 *players);
extern s32 func_80025D60(s32 course);
extern void func_80028374(s32 level, s32 character, s32 animation, s32 mode, s32 arg4, s32 arg5);
extern void func_80028528(s32 group);
extern void func_80028540(s32 cameras);
extern void func_80028D24(s32 mode);
extern void func_800291B4(void);
typedef struct O57MiddleRecord {
    O57MiddleRenderItem items[4];
} O57MiddleRecord;
extern O57MiddleRecord *func_800291C4(void);
extern void func_8002F618(void **displayList, O57MiddleTextureNode *nodes, s32 x, s32 y, u8 r, u8 g, u8 b, u8 a);
extern void func_8002FB34(void **displayList, O57MiddleTextureNode *nodes, f32 x, f32 y, f32 sx, f32 sy, s32 mode, s32 flags);
extern void frontDrawObj(s32 spacing);
extern void func_8003A680(u8 character);
extern s32 func_8003A700(u8 character);
extern s32 func_800429A4(char *buffer, const char *format, ...);
extern void fontUseFont(s32 font);
extern void func_8004B0B8(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void fontPrintXY(void **displayList, s32 x, s32 y, char *text, s32 align);
extern void func_80050688(u8 path);
extern void func_80050704(u8 path);
extern void overlay45ConfigureLayout(void *descriptor, s32 x, s32 y, s32 flags);
extern void overlay56SplitTime(s32 time, s32 *first, s32 *second, s32 *third);
extern void func_overlay_057_F0001020_18A4C18(s32 updateRate);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern s32 overlay68CheckKind(s32 course);
extern s32 overlay84GetActive(void);
extern void overlay84ActivateCurrent(s32 state);
extern void overlay84ClearMode(void);
extern void overlay84Mark(void);

/* Overlay 57 text +0x4E18..+0x60F8. Matched 2026-10-01 (lane c-o057big) by
 * discarding inherited carriers rather than by allocator work; 86 -> 0 masked:
 *
 *  - the flag word is a bitfield (unsigned halfword test, 0xFFFE byte clear);
 *  - the +/-6 page steps read and update the selection global in place, and
 *    the page number compared against `state` is its own local;
 *  - the record pointer is `records[index].items` with the first call's
 *    result in a declared local, so the countdown fill's post-decrement
 *    temporary does not share a compiler temporary with it;
 *  - `panelX + 0xA0` and `panelX + 0x56` are written at each use. uopt spills
 *    the two commoned values itself, and the row and column loops are indexed
 *    so the strength-reduction temporaries interleave with those spill slots
 *    exactly as the target's frame has them (homes 0x64 and 0x5C);
 *  - the empty-row arm formats before it sets the icon index;
 *  - the fill is `count = 10; while (count--)`, the choice walk subscripts
 *    sourceState, and the tail reads the player count and the choice table
 *    directly instead of through carrier locals.
 *
 * Locals that are declared and never referenced are frame cells: every
 * declared scalar reserves a home in declaration order (L99), and the homed
 * arrays sit at the target's offsets only with this count between them.
 *
 * The seven o57MiddleO<NN>F<offset>Reloc externs are cross-overlay callees
 * reached through the module's relocation table (stored target zero). */
void func_overlay_057_F0004E18_18A8A10(s32 updateRate) {
    s32 i;
    s32 index;
    s32 limit;
    s16 input;
    s32 remainder;
    s32 previousGroup;
    s32 currentGroup;
    s32 page;
    s8 sourceState[4];
    u8 activePlayers[10];
    s32 nextSelection;
    s32 panelX;
    s32 characterId;
    s32 nextValue;
    s32 cursorValue;
    s32 outputIndex;
    s32 state;
    s32 valueA;
    s32 valueB;
    s32 valueC;
    s32 *list;
    s32 row;
    O57MiddleRenderItem *renderItems;
    O57MiddleRecord *records;
    O57MiddleTextureNode textureNodes[2];
    O57MiddleChoice *choice;
    char stackB0[2];
    s16 *color;
    char *palette;
    char renderState[24];
    u8 *active;
    s8 *source;
    O57MiddleOutput *output;
    s32 stack80;
    s32 stack7C;
    s32 stack78;
    s8 rank;

    if (overlay84GetActive() == 0) {
        if (gO57MiddleMode != 7) {
            gO57MiddleMode = 2;
        }

        if (D_800D3128.allUnlocked) {
            limit = 0x14;
            state = 3;
        } else if (gO57MiddleUnlocked19C != 0) {
            limit = 0x13;
            state = 3;
        } else if (gO57MiddleUnlocked198 != 0) {
            limit = 0xF;
            state = 2;
        } else {
            state = 1;
            limit = 0xB;
        }

        if (gO57MiddleState194 == 0) {
            page = gO57MiddleSelection / 6;
            if ((D_800D31BC < -16) && (gO57MiddleSelection > 0) && (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection -= 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 5) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x5A;
                    overlay57SetNodeValue(
                        0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                }
            } else if ((D_800D31BC >= 17) && (gO57MiddleSelection < limit) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection += 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 0) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    overlay57SetNodeValue(
                        0x2F, gO57MiddleSelection / 6, 0.012f);
                }
            } else if ((D_800D31BE < -16) && (gO57MiddleSelection < (limit - 2)) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                previousGroup = gO57MiddleSelection / 6;
                gO57MiddleSelection += 3;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                currentGroup = gO57MiddleSelection / 6;
                if (previousGroup != currentGroup) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    overlay57SetNodeValue(
                        0x2F, currentGroup, 0.012f);
                }
            } else if ((D_800D31BE >= 17) && (gO57MiddleSelection >= 3) &&
                    (gO57MiddleTransition == 0)) {
                    gO57MiddleMoving = 1;
                    overlay45ConfigureLayout(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                    gO57MiddlePreviousSelection = gO57MiddleSelection;
                    previousGroup = gO57MiddleSelection / 6;
                    gO57MiddleSelection -= 3;
                    overlay45ConfigureLayout(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                    gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                    gO57MiddleAlpha = 0xFF;
                    currentGroup = gO57MiddleSelection / 6;
                    if (previousGroup != currentGroup) {
                        gO57MiddleFade = 0;
                        gO57MiddleFadeDelay = 0x3C;
                        overlay57SetNodeValue(
                            0x2F, currentGroup + 5, 0.012f);
                    }
                } else if ((D_800D31B8 & 0x2020) && (gO57MiddleSelection >= 6) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection -= 6;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                overlay57SetNodeValue(
                    0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                } else if ((D_800D31B8 & 0x10) && (page < state) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection += 6;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                overlay57SetNodeValue(
                    0x2F, gO57MiddleSelection / 6, 0.012f);
                if (gO57MiddleSelection > limit) {
                    gO57MiddleSelection = limit;
                }
                }

            if ((D_800D31B8 & 0x4000) && (gO57MiddleTransition == 0) &&
                (gO57MiddleCanLeave != 0)) {
                func_80000F94(0xD, 0);
                overlay84ClearMode();
                func_80050704(gO57MiddlePathId);
                gO57MiddleNextMode = 7;
                overlay57SetNodeValue(
                    0x2F, 0, -0.012f);
                gO57MiddleState188 = 0;
                gO57MiddleMode = 0;
                overlay84ActivateCurrent(1);

                list = gO57MiddlePathList;
                while (*list != -1) {
                    func_80050688((u8)*list);
                    overlay57SetNodeValue(
                        *list, gO57MiddlePathIndices[*list], 0.007f);
                    list++;
                }
                list = gO57MiddleStopList;
                while (*list != -1) {
                    func_80050704((u8)*list);
                    list++;
                }
            }

            for (i = 0; i < updateRate; i++) {
                gO57MiddlePanelPosition += (-0x1400 - gO57MiddlePanelPosition) >> 2;
            }
        } else {
            if (gO57MiddleState194 == 1) {
                for (i = 0; i < updateRate; i++) {
                    gO57MiddlePanelPosition += (-gO57MiddlePanelPosition) >> 2;
                }
            }
            if ((D_800D31B8 & 0x4000) && (gO57MiddleTransition == 0)) {
                func_80000F94(0xD, 0);
                gO57MiddleState194 = 0;
                gO57MiddleMode = 2;
            }
        }

        func_overlay_057_F0001020_18A4C18(updateRate);
        if ((gO57MiddlePanelPosition >> 4) >= -0x135) {
            records = func_800291C4();
            renderItems = records[func_80025D60(D_8007C0E8[gO57MiddleSelection])].items;
            fontUseFont(0);
            row = 0x51;
            panelX = gO57MiddlePanelPosition >> 4;
            D_800D3550.position = (f32) panelX;
            D_800D3550.scale = 7.0f;
            func_80022A50(&D_800D3140, &D_800D3144);
            frontDrawObj(8);
            func_8004B0B8(
                gO57MiddleInfo.value08, gO57MiddleInfo.value09, gO57MiddleInfo.value0A,
                0xFF, 0xFF);
            fontPrintXY(
                &D_800D3140, panelX + 0xA0, 0x23,
                D_8007C0B8[D_8007C11C[gO57MiddleSelection]], 4);
            func_8004B0B8(0xFF, 0x80, 0, 0xFF, 0xFF);
            fontPrintXY(
                &D_800D3140, panelX + 0xA0, 0x37,
                D_8007C0B8[0xD0 / 4], 4);
            func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            if (overlay68CheckKind(
                    D_8007C0E8[gO57MiddleSelection]) != 0) {
                    func_8002F618(
                        &D_800D3140, gO57MiddleBadge, panelX + 0x30,
                        0x26, 0xFF, 0xFF, 0xFF, 0xFF);
            }

            for (i = 0; i < 4; i++) {
                overlay56SplitTime(
                    renderItems[i].type, &stack80, &stack7C, &stack78);
                if (renderItems[i].type == 0) {
                    func_800429A4(renderState, gO57MiddleEmptyFormat);
                    valueA = 0x4A;
                } else {
                    valueB = func_8003A700(
                                 renderItems[i].value04) & 0xFF;
                    valueC = func_8003A700(
                                 renderItems[i].value05) & 0xFF;
                    func_800429A4(
                        renderState, gO57MiddleResultFormat, valueB, valueC,
                        func_8003A700(
                            renderItems[i].value06),
                        stack80, stack7C, stack78);
                    valueA = renderItems[i].value07 + 0x51;
                }
                if (i < 3) {
                    fontPrintXY(
                        &D_800D3140, panelX + 0x2E, row,
                        gO57MiddleLabels[i], 0);
                }
                textureNodes[0].texture = D_800D31C8[valueA];
                textureNodes[0].alternate = NULL;
                textureNodes[0].x = panelX + 0x56;
                textureNodes[0].y = (s16)(row - 4);
                textureNodes[0].packedOffset = 0;
                textureNodes[1].texture = NULL;
                func_8002F618(
                    &D_800D3140, textureNodes, 0, 0,
                    0xFF, 0xFF, 0xFF, 0xFF);
                for (index = 0; index < 11; index++) {
                    stackB0[0] = renderState[index];
                    stackB0[1] = 0;
                    fontPrintXY(
                        &D_800D3140, gO57MiddleColumns[index] + panelX, row, stackB0, 0);
                }
                if (i == 2) {
                    row += 0x1B;
                    func_8004B0B8(
                        0xFF, 0x80, 0, 0xFF, 0xFF);
                    fontPrintXY(
                        &D_800D3140, panelX + 0xA0, row,
                        D_8007C0B8[0xD4 / 4], 4);
                    func_8004B0B8(
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                row += 0x1B;
            }
            func_8002FB34(
                &D_800D3140, gO57MiddleCaption, (f32)(panelX + 0x2D),
                184.0f, 1.0f, 1.0f, -2, 3);
        }

        if (gO57MiddleFadeDelay > 0) {
            gO57MiddleFadeDelay -= updateRate;
        } else {
            gO57MiddleFade += updateRate * 4;
            if (gO57MiddleFade >= 0x100) {
                gO57MiddleFade = 0xFF;
            }
        }

        if ((D_800D31B8 & 0x9000) && (gO57MiddleTransition == 0)) {
            func_80000F94(0xC, 0);
            if (D_8007C1A0 >= 2 || gO57MiddleState194 == 1) {
                outputIndex = 10;
                while (outputIndex--) {
                    activePlayers[outputIndex] = 1;
                }
                i = 0;
                for (index = 0; &gO57MiddleChoices[index] < &gO57MiddleChoices[4]; index++) {
                    rank = gO57MiddleChoices[index].active;
                    sourceState[index] = rank;
                    if (rank != 0) {
                        gO57MiddleOutput[i].controller =
                            gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex];
                        activePlayers[gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex]] = 0;
                        i++;
                    }
                }
                i = 0;
                for (outputIndex = D_8007C1A0; outputIndex < 6; outputIndex++) {
                    while (activePlayers[i] == 0) {
                        i++;
                    }
                    gO57MiddleOutput[outputIndex].controller = i;
                    i++;
                }
                func_80025444(
                    sourceState);
                func_80028D24(0);
                func_80028540(D_8007C1A0);
                D_8007BF44 = 0;
                if (D_8007C1A0 == 1) {
                    gO57MiddleOutput[0].value0 = 1;
                    if (D_800D3128.pending) {
                        D_800D3128.pending = 0;
                        func_800291B4();
                        func_8003A680(0x14);
                    }
                } else {
                    gO57MiddleOutput[0].value0 = 4;
                }
                if (((D_8007C1A0 == 2) || (D_8007C1A0 == 3)) &&
                    (gO57MiddleData3214 != 0)) {
                    D_8007BEFC = (s8)(4 - D_8007C1A0);
                    D_8007BEF8 = 4;
                } else {
                    D_8007BEFC = 0;
                    D_8007BEF8 = (u8)D_8007C1A0;
                }
                D_8007BF0C = (u8)(D_8007C1A0 >= 2);
                D_8007BF04 = 2;
                gO57MiddleData31B8 = gO57MiddleData31B4;
                D_8007BF08 = 0;
                if (D_8007BF44 > 0) {
                    gO57MiddleData31E8 = D_8007C0E8[gO57MiddleSelection];
                    gO57MiddleData31EC = gO57MiddleCharacterIds[gO57MiddleChoices[0].tableIndex];
                    gO57MiddleData31F0 = 5;
                    gO57MiddleData31F4 = 0;
                    func_80028374(0x12, 0, 0, 0xF, 1, 0);
                    func_80028528(1);
                } else {
                    func_80028374(
                        D_8007C0E8[gO57MiddleSelection],
                        gO57MiddleCharacterIds[gO57MiddleChoices[0].tableIndex],
                        0, 5, 1, 0);
                }
                gO57MiddleTransition = 1;
                overlay84Mark();
                if ((D_8007BF0C == 0) ||
                    ((D_8007BF0C != 0) && (gO57MiddleData3208 != 0))) {
                    func_80005548(D_8007BEF8);
                    if (gO57MiddleData3208 != 0) {
                        gO57MiddleData3208 = 0;
                    }
                }
            } else {
                gO57MiddleState194 = 1;
            }
        }
    }
}
