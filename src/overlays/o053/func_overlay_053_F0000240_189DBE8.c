#include "PR/ultratypes.h"
#include "game/menu.h"
#include "game/gameVi.h"
#include "overlays/overlay_056.h"

/* Same 0x10-byte entry layout used by overlay53CopyOffsetEntries. */
typedef struct Overlay53Entry {
    void *resource;
    void *alternate;
    u32 value8;
    s16 x;
    s16 y;
} Overlay53Entry;

typedef struct Overlay53Racer {
    u8 pad000[0x19A];
    u8 item;
    u8 pad19B;
    s32 value19C;
    u8 pad1A0[0x383 - 0x1A0];
    s8 lap;
    u8 pad384[0x400 - 0x384];
    s32 time;
} Overlay53Racer;

typedef struct Overlay53Object {
    u8 pad00[0x64];
    Overlay53Racer *racer;
} Overlay53Object;

typedef struct Overlay53Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay53Level;

typedef struct MenuCurrentObject {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 index;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    s8 pad1C[4];
} MenuCurrentObject;

extern MenuCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D31C8[];
extern MenuCurrentObject D_800D3550[];
extern s16 D_8007C180[];
extern s32 D_800C947C;
extern s32 ext_o1_83e0;

extern void camStandardOrtho(MenuCommand **, void **);
extern Overlay53Object **func_80005750(s32 *);
extern Overlay53Level *levelGetLevel(void);
extern void camSetNo(s32);
extern void camSetScissor(MenuCommand **);
extern void func_8002F618(MenuCommand **, Overlay53Entry *, s32, s32, u8, u8, u8, u8);
extern void func_80034920(MenuCommand **);
extern void func_80039E34(s32);
extern void overlay53CopyOffsetEntries(Overlay53Entry *, Overlay53Entry *, s32, s32);
extern s32 func_800290A0(void);
extern s32 mainGetMode(void);
extern u8 *func_80028F54(void);
extern void mainChangeCameras(s32);
extern void func_800016EC(u8);
extern void func_8003A590(void);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_800005CC(f32, u8);

extern Overlay53Entry gOverlay53TimeDigits[2][10];
extern Overlay53Entry gOverlay53ClockDigits[2][10];
extern Overlay53Entry gOverlay53TimeTemplate[];
extern s32 gOverlay53TransitionDone;
extern s8 gOverlay53BlinkCounter;
extern s32 gOverlay53IconAlpha[2];
extern s16 gOverlay53HudRowY[];
extern s16 gOverlay53IconSplitX[];
extern s16 gOverlay53ItemSplitX[];
extern s16 gOverlay53ClockSplitX[];
extern s32 gOverlay53Items[2];
extern f32 gOverlay53HudHeight;

/* Overlay 53's HUD update: the cut-down sibling of overlay52TailB, written the
 * same way. The callees and resident data are the ones this overlay's
 * relocation records name, with overlay52TailB's prototypes (the u8 alpha and
 * mode parameters decide which constants uopt keeps in saved registers).
 * Built with -Wab,-r4300_mul (mk/overlays.mk) for the rotated easing loop.
 * The loops reuse `player` and `i` as overlay52TailB does, the digit rows
 * and the clock row are indexed at each use, and the declarations are the
 * shipped frame (icon[2] fills the slots below hudOffset). Three statement
 * orders were measured decisive: the lap compare reads racer->lap first, the
 * screen-mode arm stores x before y, and the split icon arm stores x before
 * y (403 masked at frame 0xF8 to 0). */
