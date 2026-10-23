#include "overlays/overlay_053.h"

/* Module-private helpers (no ORT row): static, so their calls are JUMP
 * records. The initializer hands them its raw tables unprototyped. */
static void overlay53PatchIndices();
static void overlay53CopyOffsetEntries();

/* Pinned DKR v77/v80 and JFG scans classify Overlay 53 as no donor. */
void overlay53Initialize(void) {
    Overlay53Config *configA;
    Overlay53Config *configB;
    s32 i;

    loadFrontEndList(gOverlay53Resource0);
    setupFrontEndList(gOverlay53Resource14);
    amTunePlay(4);
    gOverlay53Value280 = 0x104;
    runlinkDownloadCode(11);
    overlay53PatchIndices(gOverlay53PatchTableA);
    overlay53PatchIndices(gOverlay53PatchTableB);

    for (configA = gOverlay53ConfigA, configB = gOverlay53ConfigB, i = 0;
         i != 2;
         i++, configA++, configB++) {
        overlay53CopyOffsetEntries(gOverlay53PatchTableA, configA, i, 0);
        overlay53CopyOffsetEntries(gOverlay53PatchTableB, configB, i, 1);
    }

    gOverlay53Height290 = -80.0f;
    overlay56LoadResource();
    gOverlay53Handles288[0] = -1;
    gOverlay53Handles288[1] = -1;
    gOverlay53GlobalHandleReloc = levelGetNumber();
}

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
static void overlay53PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) D_800D31C8[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) D_800D31C8[entry->second];
        }
        entry++;
    }
}

/* Pinned DKR v77/v80 and JFG scans classify Overlay 53 as no donor. */
static void overlay53CopyOffsetEntries(Overlay53Entry *source,
                                Overlay53Entry *destination, s32 selection,
                                s32 index) {
    s16 xOffset;
    s16 yOffset;
    s16 *offsets;

    index *= 4;
    if (frontGetScreenMode() == 1) {
        index += 8;
    }
    if (selection == 0) {
        offsets = &gOverlay53Offsets[index];
        xOffset = offsets[0];
        yOffset = offsets[1];
    } else {
        offsets = &gOverlay53Offsets[index];
        xOffset = offsets[2];
        yOffset = offsets[3];
    }

    while (source->resource != 0) {
        destination->resource = source->resource;
        destination->alternate = source->alternate;
        destination->x = source->x + xOffset;
        destination->y = source->y + yOffset;
        destination->value8 = source->value8;
        source++;
        destination++;
    }
    destination->resource = 0;
}

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
            texDPInit(&D_800D3140);
            if (frontGetScreenMode() == 1) {
                D_800D3550[4].unkC = gOverlay53ClockSplitX[player];
                D_800D3550[4].unk10 = 80 - hudOffset;
            } else {
                D_800D3550[4].unkC = -44.0f;
                D_800D3550[4].unk10 = gOverlay53HudRowY[player] - hudOffset + 92;
            }
            D_800D3550[4].unk4 = racer->time * -65536 / 300;
            frontDrawObj(4);
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
            amTuneSetFade(3.0f, 0);
            gOverlay53TransitionDone = 1;
        }
    }
    camStandardOrtho(&D_800D3140, &D_800D3144);
    camSetNo(0);
}

void overlay53ReleaseResources(void) {
    s32 *handle;
    s32 *end;

    overlay53ReleaseReloc(gOverlay53ResourceA);
    overlay53ReleaseReloc();
    handle = &gOverlay53Handle0; end = &gOverlay53HandleEnd;
    do {
        if (*handle != -1) {
            overlay53ReleaseReloc(*handle);
            *handle = -1;
        }
        handle++;
    } while (handle != end);
}
