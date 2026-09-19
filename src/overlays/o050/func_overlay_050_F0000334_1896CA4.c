#include "PR/ultratypes.h"

typedef struct O50Glyph {
    void *texture;
    void *alternate;
    s32 glyph;
    s16 x;
    s16 y;
} O50Glyph;

/* Tier D: field widths and offsets from this caller's loads and stores. */
typedef struct O50Racer {
    u8 pad000[4];
    f32 speed;
    u8 pad008[0x18A];
    u8 value192;
    u8 pad193[7];
    u8 item;
    u8 itemCount;
    s32 itemState;
    u8 pad1A0[8];
    u16 flags;
    u8 pad1AA[0x1D9];
    s8 laps;
    u8 pad384;
    u8 value385;
    u8 pad386[2];
    u8 value388;
    u8 pad389[0x31];
    s16 value3BA;
    u8 pad3BC[0x44];
    s32 raceTime;
    s32 lapTimes[20];
    s16 timeDifference;
    s16 differenceTimer;
    u8 pad458[4];
    u8 finished;
} O50Racer;

typedef struct O50Object {
    u8 pad00[0x64];
    O50Racer *racer;
} O50Object;

typedef struct O50Level {
    u8 pad00[0x86];
    s8 laps;
} O50Level;

/* Tier B: callee identities decoded from overlay 50 runtime exports.
 * Signatures follow Mickey callees and the O32 argument surface. */
u8 *func_80028F54(void); /* 0:+0x28B04 */
void camStandardOrtho(void **, void **); /* 0:+0x22600 */
void overlay45ReleaseDescriptor(void *); /* 45:+0x270 */
void overlay45SetMode(void *, s32); /* 45:+0x1BE0 */
void overlay56SplitTime(s32, s32 *, s32 *, s32 *); /* 56:+0xB8 */
O50Level *levelGetLevel(void); /* 0:+0x2634C */
s32 func_800290A0(void); /* 0:+0x28C50 */
s32 func_8003A7D0(void *); /* 0:+0x3A380 */
void func_8002F618(void **, void *, s32, s32, s32, s32, s32, s32); /* 0:+0x2F1C8 */
void func_80034920(void **); /* 0:+0x344D0 */
void func_80039E34(s32); /* 0:+0x399E4 */
void freeFrontEndItem(s32); /* 0:+0x3964C */
void loadFrontEndItem(s32); /* 0:+0x39794 */
s32 overlay59Interpolate(s32, s32, s32, s32, s32, s32 *, s32 *, s32); /* 59:+0x784 */
void overlay59BuildList(s32, void *); /* 59:+0x84C */
void overlay59DrawFrame(void **, s32, s32, s32); /* 59:+0x8EC */
s32 func_80036544(void *, s32 *, s32, f32 *, s32); /* 0:+0x360F4 */
void amSndPlay(u16, void **); /* 0:+0xB44 */
u32 joyGetPressed(s32); /* 0:+0x250FC */
void func_8004A4B0(s32, s32, s32, s32, s32); /* 0:+0x4A060 */
void func_8004B0A4(s32); /* 0:+0x4AC54 */
void func_8004B0DC(s32, s32, s32, s32); /* 0:+0x4AC8C */
void fontColour(s32, s32, s32, s32, s32); /* 0:+0x4AC68 */
void func_8004B0F8(void **, s32, s32, char *, s32); /* 0:+0x4ACA8 */
s32 mainGetMode(void); /* 0:+0x288C8 */
void func_800016EC(s32); /* 0:+0x129C */
void func_8003A590(void); /* 0:+0x3A140 */
void func_80037414(s32, f32, f32, s32, s32, s32, s32); /* 0:+0x36FC4 */
void mainChangeLevel(s32, s32, s32, s32, s32, s32); /* 0:+0x27F24 */
void func_800005CC(f32, u8); /* 0:+0x17C */

void overlay50SubmitTimeGlyphs(s32, s32, s32, s32);
extern O50Glyph D_12C[10];
extern O50Glyph D_1CC[5];
extern s16 D_21C;
extern O50Glyph D_230[8];
/* Tier B: distinct relocation anchors within the eight-record template. */
extern O50Glyph D_260[];
extern O50Glyph D_290[];
extern O50Glyph D_2B0[];
extern O50Glyph D_2C0[2];
extern O50Glyph D_2E0[2];
extern O50Glyph D_300[2];
extern s32 D_328;
extern s8 D_32C;
extern s32 D_334;
extern s32 D_33C;

