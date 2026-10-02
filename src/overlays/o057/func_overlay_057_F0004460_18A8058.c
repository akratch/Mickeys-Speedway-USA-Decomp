#include "PR/ultratypes.h"

typedef struct O57MenuTransition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} O57MenuTransition;

typedef struct Overlay57MenuSource {
    u8 pad00[0x28];
    s16 index;
    s8 active;
    u8 pad2B[9];
} Overlay57MenuSource;

typedef struct Overlay57MenuEntry {
    u8 type;
    u8 variant;
    u8 mode;
    u8 subtype;
    s8 controller;
    u8 pad05[0x23];
} Overlay57MenuEntry;

typedef struct O57MenuLink {
    s32 index;
} O57MenuLink;

typedef struct O57MenuFlags {
    u32 pad:14;
    u32 unlockPending:1;
    u32 rest:17;
    u8 pad04[0xF];
    u8 unlockMask;
} O57MenuFlags;

extern s32 gOverlay57ModeFlag;
extern O57MenuTransition gO57ModeSetup21C;
extern s16 gOverlay57MenuInputY;
extern u8 gOverlay57TableIndex;
extern s32 gO57MenuSelectionCountReloc;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57LayoutBusy;
extern u32 gOverlay57MenuButtons;
extern s32 gOverlay57PlayerCount;
extern Overlay57MenuEntry *gOverlay57MenuEntries;
extern u8 gOverlay57MenuByte0;
extern u8 gOverlay57MenuByte1;
extern u8 gOverlay57MenuByte2;
extern u8 gOverlay57MenuByte3;
extern u8 gOverlay57MirrorSelection;
extern u16 gOverlay57MenuHalf;
extern u16 gO57MiddleData31B4;
extern u8 gOverlay57MirrorFlag;
extern u8 gO57ChoicePublishedReloc;
extern Overlay57MenuSource gOverlay57MenuSources[];
extern s16 gOverlay57ControllerMap[];
extern s32 gO57MiddleData31E4;
extern s32 gO57MiddleData31E8;
extern s32 gO57MiddleData31EC;
extern s32 gO57MiddleData31F0;
extern s32 gO57MiddleData31F4;
extern O57MenuFlags gO57MiddleFlags;
extern s16 gO57UnlockTable[][4];
extern s16 gO57CourseTable[][4];
extern O57MenuLink gO57ModePrimaryIds[];
extern O57MenuLink gO57ModeSecondaryIds[];
extern s32 gO57ModePrimaryValues[];
extern u8 gOverlay57ObjectId;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;

extern void amSndPlay(s32 soundId, void *handle);
/* Cross-overlay callees reached through the module's relocation table
 * (stored target zero): overlay 84 +0x12FC, +0x1350, +0x1060 and +0x1398. */
extern s32 o57MenuO84F00012FCReloc(void);
extern void o57MenuO84F0001350Reloc(void);
extern void o57MenuO84F0001060Reloc(s32 kind);
extern void o57MenuO84F0001398Reloc(void);
extern void mainChangeCameras(s32 camera);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle,
                            s32 cutscene, s32 multiplayer, s32 arg5);