void func_overlay_053_F0000240_189DBE8(s32 updateRate) {
    s32 i;
    s32 player;
    s32 desiredItems[2];
    Overlay53Level *level;
    Overlay53Object **racers;
    Overlay53Object *object;
    Overlay53Racer *racer;
    s32 racerCount;
    s32 minutes;
    s32 seconds;
    s32 hundredths;
    s32 hudOffset;
    Overlay53Entry icon[2];
    s32 width;
    u32 halfHeight;
    u8 *gameState;

    gameState = func_80028F54();
    viGetCurrentSize(&width, (s32 *)&halfHeight);
    halfHeight >>= 1;
    camStandardOrtho(&D_800D3140, &D_800D3144);
    racers = func_80005750(&racerCount);
    if (D_800C947C == 0) {
        for (i = 0; i < updateRate; i++) {
            gOverlay53HudHeight += (-11.0f - gOverlay53HudHeight) * 0.125f;
        }
    }
    hudOffset = (s32)gOverlay53HudHeight;
    level = levelGetLevel();
    gOverlay53BlinkCounter++;
    gOverlay53BlinkCounter %= 10;
    desiredItems[0] = -1;
    desiredItems[1] = -1;

    for (player = 0; player < 2; player++) {
        object = racers[player];
        if (object == NULL) {
            return;
        }
        racer = object->racer;
        if (racer->item != 255) {
            gOverlay53IconAlpha[player] += updateRate * 16;
            if (gOverlay53IconAlpha[player] >= 256) {
                gOverlay53IconAlpha[player] = 255;
            }
        } else {
            gOverlay53IconAlpha[player] -= updateRate * 8;
            if (gOverlay53IconAlpha[player] < 0) {
                gOverlay53IconAlpha[player] = 0;
            }
        }
        if (gOverlay53IconAlpha[player] > 0) {
            if (racer->value19C != 0) {
                desiredItems[player] = 53;
            } else if (racer->item != 255) {
                desiredItems[player] = D_8007C180[racer->item];
            } else {
                desiredItems[player] = gOverlay53Items[player];
            }
            if (desiredItems[player] != gOverlay53Items[player]) {
                if (gOverlay53Items[player] != -1) {
                    freeFrontEndItem(gOverlay53Items[player]);
                }
                gOverlay53Items[player] = desiredItems[player];
                if (gOverlay53Items[player] != -1) {
                    loadFrontEndItem(gOverlay53Items[player]);
                }
            }
        }
    }
    for (i = 0; i < 2; i++) {
        if (gOverlay53Items[i] != -1 && gOverlay53Items[i] != desiredItems[0] && gOverlay53Items[i] != desiredItems[1]) {
            freeFrontEndItem(gOverlay53Items[i]);
            gOverlay53Items[i] = -1;
        }
    }
    for (i = 0; i < 2; i++) {
        if (desiredItems[i] != -1) {
            gOverlay53Items[i] = desiredItems[i];
            if (D_800D31C8[gOverlay53Items[i]] == NULL) {
                loadFrontEndItem(gOverlay53Items[i]);
            }
        }
    }

    for (player = 0; player < 2; player++) {
        object = racers[player];
        if (object == NULL) {
            return;
        }
        racer = object->racer;
        camSetNo(player);
        camSetScissor(&D_800D3140);
        if (*gameState == 6) {
            overlay56SplitTime(racer->time, &minutes, &seconds, &hundredths);
            if (D_800C947C == 0 && racer->lap != level->laps &&
                func_800290A0() == 0 && racer->time != 0x83D60) {
                hundredths -= hundredths % 10;
                hundredths += gOverlay53BlinkCounter;
            }
            overlay53CopyOffsetEntries(gOverlay53TimeTemplate, gOverlay53TimeDigits[player], player, 0);
            gOverlay53TimeDigits[player][0].value8 = (minutes / 10) << 16;
            gOverlay53TimeDigits[player][1].value8 = (minutes % 10) << 16;
            gOverlay53TimeDigits[player][3].value8 = (seconds / 10) << 16;
            gOverlay53TimeDigits[player][4].value8 = (seconds % 10) << 16;
            gOverlay53TimeDigits[player][6].value8 = (hundredths / 10) << 16;
            gOverlay53TimeDigits[player][7].value8 = (hundredths % 10) << 16;
            for (i = 0; i < 8; i++) {
                if (((s32)gOverlay53TimeDigits[player][i].value8 >> 16) == 1) {
                    if (i == 0 || i == 3 || i == 6) {
                        gOverlay53TimeDigits[player][i].x++;
                    } else {
                        gOverlay53TimeDigits[player][i].x--;
                    }
                }
            }
            func_8002F618(&D_800D3140, gOverlay53TimeDigits[player], 0, hudOffset, 255, 255, 255, 255);
            func_80034920(&D_800D3140);
            if (frontGetScreenMode() == 1) {
                D_800D3550[4].unkC = gOverlay53ClockSplitX[player];
                D_800D3550[4].unk10 = 80 - hudOffset;
            } else {
                D_800D3550[4].unkC = -44.0f;
                D_800D3550[4].unk10 = gOverlay53HudRowY[player] - hudOffset + 92;
            }
            D_800D3550[4].unk4 = racer->time * -65536 / 300;
            func_80039E34(4);
            func_8002F618(&D_800D3140, gOverlay53ClockDigits[player], 0, hudOffset, 255, 255, 255, 255);
        }
        if (gOverlay53IconAlpha[player] > 0 && gOverlay53Items[player] != -1) {
            if (frontGet2PlayerSplit() != 0) {
                if (gOverlay53Items[player] == 53) {
                    icon[0].x = gOverlay53IconSplitX[player];
                    icon[0].y = 180;
                } else {
                    icon[0].x = gOverlay53ItemSplitX[player];
                    icon[0].y = 186;
                }
            } else if (gOverlay53Items[player] == 53) {
                icon[0].x = 25;
                icon[0].y = 62 - gOverlay53HudRowY[player];
            } else {
                icon[0].x = 31;
                icon[0].y = 68 - gOverlay53HudRowY[player];
            }
            icon[0].value8 = 0;
            icon[0].alternate = NULL;
            icon[1].resource = NULL;
            icon[0].resource = D_800D31C8[gOverlay53Items[player]];
            func_8002F618(&D_800D3140, icon, 0, 0, 255, 255, 255, gOverlay53IconAlpha[player]);
        }
        if (mainGetMode() == 0 && *func_80028F54() == 5 && ext_o1_83e0 == 0 && gOverlay53TransitionDone == 0) {
            mainChangeCameras(1);
            func_800016EC(1);
            func_8003A590();
            func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
            mainChangeLevel(18, 0, 0, 7, 1, 1);
            func_800005CC(3.0f, 0);
            gOverlay53TransitionDone = 1;
        }
    }
    camStandardOrtho(&D_800D3140, &D_800D3144);
    camSetNo(0);
}
