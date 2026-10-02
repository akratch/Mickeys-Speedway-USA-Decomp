#include "PR/ultratypes.h"

typedef struct O57MenuTransition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} O57MenuTransition;

typedef struct O57MenuChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} O57MenuChoice;

typedef struct O57MenuOutput {
    u8 type;
    u8 variant;
    u8 mode;
    u8 subtype;
    s8 controller;
    u8 pad05[0x23];
} O57MenuOutput;

typedef struct O57MenuLink {
    s32 index;
} O57MenuLink;

typedef struct O57MenuState {
    u32 pad:14;
    u32 unlockPending:1;
    u32 rest:17;
    u8 pad04[0xF];
    u8 unlockMask;
} O57MenuState;

extern O57MenuState gO57MenuStateReloc;
extern s32 gOverlay57ModeFlag;
extern O57MenuTransition gO57ModeSetup21C;
extern s16 gO57MenuInputXReloc;
extern u8 gO57MenuSelectionReloc;
extern s32 gO57MenuSelectionCountReloc;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57LayoutBusy;
extern u32 gO57ModeInputFlags;
extern s32 gO57MenuKindReloc;
extern s32 gO57MenuCameraReloc;
extern s32 gO57MenuModeReloc;
extern s32 gO57MenuUnlockCountReloc;
extern s32 gO57MenuTransitionFlagReloc;
extern O57MenuChoice gO57ModeChoices[4];
extern O57MenuChoice gO57ModeChoicesEnd[];
extern s16 gO57ModeValueTable[];
extern s16 gO57MenuLevelTableReloc[][4];
extern s16 gO57MenuCourseTableReloc[][4];
extern s16 gO57MenuEntranceTableReloc[];
extern s16 gO57MenuEntranceIndexReloc;
extern s32 gO57MenuLevelReloc;
extern s32 gO57MenuEntranceReloc;
extern s32 gO57MenuVehicleReloc;
extern s32 gO57MenuCutsceneReloc;
extern s32 gO57MenuPrimaryValues[];
extern O57MenuLink gO57ModePrimaryIds[];
extern O57MenuLink gO57ModeSecondaryIds[];
extern O57MenuOutput *gO57ModeOutputs;
extern u8 gO57ModeFirstByte;
extern u8 gO57ModeSecondByte;
extern u8 gO57ModeThirdByte;
extern u8 gO57ModeFourthByte;
extern s16 gO57ModeShortValue;
extern u8 gO57ModeSixthByte;
extern u8 gO57MenuMirrorReloc;
extern u8 gO57MenuMirrorEnabledReloc;
extern u8 gOverlay57ObjectId;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;

extern void amSndPlay(s32 soundId, void *handle);
extern s32 overlay84GetEnabledCurrent(void);
extern void overlay84ClearMode(void);
extern void overlay84ActivateCurrent(s32 kind);
extern void overlay84Mark(void);
extern void mainChangeCameras(s32 camera);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle,
                            s32 cutscene, s32 multiplayer, s32 arg5);
extern void mainSetAnimGroup(s32 group);
extern void mainSetMode(s32 mode);
extern void joyCreateMap(s8 *enabled);
extern void animseqStartPath(s32 pathId);
extern void animseqStopPath(s32 pathId);
extern void overlay57ApplyTable(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, s32 valueBits);
extern void o57MenuPrepareReloc(void);
extern void o57MenuResetReloc(s32 value);
extern void o57MenuApplyCameraReloc(s32 camera);
extern void o57MenuClearUnlockReloc(void);
extern void o57MenuMarkUnlockReloc(void);
extern void o57MenuApplyUnlockReloc(s32 value);
extern void o57MenuCommitUnlockReloc(s32 value);

/* 2026-10-02 n-ovl6: rewritten in the shape of the matched sibling
 * func_overlay_057_F00060F8_18A9CF0 (countdown fill, pointer-compare choice
 * walk, count loops, link walks reading the link at each use) and with the
 * unlock block the target carries inside `kind == 1` (a bitfield flag word
 * tested by shift-and-sign, a mask byte OR, two further calls). 446 to 356
 * masked words, size delta -144 to -28 (7 words short), frame still 0x68
 * against 0x58 (the locals' homes sit 16 bytes higher). */
