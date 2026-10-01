#include "PR/ultratypes.h"
#include "overlays/overlay_045.h"

typedef struct Overlay57MenuSource {
    u8 pad00[0x28];
    s16 index;
    s8 active;
    u8 pad2B[9];
} Overlay57MenuSource;

typedef struct Overlay57MenuEntry {
    u8 type;
    u8 pad01[3];
    s8 controller;
    u8 pad05[3];
    s32 value08;
    u8 pad0C[0x1C];
} Overlay57MenuEntry;

typedef struct Overlay57IndexLink {
    s32 index;
} Overlay57IndexLink;

/* Cross-overlay callees reached through the module's relocation table
 * (stored target zero): overlay 84 +0xC74, +0x1350, +0x1060, +0x1398 and
 * overlay 45 +0x314. The resident calls are renamed onto their generated
 * surface entries by the object's POSTPROCESS rule. */
extern s32 o57MenuO84F0000C74Reloc(void);
extern void o57MenuO84F0001350Reloc(void);
extern void o57MenuO84F0001060Reloc(s32 kind);
extern void o57MenuO84F0001398Reloc(void);
extern void o57MenuO45F0000314Reloc(Overlay45ResourceDescriptor *descriptor, s32 x, s32 y,
                                    s32 flags);
extern void amSndPlay(s32 soundId, void *handle);
extern void animseqStartPath(u8 pathId);
extern void animseqStopPath(u8 pathId);
extern void joyCreateMap(s8 *activePlayers);
extern void mainSetMode(s32 mode);
extern void mainChangeCameras(s32 camera);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle,
                            s32 cutscene, s32 multiplayer, s32 arg5);

extern void func_overlay_057_F0001020_18A4C18(s32 updateRate);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);

extern s32 gOverlay57State;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57Selection;
extern s32 gOverlay57PreviousSelection;
extern s32 gOverlay57LayoutValue;
extern s32 gOverlay57PreviousLayoutValue;
extern s32 gOverlay57LayoutBusy;
extern s32 gOverlay57Timer;
extern s32 gOverlay57ObjectReady;
extern u8 gOverlay57ObjectId;
extern Overlay45ResourceDescriptor *gOverlay57Layouts[];
extern Overlay57IndexLink gOverlay57StartLinks[];
extern Overlay57IndexLink gOverlay57StopLinks[];
extern s32 gOverlay57NodeArguments[];
extern s16 gOverlay57ControllerMap[];
extern Overlay57MenuEntry *gOverlay57MenuEntries;
extern s16 gOverlay57LevelBySelection[];
extern Overlay57MenuSource gOverlay57MenuSources[];
extern s32 gOverlay57PlayerCount;
extern s16 gOverlay57MenuInputX;
extern s16 gOverlay57MenuInputY;
extern u32 gOverlay57MenuButtons;
extern u8 gOverlay57MirrorSelection;
extern u8 gOverlay57MirrorFlag;
extern u8 gOverlay57MenuByte0;
extern u8 gOverlay57MenuByte1;
extern u8 gOverlay57MenuByte2;
extern u8 gOverlay57MenuByte3;
extern u16 gOverlay57MenuHalf;

/* Overlay 57 text +0x60F8..+0x67DC, the four-way menu step and start. Matched
 * 2026-10-01 (lane d-o057) from 252 masked words on overlay 57's middle-panel
 * template (func_overlay_057_F0004E18_18A8A10): the start and stop id walks
 * are `while (link->index != -1)` with the link read at each use, the player
 * setup is that function's countdown fill and choice walk reading the source
 * table and the controller map directly, the player count is one resident
 * word read at every use (it was split across three names), the entry value
 * reset is a loop IDO unrolls, and the entrance argument indexes the
 * controller map by the first source. */
void func_overlay_057_F00060F8_18A9CF0(s32 updateRate) {
    s32 count;
    s32 i;
    Overlay57IndexLink *link;
    s8 playerOrder[4];
    u8 activePlayers[10];
    s32 index;
    s8 rank;

    if (o57MenuO84F0000C74Reloc() != 0) {
        return;
    }

    gOverlay57State = 6;
    if (gOverlay57MenuInputX < -16 && gOverlay57Selection > 0 &&
        gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection--;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], -0xA0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputX >= 17 && gOverlay57Selection < 3 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection++;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0x1E0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputY < -16 && gOverlay57Selection < 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection += 2;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputY >= 17 && gOverlay57Selection >= 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection -= 2;
        o57MenuO45F0000314Reloc(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    }

    if ((gOverlay57MenuButtons & 0x4000) && gOverlay57DistanceState == 0 &&
        gOverlay57ObjectReady != 0) {
        amSndPlay(13, 0);
        o57MenuO84F0001350Reloc();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        o57MenuO84F0001060Reloc(1);

        link = gOverlay57StartLinks;
        while (link->index != -1) {
            animseqStartPath(link->index);
            overlay57SetNodeValue(link->index, gOverlay57NodeArguments[link->index], 0.007f);
            link++;
        }
        link = gOverlay57StopLinks;
        while (link->index != -1) {
            animseqStopPath(link->index);
            link++;
        }
    }

    func_overlay_057_F0001020_18A4C18(updateRate);
    if ((gOverlay57MenuButtons & 0x9000) && gOverlay57DistanceState == 0) {
        amSndPlay(12, 0);
        count = 10;
        while (count--) {
            activePlayers[count] = 1;
        }
        for (i = 0, index = 0; &gOverlay57MenuSources[index] < &gOverlay57MenuSources[4]; index++) {
            rank = gOverlay57MenuSources[index].active;
            playerOrder[index] = rank;
            if (rank != 0) {
                gOverlay57MenuEntries[i].controller =
                    gOverlay57ControllerMap[gOverlay57MenuSources[index].index];
                activePlayers[gOverlay57ControllerMap[gOverlay57MenuSources[index].index]] = 0;
                i++;
            }
        }
        i = 0;
        for (count = gOverlay57PlayerCount; count < 6; count++) {
            while (activePlayers[i] == 0) {
                i++;
            }
            gOverlay57MenuEntries[count].controller = i;
            i++;
        }
        joyCreateMap(playerOrder);
        mainSetMode(0);
        mainChangeCameras(gOverlay57PlayerCount);
        gOverlay57MenuEntries[0].type = 5;
        if (gOverlay57PlayerCount == 1) {
            gOverlay57MenuByte0 = 4;
            gOverlay57MenuByte1 = 3;
        } else if (gOverlay57MirrorFlag != 0) {
            gOverlay57MenuByte0 = 4;
            gOverlay57MenuByte1 = 4 - gOverlay57PlayerCount;
        } else {
            gOverlay57MenuByte0 = gOverlay57PlayerCount;
            gOverlay57MenuByte1 = 0;
        }
        gOverlay57MenuByte2 = gOverlay57PlayerCount >= 2;
        gOverlay57MenuByte3 = 2;
        for (i = 0; i < 6; i++) {
            gOverlay57MenuEntries[i].value08 = 0;
        }
        gOverlay57MenuHalf = 0x3FC;
        gOverlay57MirrorSelection = 0;
        mainChangeLevel(gOverlay57LevelBySelection[gOverlay57Selection],
                        gOverlay57ControllerMap[gOverlay57MenuSources[0].index],
                        0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        o57MenuO84F0001398Reloc();
    }
}