extern O50Glyph D_6C[4];
extern f32 D_A4;
extern f32 D_A8;
extern O50Glyph D_AC[3];
extern s32 D_B0[];
extern s32 D_BC;
extern s32 D_C0;
extern s16 D_C8;
extern s16 D_CA;
extern O50Glyph D_DC[2];
extern O50Glyph D_FC[3];

typedef struct O50MenuObject {
    s16 value00;
    s16 value02;
    s16 angle;
    s16 index;
    f32 value08;
    f32 x;
    f32 y;
    f32 value14;
    f32 frame;
    u8 pad1C[4];
} O50MenuObject;

/* Tier B: runtime export identities, kept distinct from stored addends. */
extern void *D_800D3140;
extern void *D_800D3144;
extern s32 D_800C947C;
extern s16 D_8007C180[];
extern void *D_800D31C8[];
extern O50MenuObject D_800D3550[];
extern s32 D_8007C1B0;
extern u16 D_8007BF1C;
extern s32 D_8007C0BC;
extern s32 o50Overlay1StateReloc; /* overlay 1 +0x83E0 */

/* Tier B: local data, read-only data, and BSS ownership from runtime records. */
extern s8 o50Data320[];
extern s8 o50Data330;
extern void *o50BssC4;
extern O50Glyph o50LapGlyphs[10];
/* Tier B: BSS anchors for sign, digit range, and list terminator. */
extern O50Glyph D_0;
extern O50Glyph D_10[8];
extern O50Glyph D_90;
extern char o50SpeedText[];
extern f32 o50ReverseScale;
extern f32 o50ForwardScale;

/* NON_MATCHING: runtime call/global identities and record layouts are recovered.
 * The configured candidate still has a structural and allocation residual;
 * the assembly fallback remains the only ROM-exact implementation. */
