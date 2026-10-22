#include "overlays/overlay_050.h"

/* The module's runtime records address its .data and .bss through LOCAL
 * (section-relative) records, and as1 shares one high half across the stores
 * to adjacent fields of record 0 only for a TU-local symbol. So the module
 * defines its data at the offsets the records address (.data 0x370 and .bss
 * 0xD0). The bytes belong to the retained overlay image until phase 3: these
 * zero initializers only fix the layout, and the object's .data and .bss are
 * dropped at POSTPROCESS with their records rebound to zero-valued bases
 * (mk/overlays.mk). The D_ names keep their section offsets. */

static s32 sOverlay50Data000[27] = { 0 };

static O50Glyph D_6C[4] = { 0 };

static O50Glyph D_AC[3] = { 0 };

static O50Glyph D_DC[2] = { 0 };

static O50Glyph D_FC[3] = { 0 };

static O50Glyph D_12C[10] = { 0 };

static O50Glyph D_1CC[5] = { 0 };

static s16 D_21C = 0;

static O50Glyph sOverlay50LapTemplate[10] = { 0 };

static O50Glyph D_2C0[2] = { 0 };

static O50Glyph D_2E0[2] = { 0 };

static O50Glyph D_300[2] = { 0 };

static s8 o50Data320[8] = { 0 };

static s32 D_328 = 0;

static s8 D_32C[4] = { 0 };

static s8 o50Data330[4] = { 0 };

static s32 D_334 = 0;

static s32 sOverlay50Data338 = 0;

static s32 D_33C = 0;

static s32 sOverlay50Data340[12] = { 0 };

static O50Glyph o50LapGlyphs[10];

static s32 sOverlay50BssA0;

static f32 D_A4;

static f32 D_A8;

static s32 sOverlay50BssAC;

static s32 D_B0[3];

static s32 D_BC;

static s32 D_C0;

static void *o50BssC4;

static s16 D_C8;

static s16 D_CA;

/* Module-private helpers (no ORT row): static, so their calls are JUMP
 * records. The initializer passes the glyph tables unprototyped. */
static void overlay50PatchIndices();
static void overlay50SubmitTimeGlyphs(s32, s32, s32, s32);

/* Matched 2026-10-02 from a 104-word plateau. What it took:
 *   - the overlay's data defined in the TU (see above), so the copy loop
 *     indexes two real arrays and IDO's default unroller emits the shipped
 *     one-plus-two-by-four copy;
 *   - the descriptor stored to its global and read back from it, with no
 *     carrier local: the global's address is then one web hoisted above the
 *     branch, and as1 puts the NULL store in a branch-likely delay slot
 *     (no -Wab,-r4300_mul needed);
 *   - one resident callee (func_80028F54) called twice and levelGetNumber
 *     called twice, where the inherited candidate had four names for them;
 *   - 0.7f as a literal and an unused leading local for the saved-flag home.
 */
void func_overlay_050_F0000000_1896970(void) {
    s32 pad;
    u8 *saved;
    s32 i;

    saved = func_80028F54_o050Reloc();
    loadFrontEndList_o050Reloc(sOverlay50Data000);
    setupFrontEndList_o050Reloc(&sOverlay50Data000[18]);
    amTunePlay_o050Reloc(4);
    sOverlay50BssA0 = 0x104;
    runlinkDownloadCode_o050Reloc(0xB);

    overlay50PatchIndices(D_6C);
    overlay50PatchIndices(D_12C);
    overlay50PatchIndices(D_AC);
    overlay50PatchIndices(D_DC);
    overlay50PatchIndices(D_FC);
    overlay50PatchIndices(D_300);
    overlay50PatchIndices(D_2C0);
    overlay50PatchIndices(D_2E0);
    overlay50PatchIndices(D_1CC);

    D_A4 = -100.0f;
    D_A8 = 120.0f;
    D_800D3550_o050Reloc[4].x = 42.0f;
    D_800D3550_o050Reloc[1].x = -78.0f;
    D_800D3550_o050Reloc[1].value08 = 0.7f;
    D_800D3550_o050Reloc[0].angle = 0x4000;
    for (i = 0; i < 3; i++) {
        D_B0[i] = 0xA0;
    }

    overlay56LoadResource_o050Reloc();
    o50Data330[0] = -1;
    D_800D304E_o050Reloc = levelGetNumber_o050Reloc();
    D_BC = -0x500;
    D_C0 = -0x140;
    for (i = 0; i < 9; i++) {
        o50LapGlyphs[i].x = sOverlay50LapTemplate[i].x;
        o50LapGlyphs[i].y = sOverlay50LapTemplate[i].y;
        o50LapGlyphs[i].glyph = sOverlay50LapTemplate[i].glyph;
    }

    if (*func_80028F54_o050Reloc() == 1) {
        D_800D3550_o050Reloc[1].x += -40.0f;
        D_FC[0].x -= 0x28;
        D_FC[1].x -= 0x28;
    }

    if (*saved == 0) {
        fontUseFont_o050Reloc(3);
        o50BssC4 = overlay45CreateDescriptor_o050Reloc(
            func_8003A5A0_o050Reloc(levelGetNumber_o050Reloc()), 0xA0, 0x78, 0xC);
        overlay45SetMode_o050Reloc(o50BssC4, 0);
    } else {
        o50BssC4 = NULL;
    }
    D_CA = 0;
    D_C8 = 0;
}

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
static void overlay50PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) D_800D31C8[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) D_800D31C8[entry->second];
        }
        entry++;
    }
}