#ifdef NON_MATCHING
void func_overlay_057_F0004460_18A8058(s32 updateRate) {
    s8 enabled[4];
    u8 activePlayers[10];
    s32 count;
    s32 i;
    s32 x;
    s32 y;
    s32 index;
    s32 current;
    O57MenuLink *link;

    gOverlay57ModeFlag = 0;
    i = 0;
    if (updateRate > 0) {
        do {
            i++;
            gO57ModeSetup21C.horizontal = (s16)(gO57ModeSetup21C.horizontal +
                ((0x104 - gO57ModeSetup21C.horizontal) >> 3));
            gO57ModeSetup21C.vertical = (s16)(gO57ModeSetup21C.vertical +
                ((0xBE - gO57ModeSetup21C.vertical) >> 3));
        } while (i != updateRate);
    }

    if ((gO57MenuInputXReloc < -16) &&
        (gOverlay57DistanceState == 0)) {
        if (gO57MenuSelectionReloc < gO57MenuSelectionCountReloc) {
            gO57MenuSelectionReloc++;
            gOverlay57LayoutBusy = 1;
        }
    } else if ((gO57MenuInputXReloc >= 17) &&
               (gOverlay57DistanceState == 0)) {
        if (gO57MenuSelectionReloc > 0) {
            gO57MenuSelectionReloc--;
            gOverlay57LayoutBusy = 1;
        }
    }

    overlay57ApplyTable();
    if (((gO57ModeInputFlags & 0x9000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(12, 0);
        current = overlay84GetEnabledCurrent();
        mainChangeCameras(gO57MenuCameraReloc);
        if (gO57MenuKindReloc == 1) {
            gO57ModeOutputs[0].type = 0;
            gO57ModeFirstByte = 6;
            gO57ModeSecondByte = 5;
            gO57ModeThirdByte = 0;
            if (gO57MenuSelectionReloc == 3) {
                gO57ModeFourthByte = 2;
                gO57ModeSixthByte = 1;
            } else {
                gO57ModeFourthByte = gO57MenuSelectionReloc;
                gO57ModeSixthByte = 0;
            }
            gO57ModeShortValue = 0x3FC;
        } else {
            gO57ModeOutputs[0].type = 3;
            if (((gO57MenuSelectionReloc == 2) ||
                 (gO57MenuSelectionReloc == 3)) &&
                (gO57MenuMirrorEnabledReloc != 0)) {
                gO57ModeFirstByte = 4;
                gO57ModeSecondByte = 4 - gO57MenuSelectionReloc;
            } else {
                gO57ModeFirstByte = gO57MenuSelectionReloc;
                gO57ModeSecondByte = 0;
            }
            gO57ModeThirdByte = 1;
            gO57ModeFourthByte = 2;
            gO57ModeShortValue = gO57MenuEntranceIndexReloc;
            gO57ModeSixthByte = (gO57MenuSelectionReloc == 3);
        }

        count = 10;
        while (count--) {
            activePlayers[count] = 1;
        }
        if (gO57MenuKindReloc == 1) {
            o57MenuPrepareReloc();
        }

        x = 0;
        i = 0;
        do {
            enabled[i] = gO57ModeChoices[i].enabled;
            if (gO57ModeChoices[i].enabled != 0) {
                y = gO57ModeValueTable[gO57ModeChoices[i].tableIndex];
                gO57ModeOutputs[x++].controller = y;
                activePlayers[y] = 0;
            }
            i++;
        } while (&gO57ModeChoices[i] != gO57ModeChoicesEnd);

        y = 0;
        for (count = x; count < 6; count++) {
            while (activePlayers[y] == 0) {
                y++;
            }
            gO57ModeOutputs[count].controller = y;
            y++;
        }

        joyCreateMap(enabled);
        gO57ModeOutputs[0].mode = current;
        gO57ModeOutputs[0].variant = 0;
        gO57ModeOutputs[0].subtype = 3;
        if ((gO57MenuMirrorReloc == 0) ||
            (gO57MenuMirrorEnabledReloc != 0)) {
            o57MenuResetReloc(gO57MenuSelectionReloc);
            if (gO57MenuMirrorEnabledReloc != 0) {
                gO57MenuMirrorEnabledReloc = 0;
            }
        }

        gO57MenuTransitionFlagReloc = 0;
        if (gO57MenuKindReloc == 1) {
            if (gO57MenuStateReloc.unlockPending) {
                gO57MenuStateReloc.unlockPending = 0;
                o57MenuClearUnlockReloc();
                o57MenuCommitUnlockReloc(0);
            }
            y = gO57MenuLevelTableReloc[gO57ModeOutputs[0].mode]
                                       [gO57ModeOutputs[0].variant];
            if (y != -1) {
                if (!(gO57MenuStateReloc.unlockMask & (1 << y))) {
                    gO57MenuStateReloc.unlockMask |= 1 << y;
                    o57MenuMarkUnlockReloc();
                    o57MenuApplyUnlockReloc(
                        gO57MenuCourseTableReloc[gO57ModeOutputs[0].mode]
                                                [gO57ModeOutputs[0].variant] + 0xE);
                }
            }
        }

        if (gO57MenuUnlockCountReloc > 0) {
            gO57MenuLevelReloc =
                gO57MenuLevelTableReloc[gO57ModeOutputs[0].mode]
                                        [gO57ModeOutputs[0].variant];
            gO57MenuEntranceReloc =
                gO57ModeValueTable[gO57MenuEntranceIndexReloc];
            gO57MenuVehicleReloc = 5;
            gO57MenuCutsceneReloc = 0;
            mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
            mainSetAnimGroup(1);
        } else {
            mainSetMode(0);
            mainChangeLevel(
                gO57MenuCourseTableReloc[gO57ModeOutputs[0].mode]
                                         [gO57ModeOutputs[0].variant],
                gO57ModeValueTable[gO57MenuEntranceIndexReloc],
                0, 5, 1, 0);
        }
        gOverlay57DistanceState = 1;
        overlay84Mark();
        return;
    }

    if (((gO57ModeInputFlags & 0x4000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(3);

        link = gO57ModePrimaryIds;
        while (link->index != -1) {
            animseqStartPath(link->index & 0xFF);
            overlay57SetNodeValue(link->index, gO57MenuPrimaryValues[link->index],
                                  0x3BE56042);
            link++;
        }
        link = gO57ModeSecondaryIds;
        while (link->index != -1) {
            animseqStopPath(link->index & 0xFF);
            link++;
        }
        gOverlay57State = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0004460_18A8058/func_overlay_057_F0004460_18A8058.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_057_F0004460_18A8058:start
 * symbol: func_overlay_057_F0004460_18A8058
 * score: 356/447 words
 * frame: 0x68
 * relocations: 235
 * first-mismatch: +0x0
 * summary: Sibling-shape rewrite plus unlock bitfield block: 446 to 356 masked, size -144 to -28; frame 0x68 vs 0x58 open.
 * PLATEAU-HANDOFF:func_overlay_057_F0004460_18A8058:end
 */