#ifdef NON_MATCHING
void func_overlay_050_F0000334_1896CA4(O50Object *object, s32 updateRate) {
    s32 messageX;
    s32 messageY;
    O50Glyph messageGlyphs[5];
    O50Glyph itemGlyphs[2];
    O50Level *level;
    O50Racer *racer;
    u8 *modeFlag;
    s32 timeMagnitude;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudY;
    s32 hudX;
    O50Glyph *glyphCursor;
    O50Glyph *templateCursor;
    O50Glyph *glyphEnd;
    s32 itemId;
    void *glyphTexture;
    s32 remainder;
    void *glyphAlternate;
    s32 i;
    s32 lapIndex;
    s32 bannerVisible;
    s32 targetAngle;
    s32 lapCount;
    s32 *lapOffsets;
    s32 *lapTimes;
    s32 lapY;

    modeFlag = (u8 *)func_80028F54();
    if (object != NULL) {
        racer = object->racer;
        camStandardOrtho(&D_800D3140, &D_800D3144);
        if (o50BssC4 != 0) {
            D_C8 += updateRate;
            if (D_C8 >= 0x3D) {
                if (D_C8 >= 0xF1) {
                    D_CA -= updateRate * 4;
                    if (D_CA < 0) {
                        overlay45ReleaseDescriptor(o50BssC4);
                        o50BssC4 = 0;
                    } else {
                        overlay45SetMode(o50BssC4, D_CA);
                    }
                } else {
                    D_CA += updateRate * 4;
                    if (D_CA >= 0x100) {
                        D_CA = 0xFF;
                    }
                    overlay45SetMode(o50BssC4, D_CA);
                }
            }
        }
        if (D_800C947C == 0) {
            i = 0;
            if (updateRate > 0) {
                remainder = updateRate & 3;
                if (*(s32 *)&remainder != 0) {
                    do {
                        i++;
                        D_A4 += (0.0f - D_A4) * 0.125f;
                        D_A8 += (0.0f - D_A8) * 0.125f;
                    } while (i != remainder);
                    if (i != updateRate) {
                        goto hudQuad;
                    }
                } else {
hudQuad:
                    do {
                        i += 4;
                        D_A4 += (0.0f - D_A4) * 0.125f;
                        D_A8 += (0.0f - D_A8) * 0.125f;
                        D_A4 += (0.0f - D_A4) * 0.125f;
                        D_A8 += (0.0f - D_A8) * 0.125f;
                        D_A4 += (0.0f - D_A4) * 0.125f;
                        D_A8 += (0.0f - D_A8) * 0.125f;
                        D_A4 += (0.0f - D_A4) * 0.125f;
                        D_A8 += (0.0f - D_A8) * 0.125f;
                    } while (i != updateRate);
                }
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
        overlay56SplitTime(racer->raceTime, &minutes, &seconds, &centiseconds);
        level = levelGetLevel();
        if ((D_800C947C == 0) && (level->laps != racer->laps) && (func_800290A0() == 0) && (func_8003A7D0(object) != racer->raceTime)) {
            centiseconds -= centiseconds % 10;
            centiseconds += D_32C;
            D_32C++;
            D_32C = (s8) ((s8) D_32C % 10);
        }
        D_12C[0].glyph = (s32) ((minutes / 10) << 0x10);
        D_12C[1].glyph = (s32) ((minutes % 10) << 0x10);
        D_12C[3].glyph = (s32) ((seconds / 10) << 0x10);
        D_12C[4].glyph = (s32) ((seconds % 10) << 0x10);
        D_12C[6].glyph = (s32) ((centiseconds / 10) << 0x10);
        D_12C[7].glyph = (s32) ((centiseconds % 10) << 0x10);
        func_8002F618(&D_800D3140, D_12C, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        func_8002F618(&D_800D3140, D_FC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        if (*modeFlag != 1) {
            if (racer->flags & 8) {
                func_8002F618(&D_800D3140, D_DC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            } else {
                func_8002F618(&D_800D3140, D_AC, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
        func_80034920(&D_800D3140);
        D_800D3550[4].y = (f32) (0x54 - hudY);
        D_800D3550[4].angle = (s16) ((s32) (racer->raceTime * -0x10000) / 300);
        func_80039E34(4);
        D_800D3550[1].y = (f32) (0x43 - hudY);
        func_80039E34(1);
        func_8002F618(&D_800D3140, D_2C0, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
        func_80034920(&D_800D3140);
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
                itemId = D_8007C180[racer->item];
            } else {
                itemId = o50Data330;
            }
            if (itemId != o50Data330) {
                if (o50Data330 != -1) {
                    freeFrontEndItem(o50Data330);
                }
                o50Data330 = itemId;
                if (o50Data330 != -1) {
                    loadFrontEndItem(o50Data330);
                }
            }
            if (o50Data330 != -1) {
                if (o50Data330 == 0x35) {
                    itemGlyphs[0].x = 0x8A;
                    itemGlyphs[0].y = 0xF;
                } else {
                    itemGlyphs[0].x = 0x90;
                    itemGlyphs[0].y = 0x15;
                }
                itemGlyphs[0].glyph = 0;
                itemGlyphs[0].alternate = 0;
                itemGlyphs[1].texture = 0;
                itemGlyphs[0].texture = D_800D31C8[o50Data330];
                func_8002F618(&D_800D3140, itemGlyphs, 0, 0, 0xFF, 0xFF, 0xFF, D_334);
                if (o50Data330 != 0x35) {
                    if ((s32) racer->itemCount >= 2) {
                        D_300[0].glyph = (s32) (racer->itemCount << 0x10);
                        func_8002F618(&D_800D3140, D_300, 0xAB, 0x2E, 0, 0, 0, D_334);
                        func_8002F618(&D_800D3140, D_300, 0xAC, 0x30, 0, 0, 0, D_334);
                        func_8002F618(&D_800D3140, D_300, 0xAC, 0x2F, 0xFF, 0xFF, 0xFF, D_334);
                    }
                }
            }
        } else {
            if (o50Data330 != -1) {
                freeFrontEndItem((s32) o50Data330);
                o50Data330 = -1;
            }
        }
        if (racer->value388 != 0) {
            i = 0;
            if (updateRate > 0) {
                remainder = updateRate & 3;
                if (remainder != 0) {
                    do {
                        i += 1;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                    } while (remainder != i);
                }
                if (i != updateRate) {
                    do {
                        i += 4;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                    } while (i != updateRate);
                }
            }
            bannerVisible = 1;
        } else if (D_21C == -0x420) {
            bannerVisible = 0;
        } else {
            i = 0;
            if (updateRate > 0) {
                remainder = updateRate & 3;
                if (remainder != 0) {
                    do {
                        i += 1;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                    } while (remainder != i);
                }
                if (i != updateRate) {
                    do {
                        i += 4;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                    } while (i != updateRate);
                }
            }
            bannerVisible = 1;
            if (((s16) D_21C >> 6) == 0x60) {
                D_21C = -0x420;
                bannerVisible = 0;
            }
        }
        if (bannerVisible != 0) {
            D_1CC[0].x = (s16) (D_21C >> 4);
            D_1CC[1].x = (s16) (D_21C >> 4);
            D_1CC[2].x = (s16) (D_21C >> 4);
            D_1CC[3].x = (s16) (D_21C >> 4);
            func_8002F618(&D_800D3140, D_1CC, 0, 0, 0xFF, 0xFF, 0xFF, 0xC0);
        }
        if (overlay59Interpolate(0, -0x18, 0xBE, 0x30, 0xBE, &messageX, &messageY, 1) != 0) {
            overlay59BuildList(0, messageGlyphs);
            if (messageGlyphs[0].texture != 0) {
                func_8002F618(&D_800D3140, messageGlyphs, messageX, messageY, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            overlay59DrawFrame(&D_800D3140, 0, messageX, messageY);
        }
        func_80036544(D_800D31C8[0], &D_328, 0x14, &D_800D3550[1].frame, updateRate);
        if (racer->laps < level->laps) {
            if (racer->differenceTimer >= updateRate) {
                i = 0;
                if ((racer->differenceTimer == 0xB4) && (racer->timeDifference >= 0)) {
                    amSndPlay(0x1F8, 0);
                }
                racer->differenceTimer -= updateRate;
                if (updateRate > 0) {
                    if ((updateRate & 3) != 0) {
                        do {
                            i += 1;
                            D_BC += (s32) (0x830 - D_BC) >> 3;
                            D_C0 += (s32) (0x550 - D_C0) >> 3;
                        } while ((updateRate & 3) != i);
                    }
                    if (i != updateRate) {
                        do {
                            i += 4;
                            D_BC += (0x830 - D_BC) >> 3;
                            D_C0 += (0x550 - D_C0) >> 3;
                            D_BC += (0x830 - D_BC) >> 3;
                            D_C0 += (0x550 - D_C0) >> 3;
                            D_BC += (0x830 - D_BC) >> 3;
                            D_C0 += (0x550 - D_C0) >> 3;
                            D_BC += (0x830 - D_BC) >> 3;
                            D_C0 += (0x550 - D_C0) >> 3;
                        } while (i != updateRate);
                    }
                }
            } else if (D_BC != -0x500) {
                i = 0;
                if (racer->differenceTimer != -1) {
                    amSndPlay(0x1F9, 0);
                    racer->differenceTimer = -1;
                }
                if (updateRate > 0) {
                    if ((updateRate & 3) != 0) {
                        do {
                            i += 1;
                            D_BC += (s32) (0x1900 - D_BC) >> 3;
                            D_C0 += (s32) (-0x140 - D_C0) >> 3;
                        } while ((updateRate & 3) != i);
                    }
                    if (i != updateRate) {
                        do {
                            i += 4;
                            D_BC += (0x1900 - D_BC) >> 3;
                            D_C0 += (-0x140 - D_C0) >> 3;
                            D_BC += (0x1900 - D_BC) >> 3;
                            D_C0 += (-0x140 - D_C0) >> 3;
                            D_BC += (0x1900 - D_BC) >> 3;
                            D_C0 += (-0x140 - D_C0) >> 3;
                            D_BC += (0x1900 - D_BC) >> 3;
                            D_C0 += (-0x140 - D_C0) >> 3;
                        } while (i != updateRate);
                    }
                }
                if (D_BC >= 0x1861) {
                    D_BC = -0x500;
                    D_C0 = -0x140;
                }
            }
            glyphEnd = &D_90;
            if (racer->timeDifference <= 0) {
                D_0.glyph = 12 << 16;
                glyphAlternate = D_800D31C8[21];
                glyphTexture = D_800D31C8[20];
                timeMagnitude = (s32) -racer->timeDifference;
                glyphCursor = D_10;
                D_0.alternate = glyphAlternate;
                D_0.texture = glyphTexture;
                do {
                    glyphCursor += 4;
                    glyphCursor[-3].texture = glyphTexture;
                    glyphCursor[-3].alternate = glyphAlternate;
                    glyphCursor[-2].texture = glyphTexture;
                    glyphCursor[-2].alternate = glyphAlternate;
                    glyphCursor[-1].texture = glyphTexture;
                    glyphCursor[-1].alternate = glyphAlternate;
                    glyphCursor[-4].texture = glyphTexture;
                    glyphCursor[-4].alternate = glyphAlternate;
                } while (glyphCursor != glyphEnd);
            } else {
                D_0.glyph = 13 << 16;
                glyphAlternate = D_800D31C8[21];
                glyphTexture = D_800D31C8[80];
                glyphCursor = D_10;
                D_0.alternate = glyphAlternate;
                D_0.texture = glyphTexture;
                timeMagnitude = (s32) racer->timeDifference;
                do {
                    glyphCursor += 4;
                    glyphCursor[-3].texture = glyphTexture;
                    glyphCursor[-3].alternate = glyphAlternate;
                    glyphCursor[-2].texture = glyphTexture;
                    glyphCursor[-2].alternate = glyphAlternate;
                    glyphCursor[-1].texture = glyphTexture;
                    glyphCursor[-1].alternate = glyphAlternate;
                    glyphCursor[-4].texture = glyphTexture;
                    glyphCursor[-4].alternate = glyphAlternate;
                } while (glyphCursor != glyphEnd);
            }
            overlay56SplitTime(timeMagnitude, &minutes, &seconds, &centiseconds);
            o50LapGlyphs[1].glyph = (minutes / 10) << 0x10;
            o50LapGlyphs[2].glyph = (minutes % 10) << 0x10;
            o50LapGlyphs[4].glyph = (seconds / 10) << 0x10;
            o50LapGlyphs[5].glyph = (seconds % 10) << 0x10;
            o50LapGlyphs[7].glyph = (centiseconds / 10) << 0x10;
            o50LapGlyphs[8].glyph = (centiseconds % 10) << 0x10;
            templateCursor = D_230;
            glyphCursor = D_10;
            do {
                if (((s32) glyphCursor[0].glyph >> 0x10) == 1) {
                    if ((templateCursor == D_230) || (templateCursor == D_260) || (templateCursor == D_290)) {
                        glyphCursor[0].x = (s16) (templateCursor[0].x + 1);
                    } else {
                        glyphCursor[0].x = (s16) (templateCursor[0].x - 1);
                    }
                } else {
                    glyphCursor[0].x = (s16) templateCursor[0].x;
                }
                templateCursor++;
                glyphCursor++;
            } while (templateCursor != D_2B0);
            func_8002F618(&D_800D3140, o50LapGlyphs, D_BC >> 4, D_C0 >> 4, 0xFF, 0xFF, 0xFF, 0xFF);
        }

        if (joyGetPressed(0) & 1) {
            D_8007C1B0 ^= 1;
        }
        lapIndex = 0;
        if (D_8007C1B0 != 0) {
            if (D_8007BF1C & 1) {
                timeMagnitude = (s32) (racer->speed * 6.25f);
                if (timeMagnitude < 0) {
                    timeMagnitude = -timeMagnitude;
                }
                func_8004A4B0(0xE6, 0xB4, timeMagnitude, 3, 0);
                func_8004B0A4(2);
                func_8004B0DC(0, 0, 0, 0);
                fontColour(0x40, 0xFF, 0x40, 0xFF, 0xE0);
                func_8004B0F8(&D_800D3140, 0x106, 0xBC, o50SpeedText, 0);
            } else {
                func_8002F618(&D_800D3140, D_6C, hudX, 0, 0xFF, 0xFF, 0xFF, 0xFF);
                if (racer->speed < 0.0f) {
                    targetAngle = (s32) (16384.0f - (-racer->speed * o50ReverseScale));
                } else {
                    targetAngle = (s32) (16384.0f - (racer->speed * o50ForwardScale));
                }
                if (updateRate > 0) {
                    if ((updateRate & 3) != 0) {
                        do {
                            lapIndex += 1;
                            D_800D3550[0].angle = (s16) (D_800D3550[0].angle + ((s32) (targetAngle - D_800D3550[0].angle) >> 2));
                        } while ((updateRate & 3) != lapIndex);
                    }
                    if (lapIndex != updateRate) {
                        do {
                            lapIndex += 4;
                            D_800D3550[0].angle = (s16) (D_800D3550[0].angle + ((s32) (targetAngle - D_800D3550[0].angle) >> 2));
                            D_800D3550[0].angle = (s16) (D_800D3550[0].angle + ((s32) (targetAngle - D_800D3550[0].angle) >> 2));
                            D_800D3550[0].angle = (s16) (D_800D3550[0].angle + ((s32) (targetAngle - D_800D3550[0].angle) >> 2));
                            D_800D3550[0].angle = (s16) (D_800D3550[0].angle + ((s32) (targetAngle - D_800D3550[0].angle) >> 2));
                        } while (lapIndex != updateRate);
                    }
                    lapIndex = 0;
                }

                D_800D3550[0].y = -85.0f;
                D_800D3550[0].x = (f32) (hudX + 0x77);
                func_80034920(&D_800D3140);
                D_8007C0BC = 0xFF;
                func_80039E34(0);
                D_8007C0BC = 0xFF;
            }
        }
        lapOffsets = D_B0;
        lapTimes = racer->lapTimes;
        lapY = 0x3C;
        lapCount = racer->laps;
        if (racer->finished != 0) {
            lapCount += 1;
        }
        if (lapCount > 0) {
            do {
                i = 0;
                if (updateRate > 0) {
                    if ((updateRate & 3) != 0) {
                        do {
                            i += 1;
                            *lapOffsets += (-*lapOffsets) >> 2;
                        } while ((updateRate & 3) != i);
                    }
                    if (i != updateRate) {
                        do {
                            i += 4;
                            *lapOffsets += (-*lapOffsets) >> 2;
                            *lapOffsets += (-*lapOffsets) >> 2;
                            *lapOffsets += (-*lapOffsets) >> 2;
                            *lapOffsets += (-*lapOffsets) >> 2;
                        } while (i != updateRate);
                    }
                }

                overlay50SubmitTimeGlyphs(lapIndex + 1, *lapOffsets + 0xD6, lapY, *lapTimes);
                lapIndex++;
                lapOffsets++;
                lapTimes++;
                lapY += 10;
            } while (lapIndex != lapCount);
        }
        if (lapCount > 0) {
            func_8002F618(&D_800D3140, D_2E0, D_B0[0], 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (mainGetMode() == 0) {
            if (*modeFlag == 0) {
                if ((o50Overlay1StateReloc == 0) && (joyGetPressed(0) & 0x9000) && (D_33C == 0)) {
                    func_800016EC(1);
                    func_8003A590();
                    func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel(0x12, 0, 0, 7, 1, 1);
                    func_800005CC(3.0f, 0);
                    D_33C = 1;
                    return;
                }
            } else if (*modeFlag == 1) {
                if ((o50Overlay1StateReloc == 0) && (joyGetPressed(0) & 0x9000) && (D_33C == 0)) {
                    func_800016EC(1);
                    func_8003A590();
                    func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel(0x12, 0, 0, 7, 1, 0);
                    func_800005CC(3.0f, 0);
                    D_33C = 1;
                }
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o050/func_overlay_050_F0000334_1896CA4/func_overlay_050_F0000334_1896CA4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_050_F0000334_1896CA4:start
 * symbol: func_overlay_050_F0000334_1896CA4
 * score: 1115 differing words
 * frame: 0x110
 * relocations: 315
 * first-mismatch: +0x0
 * summary: Size closed at 1575 via L144 remainder plus inlined lap peel. Colour packing 1094 of 1115. Structural insertions remain.
 * PLATEAU-HANDOFF:func_overlay_050_F0000334_1896CA4:end
 */