extern s32 o50Overlay1StateReloc; /* overlay 1 +0x83E0 */

/* Matched 2026-10-01 from a 1,115-word plateau. What it took:
 *   - -Wab,-r4300_mul on this TU (mk/overlays.mk) and plain for loops for
 *     every easing loop: IDO emits the shipped rotated loops from them;
 *   - the overlay's data defined in the TU (see above);
 *   - one counter for the speedometer angle loop and the lap loop, each
 *     `for (i = 0; ...)`: uopt hoists the lap loop's `i = 0` above the
 *     joypad test, which is the shipped spill at 0x114;
 *   - the speed readout in its own variable, the time magnitude only for
 *     the split, `racer->laps != level->laps`, a switch on the mode flag,
 *     and every local at function scope in the target's frame order. */
void func_overlay_050_F0000334_1896CA4(O50Object *object, s32 updateRate) {
    s32 i;
    s32 lapIndex;
    s32 messageX;
    s32 messageY;
    s32 bannerVisible;
    s32 itemId;
    s32 lapCount;
    O50Glyph messageGlyphs[5];
    O50Glyph itemGlyphs[2];
    O50Level *level;
    O50Racer *racer;
    u8 *modeFlag;
    s32 timeMagnitude;
    s32 targetAngle;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudY;
    s32 hudX;
    s32 speed;

    modeFlag = (u8 *)func_80028F54_o050Reloc();
    if (object != NULL) {
        racer = object->racer;
        camStandardOrtho_o050Reloc(&D_800D3140_o050Reloc, &D_800D3144_o050Reloc);
        if (o50BssC4 != 0) {
            D_C8 += updateRate;
            if (D_C8 >= 0x3D) {
                if (D_C8 >= 0xF1) {
                    D_CA -= updateRate * 4;
                    if (D_CA < 0) {
                        overlay45ReleaseDescriptor_o050Reloc(o50BssC4);
                        o50BssC4 = 0;
                    } else {
                        overlay45SetMode_o050Reloc(o50BssC4, D_CA);
                    }
                } else {
                    D_CA += updateRate * 4;
                    if (D_CA >= 0x100) {
                        D_CA = 0xFF;
                    }
                    overlay45SetMode_o050Reloc(o50BssC4, D_CA);
                }
            }
        }
        if (D_800C947C_o050Reloc == 0) {
            for (i = 0; i < updateRate; i++) {
                D_A4 += (0.0f - D_A4) * 0.125f;
                D_A8 += (0.0f - D_A8) * 0.125f;
            }
        }
        hudY = (s32) D_A4;
        hudX = (s32) D_A8;
        D_FC[0].glyph = (s32) (((s32) racer->value192 / 10) << 0x10);
        D_FC[1].glyph = (s32) (((s32) racer->value192 % 10) << 0x10);
        if (racer->value3BA != 0xFF) {
            D_AC[0].glyph = racer->value3BA << 0x10;
            D_AC[1].glyph = (s32) (o50Data320[racer->value3BA] << 0x10);
        } else {
            D_AC[0].glyph = (s32) (racer->value385 << 0x10);
            D_AC[1].glyph = (s32) (o50Data320[racer->value385] << 0x10);
        }
        overlay56SplitTime_o050Reloc(racer->raceTime, &minutes, &seconds, &centiseconds);
        level = levelGetLevel_o050Reloc();
        if ((D_800C947C_o050Reloc == 0) && (
racer->laps != level->laps
) && (func_800290A0_o050Reloc() == 0) && (func_8003A7D0_o050Reloc(object) != racer->raceTime)) {
            centiseconds -= centiseconds % 10;
            centiseconds += D_32C[0];
            D_32C[0]++;
            D_32C[0] = (s8) ((s8) D_32C[0] % 10);
        }
        D_12C[0].glyph = (s32) ((minutes / 10) << 0x10);
        D_12C[1].glyph = (s32) ((minutes % 10) << 0x10);
        D_12C[3].glyph = (s32) ((seconds / 10) << 0x10);
        D_12C[4].glyph = (s32) ((seconds % 10) << 0x10);
        D_12C[6].glyph = (s32) ((centiseconds / 10) << 0x10);
        D_12C[7].glyph = (s32) ((centiseconds % 10) << 0x10);
        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_12C, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_FC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        if (*modeFlag != 1) {
            if (racer->flags & 8) {
                func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_DC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            } else {
                func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_AC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
        texDPInit_o050Reloc(&D_800D3140_o050Reloc);
        D_800D3550_o050Reloc[4].y = (f32) (0x54 - hudY);
        D_800D3550_o050Reloc[4].angle = (s16) ((s32) (racer->raceTime * -0x10000) / 300);
        frontDrawObj_o050Reloc(4);
        D_800D3550_o050Reloc[1].y = (f32) (0x43 - hudY);
        frontDrawObj_o050Reloc(1);
        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_2C0, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        texDPInit_o050Reloc(&D_800D3140_o050Reloc);
        if (racer->item != 0xFF) {
            D_334 += updateRate * 16;
            if (D_334 >= 256) {
                D_334 = 255;
            }
        } else {
            D_334 -= updateRate * 8;
            if (D_334 < 0) {
                D_334 = 0;
            }
        }
        if (D_334 > 0) {
            if (racer->itemState != 0) {
                itemId = 0x35;
            } else if (racer->item != 0xFF) {
                itemId = D_8007C180_o050Reloc[racer->item];
            } else {
                itemId = o50Data330[0];
            }
            if (itemId != o50Data330[0]) {
                if (o50Data330[0] != -1) {
                    freeFrontEndItem_o050Reloc(o50Data330[0]);
                }
                o50Data330[0] = itemId;
                if (o50Data330[0] != -1) {
                    loadFrontEndItem_o050Reloc(o50Data330[0]);
                }
            }
            if (o50Data330[0] != -1) {
                if (o50Data330[0] == 0x35) {
                    itemGlyphs[0].x = 0x8A;
                    itemGlyphs[0].y = 0xF;
                } else {
                    itemGlyphs[0].x = 0x90;
                    itemGlyphs[0].y = 0x15;
                }
                itemGlyphs[0].glyph = 0;
                itemGlyphs[0].alternate = 0;
                itemGlyphs[1].texture = 0;
                itemGlyphs[0].texture = D_800D31C8_o050Reloc[o50Data330[0]];
                func_8002F618_o050Reloc(&D_800D3140_o050Reloc, itemGlyphs, 0, 0, 0xFF, 0xFF, 0xFF, D_334);
                if (o50Data330[0] != 0x35) {
                    if ((s32) racer->itemCount >= 2) {
                        D_300[0].glyph = (s32) (racer->itemCount << 0x10);
                        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_300, 0xAB, 0x2E, 0, 0, 0, D_334);
                        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_300, 0xAC, 0x30, 0, 0, 0, D_334);
                        func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_300, 0xAC, 0x2F, 0xFF, 0xFF, 0xFF, D_334);
                    }
                }
            }
        } else {
            if (o50Data330[0] != -1) {
                freeFrontEndItem_o050Reloc((s32) o50Data330[0]);
                o50Data330[0] = -1;
            }
        }
        if (racer->value388 != 0) {
            for (i = 0; i < updateRate; i++) {
                D_21C += (0x800 - D_21C) >> 3;
            }
            bannerVisible = 1;
        } else if (D_21C == -0x420) {
            bannerVisible = 0;
        } else {
            for (i = 0; i < updateRate; i++) {
                D_21C += (0x1820 - D_21C) >> 3;
            }
            if ((D_21C >> 6) == 0x60) {
                D_21C = -0x420;
                bannerVisible = 0;
            } else {
                bannerVisible = 1;
            }
        }
        if (bannerVisible != 0) {
            D_1CC[0].x = (s16) (D_21C >> 4);
            D_1CC[1].x = (s16) (D_21C >> 4);
            D_1CC[2].x = (s16) (D_21C >> 4);
            D_1CC[3].x = (s16) (D_21C >> 4);
            func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_1CC, 0, 0, 0xFF, 0xFF, 0xFF, 0xC0);
        }
        if (overlay59Interpolate_o050Reloc(0, -0x18, 0xBE, 0x30, 0xBE, &messageX, &messageY, 1) != 0) {
            overlay59BuildList_o050Reloc(0, messageGlyphs);
            if (messageGlyphs[0].texture != 0) {
                func_8002F618_o050Reloc(&D_800D3140_o050Reloc, messageGlyphs, messageX, messageY, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            overlay59DrawFrame_o050Reloc(&D_800D3140_o050Reloc, 0, messageX, messageY);
        }
        texAnimateSprite_o050Reloc(D_800D31C8_o050Reloc[0], &D_328, 0x14, &D_800D3550_o050Reloc[1].frame, updateRate);
        if (racer->laps < level->laps) {
            if (racer->differenceTimer >= updateRate) {
                if ((racer->differenceTimer == 0xB4) && (racer->timeDifference >= 0)) {
                    amSndPlay_o050Reloc(0x1F8, 0);
                }
                racer->differenceTimer -= updateRate;
                for (i = 0; i < updateRate; i++) {
                    D_BC += (0x830 - D_BC) >> 3;
                    D_C0 += (0x550 - D_C0) >> 3;
                }
            } else if (D_BC != -0x500) {
                if (racer->differenceTimer != -1) {
                    amSndPlay_o050Reloc(0x1F9, 0);
                    racer->differenceTimer = -1;
                }
                for (i = 0; i < updateRate; i++) {
                    D_BC += (0x1900 - D_BC) >> 3;
                    D_C0 += (-0x140 - D_C0) >> 3;
                }
                if (D_BC >= 0x1861) {
                    D_BC = -0x500;
                    D_C0 = -0x140;
                }
            }
            if (racer->timeDifference <= 0) {
                o50LapGlyphs[0].glyph = 12 << 16;
                timeMagnitude = -racer->timeDifference;
                for (i = 0; i < 9; i++) {
                    o50LapGlyphs[i].texture = D_800D31C8_o050Reloc[20];
                    o50LapGlyphs[i].alternate = D_800D31C8_o050Reloc[21];
                }
            } else {
                o50LapGlyphs[0].glyph = 13 << 16;
                timeMagnitude = racer->timeDifference;
                for (i = 0; i < 9; i++) {
                    o50LapGlyphs[i].texture = D_800D31C8_o050Reloc[80];
                    o50LapGlyphs[i].alternate = D_800D31C8_o050Reloc[21];
                }
            }
            overlay56SplitTime_o050Reloc(timeMagnitude, &minutes, &seconds, &centiseconds);
            o50LapGlyphs[1].glyph = (minutes / 10) << 0x10;
            o50LapGlyphs[2].glyph = (minutes % 10) << 0x10;
            o50LapGlyphs[4].glyph = (seconds / 10) << 0x10;
            o50LapGlyphs[5].glyph = (seconds % 10) << 0x10;
            o50LapGlyphs[7].glyph = (centiseconds / 10) << 0x10;
            o50LapGlyphs[8].glyph = (centiseconds % 10) << 0x10;
            for (i = 1; i < 9; i++) {
                if ((o50LapGlyphs[i].glyph >> 16) == 1) {
                    if (i == 1 || i == 4 || i == 7) {
                        o50LapGlyphs[i].x = sOverlay50LapTemplate[i].x + 1;
                    } else {
                        o50LapGlyphs[i].x = sOverlay50LapTemplate[i].x - 1;
                    }
                } else {
                    o50LapGlyphs[i].x = sOverlay50LapTemplate[i].x;
                }
            }
            func_8002F618_o050Reloc(&D_800D3140_o050Reloc, o50LapGlyphs, D_BC >> 4, D_C0 >> 4, 0xFF, 0xFF, 0xFF, 0xFF);
        }

        if (joyGetPressed_o050Reloc(0) & 1) {
            D_8007C1B0_o050Reloc ^= 1;
        }
        if (D_8007C1B0_o050Reloc != 0) {
            if (D_8007BF1C_o050Reloc & 1) {
                speed = (s32) (racer->speed * 6.25f);
                if (speed < 0) {
                    speed = -speed;
                }
                func_8004A4B0_o050Reloc(0xE6, 0xB4, speed, 3, 0);
                fontUseFont_o050Reloc(2);
                fontBackground_o050Reloc(0, 0, 0, 0);
                fontColour_o050Reloc(0x40, 0xFF, 0x40, 0xFF, 0xE0);
                fontPrintXY_o050Reloc(&D_800D3140_o050Reloc, 0x106, 0xBC, o50SpeedText, 0);
            } else {
                func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_6C, hudX, 0, 0xFF, 0xFF, 0xFF, 0xFF);
                if (racer->speed < 0.0f) {
                    targetAngle = (s32) (16384.0f - (-racer->speed * o50ReverseScale));
                } else {
                    targetAngle = (s32) (16384.0f - (racer->speed * o50ForwardScale));
                }
                for (i = 0; i < updateRate; i++) {
                    D_800D3550_o050Reloc[0].angle += (targetAngle - D_800D3550_o050Reloc[0].angle) >> 2;
                }

                D_800D3550_o050Reloc[0].x = hudX + 0x77;
                D_800D3550_o050Reloc[0].y = -85.0f;
                texDPInit_o050Reloc(&D_800D3140_o050Reloc);
                D_8007C0BC_o050Reloc = 0xFF;
                frontDrawObj_o050Reloc(0);
                D_8007C0BC_o050Reloc = 0xFF;
            }
        }
        lapCount = racer->laps;
        if (racer->finished != 0) {
            lapCount += 1;
        }
        for (i = 0; i < lapCount; i++) {
            for (lapIndex = 0; lapIndex < updateRate; lapIndex++) {
                D_B0[i] += -D_B0[i] >> 2;
            }
            overlay50SubmitTimeGlyphs(i + 1, D_B0[i] + 0xD6, i * 10 + 0x3C, racer->lapTimes[i]);
        }
        if (lapCount > 0) {
            func_8002F618_o050Reloc(&D_800D3140_o050Reloc, D_2E0, D_B0[0], 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (mainGetMode_o050Reloc() == 0) {
            switch (*modeFlag) {
            case 0:
                if ((o50Overlay1StateReloc == 0) && (joyGetPressed_o050Reloc(0) & 0x9000) && (D_33C == 0)) {
                    func_800016EC_o050Reloc(1);
                    func_8003A590_o050Reloc();
                    func_80037414_o050Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o050Reloc(0x12, 0, 0, 7, 1, 1);
                    amTuneSetFade_o050Reloc(3.0f, 0);
                    D_33C = 1;
                }
                break;
            case 1:
                if ((o50Overlay1StateReloc == 0) && (joyGetPressed_o050Reloc(0) & 0x9000) && (D_33C == 0)) {
                    func_800016EC_o050Reloc(1);
                    func_8003A590_o050Reloc();
                    func_80037414_o050Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o050Reloc(0x12, 0, 0, 7, 1, 0);
                    amTuneSetFade_o050Reloc(3.0f, 0);
                    D_33C = 1;
                }
                break;
            }
        }
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this resource teardown. */
void overlay50Cleanup(void) {
    if (gOverlay50ResourceA != NULL) {
        overlay50ReleaseResourceReloc(gOverlay50ResourceA);
    }
    freeFrontEndList(&gOverlay50Storage);
    overlay56ReleaseResource();
    if (gOverlay50ResourceB != NULL) {
        overlay50ReleaseResourceReloc(gOverlay50ResourceB);
    }
    if (gOverlay50Slot != -1) {
        freeFrontEndItem(gOverlay50Slot);
        gOverlay50Slot = -1;
    }
}

/* Exact DKR v77/v80 and JFG donor scans are negative for this formatter. */
static void overlay50SubmitTimeGlyphs(s32 leadingGlyph, s32 x, s32 y,
                               s32 timeValue) {
    s32 i;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    Overlay50GlyphRecord records[10];

    overlay56SplitTime(timeValue, &minutes, &seconds, &centiseconds);

    y -= 4;
    for (i = 0; i < 9; i++) {
        x += gOverlay50GlyphOffsets[i];
        records[i].x = x;
        records[i].y = y;
        records[i].sharedWord0 = gOverlay50GlyphWord0;
        records[i].sharedWord1 = gOverlay50GlyphWord1;
    }

    records[9].sharedWord0 = 0;
    records[9].sharedWord1 = 0;

    records[0].packedGlyph = leadingGlyph << 16;
    records[1].packedGlyph = (minutes / 10) << 16;
    records[2].packedGlyph = (minutes % 10) << 16;
    records[3].packedGlyph = 11 << 16;
    records[4].packedGlyph = (seconds / 10) << 16;
    records[5].packedGlyph = (seconds % 10) << 16;
    records[6].packedGlyph = 10 << 16;
    records[7].packedGlyph = (centiseconds / 10) << 16;
    records[8].packedGlyph = (centiseconds % 10) << 16;

    for (i = 1; i < 7; i++) {
        if ((records[i].packedGlyph >> 16) == 1) {
            if (!(i & 1)) {
                records[i].x++;
            } else {
                records[i].x--;
            }
        }
    }

    func_8002F618(D_800D3140, records, 0, 0,
                               255, 255, 255, 255);
}