extern void mainSetAnimGroup(s32 group);
extern void mainSetMode(s32 mode);
extern void joyCreateMap(s8 *enabled);
extern void animseqStartPath(u8 pathId);
extern void animseqStopPath(u8 pathId);
extern void overlay57ApplyTable(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void o57PublishChoicesReloc(void);
extern void func_80005548(s32 value);
extern void func_800291B4(void);
extern void func_8003A680(s32 value);

/* Overlay 57 text +0x4460..+0x4C18, the one-player menu step and start.
 * Matched 2026-10-02 (lane o-ovl7) from 356 masked words on the matched
 * sibling func_overlay_057_F00060F8_18A9CF0's shape: the resident player
 * count is one word read at every use (it had been split across three
 * names, and the two course tables and the two unlock callees had been
 * crossed), the choice walk and fill read the source table and controller
 * map directly, and the declared locals give the 0x58 frame ladder. The
 * smoothing loop and both later loops share one index local, which coalesces the count web with its scaled induction
 * (the `or a3,a0` copy), and the unlock test is written bit-first. */
void func_overlay_057_F0004460_18A8058(s32 updateRate) {
    s8 activePlayers[10];
    O57MenuLink *link;
    s32 i;
    s32 current;
    s32 index;
    s8 playerOrder[4];

    gOverlay57ModeFlag = 0;
    for (index = 0; index < updateRate; index++) {
        gO57ModeSetup21C.horizontal += (0x104 - gO57ModeSetup21C.horizontal) >> 3;
        gO57ModeSetup21C.vertical += (0xBE - gO57ModeSetup21C.vertical) >> 3;
    }

    if (gOverlay57MenuInputY < -16 && gOverlay57DistanceState == 0) {
        if (gOverlay57TableIndex < gO57MenuSelectionCountReloc) {
            gOverlay57TableIndex++;
            gOverlay57LayoutBusy = 1;
        }
    } else if (gOverlay57MenuInputY >= 17 && gOverlay57DistanceState == 0) {
        if (gOverlay57TableIndex > 0) {
            gOverlay57TableIndex--;
            gOverlay57LayoutBusy = 1;
        }
    }

    overlay57ApplyTable();
    if ((gOverlay57MenuButtons & 0x9000) && gOverlay57DistanceState == 0) {
        amSndPlay(12, 0);
        current = o57MenuO84F00012FCReloc();
        mainChangeCameras(gOverlay57PlayerCount);
        if (gOverlay57PlayerCount == 1) {
            gOverlay57MenuEntries[0].type = 0;
            gOverlay57MenuByte0 = 6;
            gOverlay57MenuByte1 = 5;
            gOverlay57MenuByte2 = 0;
            if (gOverlay57TableIndex == 3) {
                gOverlay57MenuByte3 = 2;
                gOverlay57MirrorSelection = 1;
            } else {
                gOverlay57MenuByte3 = gOverlay57TableIndex;
                gOverlay57MirrorSelection = 0;
            }
            gOverlay57MenuHalf = 0x3FC;
        } else {
            gOverlay57MenuEntries[0].type = 3;
            if ((gOverlay57PlayerCount == 2 || gOverlay57PlayerCount == 3) &&
                gOverlay57MirrorFlag != 0) {
                gOverlay57MenuByte1 = 4 - gOverlay57PlayerCount;
                gOverlay57MenuByte0 = 4;
            } else {
                gOverlay57MenuByte1 = 0;
                gOverlay57MenuByte0 = gOverlay57PlayerCount;
            }
            gOverlay57MenuByte2 = 1;
            gOverlay57MenuByte3 = 2;
            gOverlay57MenuHalf = gO57MiddleData31B4;
            if (gOverlay57TableIndex == 3) {
                gOverlay57MirrorSelection = 1;
            } else {
                gOverlay57MirrorSelection = 0;
            }
        }

        for (i = 0; i < 10; i++) {
            activePlayers[i] = 1;
        }
        if (gOverlay57PlayerCount == 1) {
            o57PublishChoicesReloc();
        }
        for (i = 0, index = 0; &gOverlay57MenuSources[index] < &gOverlay57MenuSources[4]; index++) {
            playerOrder[index] = gOverlay57MenuSources[index].active;
            if (gOverlay57MenuSources[index].active != 0) {
                gOverlay57MenuEntries[i].controller =
                    gOverlay57ControllerMap[gOverlay57MenuSources[index].index];
                activePlayers[gOverlay57ControllerMap[gOverlay57MenuSources[index].index]] = 0;
                i++;
            }
        }
        i = 0;
        for (index = gOverlay57PlayerCount; index < 6; index++) {
            while (activePlayers[i] == 0) {
                i++;
            }
            gOverlay57MenuEntries[index].controller = i;
            i++;
        }
        joyCreateMap(playerOrder);
        gOverlay57MenuEntries[0].mode = current;
        gOverlay57MenuEntries[0].variant = 0;
        gOverlay57MenuEntries[0].subtype = 3;
        if (gOverlay57MenuByte2 == 0 || (gOverlay57MenuByte2 != 0 && gO57ChoicePublishedReloc != 0)) {
            func_80005548(gOverlay57MenuByte0);
            if (gO57ChoicePublishedReloc != 0) {
                gO57ChoicePublishedReloc = 0;
            }
        }

        gO57MiddleData31E4 = 0;
        if (gOverlay57PlayerCount == 1) {
            if (gO57MiddleFlags.unlockPending) {
                gO57MiddleFlags.unlockPending = 0;
                func_800291B4();
                func_8003A680(0);
            }
            i = gO57UnlockTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant];
            if (i != -1) {
                if (!((1 << i) & gO57MiddleFlags.unlockMask)) {
                    gO57MiddleFlags.unlockMask |= 1 << i;
                    func_800291B4();
                    func_8003A680(
                        gO57UnlockTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant] + 0xE);
                }
            }
        }

        if (gO57MiddleData31E4 > 0) {
            gO57MiddleData31E8 = gO57CourseTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant];
            gO57MiddleData31EC = gOverlay57ControllerMap[gOverlay57MenuSources[0].index];
            gO57MiddleData31F0 = 5;
            gO57MiddleData31F4 = 0;
            mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
            mainSetAnimGroup(1);
        } else {
            mainSetMode(0);
            mainChangeLevel(gO57CourseTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant],
                            gOverlay57ControllerMap[gOverlay57MenuSources[0].index], 0, 5, 1, 0);
        }
        gOverlay57DistanceState = 1;
        o57MenuO84F0001398Reloc();
    } else if ((gOverlay57MenuButtons & 0x4000) && gOverlay57DistanceState == 0) {
        amSndPlay(13, 0);
        o57MenuO84F0001350Reloc();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        o57MenuO84F0001060Reloc(3);

        link = gO57ModePrimaryIds;
        while (link->index != -1) {
            animseqStartPath(link->index);
            overlay57SetNodeValue(link->index, gO57ModePrimaryValues[link->index], 0.007f);
            link++;
        }
        link = gO57ModeSecondaryIds;
        while (link->index != -1) {
            animseqStopPath(link->index);
            link++;
        }
        gOverlay57State = 0;
    }
}
