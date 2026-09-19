#include "PR/ultratypes.h"
#include "game/menu.h"
#include "game/gameVi.h"
#include "overlays/overlay_045.h"
#include "overlays/overlay_056.h"

typedef struct RcpTextureInfo RcpTextureInfo;

/* Same 0x10-byte entry layout used by overlay52CopyOffsetEntries. */
typedef struct Overlay52Entry {
    RcpTextureInfo *resource;
    RcpTextureInfo *alternate;
    u32 value8;
    s16 x;
    s16 y;
} Overlay52Entry;

/* Partial racer layout, from this overlay's accesses and Mickey's vehicle code. */
typedef struct Overlay52Racer {
    s8 playerIndex;
    u8 pad001[0x192 - 1];
    u8 value192;
    u8 pad193[7];
    u8 item;
    u8 itemCount;
    s32 value19C;
    u8 pad1A0[8];
    u16 flags;
    u8 pad1AA[0x383 - 0x1AA];
    s8 lap;
    u8 pad384;
    u8 position;
    u8 pad386[2];
    u8 value388;
    u8 pad389[0x3BA - 0x389];
    s16 value3BA;
    u8 pad3BC[0x400 - 0x3BC];
    s32 time;
    u8 pad404[0x454 - 0x404];
    s16 timeDifference;
    s16 timeDifferenceTimer;
    u8 pad458[4];
    u8 value45C;
} Overlay52Racer;

typedef struct Overlay52Object {
    u8 pad00[0x64];
    Overlay52Racer *racer;
} Overlay52Object;

typedef struct Overlay52Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay52Level;

/* Same 0x20-byte record used by menu.c's MenuCurrentObject. */
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
extern s32 D_8007C1B0;
extern s32 D_800C947C;
extern s32 ext_o1_83e0;

/* Section-qualified names prevent the extracted data/BSS zero bases aliasing. */
extern Overlay52Entry o52_bss_0[2][3];
extern Overlay52Entry o52_bss_60[2][2];
extern Overlay52Entry o52_bss_A0[2][3];
extern Overlay52Entry o52_bss_100[2][3];
extern Overlay52Entry o52_bss_160[10];
extern Overlay52Entry o52_bss_200[2][10];
extern Overlay52Entry o52_bss_340[2][10];
extern Overlay52Entry o52_bss_480[2];
extern s16 o52_bss_4A0[2];
extern s32 o52_bss_4A8[2];
extern f32 o52_bss_4B0;
extern s16 o52_bss_4B4[2];
extern s16 o52_bss_4B8[2];
extern s16 o52_bss_4BC[2];
extern s16 o52_bss_4C0[2];
extern s16 o52_bss_4C4[2];
extern s16 o52_bss_4C8[2];
extern Overlay45ResourceDescriptor *o52_bss_4CC;
extern s16 o52_bss_4D0;
extern s16 o52_bss_4D2;
extern Overlay52Entry o52_data_80[];
extern Overlay52Entry o52_data_F0[9];
extern Overlay52Entry o52_data_180[];
extern s8 o52_data_240[2];
extern s8 o52_data_244[];
extern s16 o52_data_24C[2];
extern s16 o52_data_254[2];
extern s16 o52_data_258[2];
extern s16 o52_data_25C[2];
extern s16 o52_data_260[2];
extern s16 o52_data_264[2];
extern s16 o52_data_268[2];
extern s16 o52_data_26C[2];
extern s16 o52_data_270[2];
extern s16 o52_data_274[2];
extern s16 o52_data_278[2];
extern Overlay52Entry o52_data_2F4[];
extern s32 o52_data_314;
extern s32 o52_data_318;
extern s8 o52_data_31C;
extern s32 o52_data_320[2];

extern void camStandardOrtho(MenuCommand **, void **);
extern Overlay52Object **func_80005750(s32 *);
extern Overlay52Level *levelGetLevel(void);
extern s32 func_80036544(u8 *, s32 *, s32, f32 *, s32);
extern u16 joyGetPressed(s32);
extern void camSetNo(s32);
extern void camSetScissor(MenuCommand **);
extern void func_8002F618(MenuCommand **, Overlay52Entry *, s32, s32, u8, u8, u8, u8);
extern void func_80034920(MenuCommand **);
extern void func_80034DE4(s32);
extern void func_80039E34(s32);
extern void func_overlay_052_F0000540_189ABB0(Overlay52Entry *, Overlay52Entry *, s32, s32);
extern s32 func_800290A0(void);
extern s32 func_8003A7D0(Overlay52Object *);
extern void amSndPlay(u16, void **);
extern s32 mainGetMode(void);
extern u8 *func_80028F54(void);
extern void mainChangeCameras(s32);
extern void func_800016EC(u8);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_800005CC(f32, u8);

/* Reconstructed from Mickey's call/global relocation identities and field accesses. */
#ifdef NON_MATCHING
void func_overlay_052_F000063C_189ACAC(s32 updateRate) {
    s32 i;
    s32 buttons;
    s32 player;
    s32 desiredItems[2];
    s32 mode;
    s32 split;
    Overlay52Object **racers;
    Overlay52Level *level;
    Overlay52Object *object;
    Overlay52Racer *racer;
    Overlay52Entry *digits;
    s32 racerCount;
    Overlay52Entry *secondary;
    s32 minutes;
    s32 seconds;
    s32 hundredths;
    s32 hudOffset;
    Overlay52Entry icon[2];
    s32 width;
    u32 halfHeight;
    s32 value0;
    s32 value1;
    s32 value2;
    s32 active;
    s32 alpha;
    s32 difference;
    s32 *slot;
    f32 step;

    viGetCurrentSize(&width, (s32 *)&halfHeight);
    halfHeight >>= 1;
    split = frontGet2PlayerSplit();
    mode = frontGetScreenMode() & 1;
    camStandardOrtho(&D_800D3140, &D_800D3144);
    if (o52_bss_4CC != 0) {
        o52_bss_4D0 += updateRate;
        if (o52_bss_4D0 >= 61) {
            if (o52_bss_4D0 >= 241) {
                o52_bss_4D2 -= updateRate * 4;
                if (o52_bss_4D2 < 0) {
                    overlay45ReleaseDescriptor(o52_bss_4CC);
                    o52_bss_4CC = 0;
                } else {
                    overlay45SetMode(o52_bss_4CC, o52_bss_4D2);
                }
            } else {
                o52_bss_4D2 += updateRate * 4;
                if (o52_bss_4D2 >= 256) {
                    o52_bss_4D2 = 255;
                }
                overlay45SetMode(o52_bss_4CC, o52_bss_4D2);
            }
        }
    }
    racers = func_80005750(&racerCount);
    if (mode != 0) {
        o52_data_24C[0] = 0;
        o52_data_24C[1] = -120;
    } else {
        o52_data_24C[0] = -12;
        o52_data_24C[1] = -120;
    }
    if (D_800C947C == 0 && updateRate > 0) {
        i = 0;
        value0 = updateRate & 3;
        if (value0 != 0) {
            step = (-11.0f - o52_bss_4B0) * 0.125f;
            for (;;) {
                i++;
                if (i == value0) {
                    break;
                }
                o52_bss_4B0 += step;
                step = (-11.0f - o52_bss_4B0) * 0.125f;
            }
            o52_bss_4B0 += step;
        }
        if (i != updateRate) {
            step = (-11.0f - o52_bss_4B0) * 0.125f;
            for (;;) {
                i += 4;
                if (i == updateRate) {
                    break;
                }
                o52_bss_4B0 += step;
                o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
                o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
                o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
                step = (-11.0f - o52_bss_4B0) * 0.125f;
            }
            o52_bss_4B0 += step;
            o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
            o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
            o52_bss_4B0 += (-11.0f - o52_bss_4B0) * 0.125f;
        }
    }
    func_80036544(D_800D31C8[2], &o52_data_314, 20, &D_800D3550[2].unk18, updateRate);
    func_80036544(D_800D31C8[40], &o52_data_314, 20, &D_800D3550[1].unk18, updateRate);
    hudOffset = (s32)o52_bss_4B0;
    level = levelGetLevel();
    o52_data_31C++;
    o52_data_31C %= 10;
    desiredItems[0] = -1;
    desiredItems[1] = -1;

    for (player = 0; player < 2; player++) {
        object = racers[player];
        if (object == NULL) {
            return;
        }
        racer = object->racer;
        if (racer->item != 255) {
            o52_data_320[player] += updateRate * 16;
            if (o52_data_320[player] >= 256) {
                o52_data_320[player] = 255;
            }
        } else {
            o52_data_320[player] -= updateRate * 8;
            if (o52_data_320[player] < 0) {
                o52_data_320[player] = 0;
            }
        }
        if (o52_data_320[player] > 0) {
            if (racer->value19C != 0) {
                desiredItems[player] = 53;
            } else if (racer->item != 255) {
                desiredItems[player] = D_8007C180[racer->item];
            } else {
                desiredItems[player] = o52_bss_4A8[player];
            }
            if (desiredItems[player] != o52_bss_4A8[player]) {
                if (o52_bss_4A8[player] != -1) {
                    freeFrontEndItem(o52_bss_4A8[player]);
                }
                o52_bss_4A8[player] = desiredItems[player];
                if (o52_bss_4A8[player] != -1) {
                    loadFrontEndItem(o52_bss_4A8[player]);
                }
            }
        }
    }
    for (slot = o52_bss_4A8; slot < o52_bss_4A8 + 2; slot++) {
        if (*slot != -1 && *slot != desiredItems[0] && *slot != desiredItems[1]) {
            freeFrontEndItem(*slot);
            *slot = -1;
        }
    }
    slot = o52_bss_4A8;
    for (i = 0; i < 2; i++, slot++) {
        if (desiredItems[i] != -1) {
            *slot = desiredItems[i];
            if (D_800D31C8[desiredItems[i]] == NULL) {
                loadFrontEndItem(desiredItems[i]);
            }
        }
    }

    for (player = 0; player < 2; player++) {
        object = racers[player];
        if (object == NULL) {
            return;
        }
        digits = o52_bss_0[player];
        if (joyGetPressed(player) & 2) {
            o52_data_240[player] ^= 1;
        }
        racer = object->racer;
        camSetNo(player);
        camSetScissor(&D_800D3140);
        if (racer->value3BA != 255) {
            digits[0].value8 = racer->value3BA << 16;
            digits[1].value8 = o52_data_244[racer->value3BA] << 16;
        } else {
            digits[0].value8 = racer->position << 16;
            digits[1].value8 = o52_data_244[racer->position] << 16;
        }
        if (racer->flags & 8) {
            func_8002F618(&D_800D3140, o52_bss_60[player], 0, hudOffset, 255, 255, 255, 255);
        } else {
            func_8002F618(&D_800D3140, digits, 0, hudOffset, 255, 255, 255, 255);
        }
        if (o52_data_240[player] != 0) {
            digits = o52_bss_A0[player];
            secondary = o52_bss_100[player];
            func_overlay_052_F0000540_189ABB0(o52_data_80, digits, player, 1);
            digits[0].value8 = (racer->value192 / 10) << 16;
            digits[1].value8 = (racer->value192 % 10) << 16;
            if (((s32)digits[0].value8 >> 16) == 1) {
                digits[0].x++;
            }
            if (((s32)digits[1].value8 >> 16) == 1) {
                digits[1].x--;
            }
            func_8002F618(&D_800D3140, digits, 0, hudOffset, 255, 255, 255, 255);
            value0 = racer->lap + 1;
            if (racer->value45C != 0) {
                value0++;
            }
            if (value0 >= 4) {
                value0 = 3;
            }
            if (value0 <= 0) {
                value0 = 1;
            }
            secondary[1].value8 = value0 << 16;
            func_8002F618(&D_800D3140, secondary, 0, hudOffset, 255, 255, 255, 255);
            func_80034920(&D_800D3140);
            if (split != 0) {
                D_800D3550[1].unkC = o52_data_254[racer->playerIndex];
                D_800D3550[1].unk10 = 71 - hudOffset;
            } else {
                D_800D3550[1].unkC = -55.0f;
                D_800D3550[1].unk10 = o52_data_24C[player] - hudOffset + 83;
            }
            func_80034DE4(0);
            func_80039E34(1);
            func_80034DE4(1);
            func_80034920(&D_800D3140);
        } else {
            digits = o52_bss_200[player];
            secondary = o52_bss_340[player];
            overlay56SplitTime(racer->time, &minutes, &seconds, &hundredths);
            if (D_800C947C == 0 && level->laps != racer->lap &&
                func_800290A0() == 0 && func_8003A7D0(object) != racer->time) {
                hundredths = hundredths - hundredths % 10 + o52_data_31C;
            }
            func_overlay_052_F0000540_189ABB0(o52_data_180, digits, player, 3);
            o52_bss_200[player][0].value8 = (minutes / 10) << 16;
            o52_bss_200[player][1].value8 = (minutes % 10) << 16;
            o52_bss_200[player][3].value8 = (seconds / 10) << 16;
            o52_bss_200[player][4].value8 = (seconds % 10) << 16;
            o52_bss_200[player][6].value8 = (hundredths / 10) << 16;
            o52_bss_200[player][7].value8 = (hundredths % 10) << 16;
            for (i = 0; i < 8; i++) {
                if (((s32)o52_bss_200[player][i].value8 >> 16) == 1) {
                    if (i == 0 || i == 3 || i == 6) {
                        o52_bss_200[player][i].x++;
                    } else {
                        o52_bss_200[player][i].x--;
                    }
                }
            }
            func_8002F618(&D_800D3140, digits, 0, hudOffset, 255, 255, 255, 255);
            func_80034920(&D_800D3140);
            if (split != 0) {
                D_800D3550[4].unkC = o52_data_260[player];
                D_800D3550[4].unk10 = 80 - hudOffset;
            } else {
                D_800D3550[4].unkC = -44.0f;
                D_800D3550[4].unk10 = o52_data_24C[player] - hudOffset + 92;
            }
            D_800D3550[4].unk4 = (s32)(0U - ((u32)racer->time << 16)) / 300;
            func_80039E34(4);
            func_8002F618(&D_800D3140, secondary, 0, hudOffset, 255, 255, 255, 255);
        }
        if (o52_data_320[player] > 0 && o52_bss_4A8[player] != -1) {
            if (split != 0) {
                if (o52_bss_4A8[player] == 53) {
                    icon[0].y = 180;
                    icon[0].x = o52_data_258[player];
                } else {
                    icon[0].y = 186;
                    icon[0].x = o52_data_25C[player];
                }
            } else if (o52_bss_4A8[player] == 53) {
                icon[0].x = 25;
                icon[0].y = 62 - o52_data_24C[player];
            } else {
                icon[0].x = 31;
                icon[0].y = 68 - o52_data_24C[player];
            }
            icon[0].value8 = 0;
            icon[0].alternate = NULL;
            icon[1].resource = NULL;
            icon[0].resource = D_800D31C8[o52_bss_4A8[player]];
            func_8002F618(&D_800D3140, icon, 0, 0, 255, 255, 255, o52_data_320[player]);
            if (o52_bss_4A8[player] != 53 && racer->itemCount >= 2) {
                value2 = icon[0].x;
                active = icon[0].y;
                o52_data_2F4[0].value8 = racer->itemCount << 16;
                alpha = (o52_data_320[player] - (o52_data_320[player] >> 1)) & 255;
                func_8002F618(&D_800D3140, o52_data_2F4, value2 + 28, active + 27, 0, 0, 0, alpha);
                func_8002F618(&D_800D3140, o52_data_2F4, value2 + 30, active + 29, 0, 0, 0, alpha);
                func_8002F618(&D_800D3140, o52_data_2F4, value2 + 29, active + 28, 255, 255, 255, 255);
            }
        }
        if (split != 0) {
            value0 = o52_data_264[player];
            value1 = o52_data_268[player];
            value2 = o52_data_26C[player];
        } else {
            value0 = o52_data_270[player];
            value1 = o52_data_274[player];
            value2 = o52_data_278[player];
        }
        if (racer->value388 != 0) {
            active = 1;
            for (i = 0; i < updateRate; i++) {
                o52_bss_4A0[player] += (value0 - o52_bss_4A0[player]) >> 3;
            }
        } else if (value1 != o52_bss_4A0[player]) {
            for (i = 0; i < updateRate; i++) {
                o52_bss_4A0[player] += (value2 - o52_bss_4A0[player]) >> 3;
            }
            if ((value2 >> 6) == (o52_bss_4A0[player] >> 6)) {
                o52_bss_4A0[player] = value1;
                active = 0;
            } else {
                active = 1;
            }
        } else {
            active = 0;
        }
        if (active) {
            o52_bss_480[0].x = o52_bss_4A0[player] >> 4;
            if (split != 0) {
                o52_bss_480[0].y = 60;
            } else if (player == 0) {
                o52_bss_480[0].y = 40;
            } else {
                o52_bss_480[0].y = 160;
            }
            func_8002F618(&D_800D3140, o52_bss_480, 0, 0, 255, 255, 255, 192);
        }
        if (racer->lap < level->laps) {
            if (racer->timeDifferenceTimer >= updateRate) {
                racer->timeDifferenceTimer -= updateRate;
                for (i = 0; i < updateRate; i++) {
                    o52_bss_4B4[racer->playerIndex] +=
                        (o52_bss_4C4[racer->playerIndex] - o52_bss_4B4[racer->playerIndex]) >> 3;
                    o52_bss_4B8[racer->playerIndex] +=
                        (o52_bss_4C8[racer->playerIndex] - o52_bss_4B8[racer->playerIndex]) >> 3;
                }
            } else if (o52_bss_4B4[racer->playerIndex] != o52_bss_4BC[racer->playerIndex]) {
                if (racer->timeDifferenceTimer != -1 && racer->timeDifference < 0) {
                    amSndPlay(505, NULL);
                    racer->timeDifferenceTimer = -1;
                }
                for (i = 0; i < updateRate; i++) {
                    o52_bss_4B4[racer->playerIndex] += (6400 - o52_bss_4B4[racer->playerIndex]) >> 3;
                    o52_bss_4B8[racer->playerIndex] +=
                        (halfHeight * racer->playerIndex * 16 - o52_bss_4B8[racer->playerIndex] - 320) >> 3;
                }
                if (o52_bss_4B4[racer->playerIndex] >= 6241) {
                    o52_bss_4B4[racer->playerIndex] = o52_bss_4BC[racer->playerIndex];
                    o52_bss_4B8[racer->playerIndex] = o52_bss_4C0[racer->playerIndex];
                }
            }
            if (racer->timeDifference <= 0) {
                o52_bss_160[0].value8 = 12 << 16;
                o52_bss_160[0].resource = D_800D31C8[20];
                o52_bss_160[0].alternate = D_800D31C8[21];
                difference = -racer->timeDifference;
                for (i = 1; i < 9; i++) {
                    o52_bss_160[i].resource = D_800D31C8[20];
                    o52_bss_160[i].alternate = D_800D31C8[21];
                }
            } else {
                o52_bss_160[0].value8 = 13 << 16;
                o52_bss_160[0].resource = D_800D31C8[80];
                o52_bss_160[0].alternate = D_800D31C8[21];
                difference = racer->timeDifference;
                for (i = 1; i < 9; i++) {
                    o52_bss_160[i].resource = D_800D31C8[80];
                    o52_bss_160[i].alternate = D_800D31C8[21];
                }
            }
            overlay56SplitTime(difference, &minutes, &seconds, &hundredths);
            o52_bss_160[1].value8 = (minutes / 10) << 16;
            o52_bss_160[2].value8 = (minutes % 10) << 16;
            o52_bss_160[4].value8 = (seconds / 10) << 16;
            o52_bss_160[5].value8 = (seconds % 10) << 16;
            o52_bss_160[7].value8 = (hundredths / 10) << 16;
            o52_bss_160[8].value8 = (hundredths % 10) << 16;
            for (i = 0; i < 8; i++) {
                if (((s32)o52_bss_160[i + 1].value8 >> 16) == 1) {
                    if (i == 0 || i == 3 || i == 6) {
                        o52_bss_160[i + 1].x = o52_data_F0[i].x + 1;
                    } else {
                        o52_bss_160[i + 1].x = o52_data_F0[i].x - 1;
                    }
                } else {
                    o52_bss_160[i + 1].x = o52_data_F0[i].x;
                }
            }
            func_8002F618(&D_800D3140, o52_bss_160,
                         o52_bss_4B4[racer->playerIndex] >> 4,
                         o52_bss_4B8[racer->playerIndex] >> 4, 255, 255, 255, 254);
        }
        if (joyGetPressed(player) & 1) {
            D_8007C1B0 ^= 1;
        }
        if (mainGetMode() == 0) {
            switch (*func_80028F54()) {
                case 3:
                    if (ext_o1_83e0 == 0) {
                        buttons = joyGetPressed(0) & 65535;
                        if (((joyGetPressed(1) | buttons) & 0x9000) && o52_data_318 == 0) {
                            mainChangeCameras(1);
                            func_800016EC(1);
                            func_8003A590();
                            func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
                            mainChangeLevel(18, 0, 0, 7, 1, 1);
                            func_800005CC(3.0f, 0);
                            o52_data_318 = 1;
                        }
                    }
                    break;
                case 4:
                    if (ext_o1_83e0 == 0) {
                        buttons = joyGetPressed(0) & 65535;
                        if (((joyGetPressed(1) | buttons) & 0x9000) && o52_data_318 == 0) {
                            mainChangeCameras(1);
                            func_800016EC(1);
                            func_8003A590();
                            func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
                            mainChangeLevel(18, 0, 0, 7, 1, 1);
                            func_800005CC(3.0f, 0);
                            o52_data_318 = 1;
                        }
                    }
                    break;
            }
        }
    }
    camSetNo(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o052/overlay52TailB/func_overlay_052_F000063C_189ACAC.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_052_F000063C_189ACAC:start
 * symbol: func_overlay_052_F000063C_189ACAC
 * score: 1337 differing words
 * frame: 0x118
 * relocations: 310
 * first-mismatch: +0x130
 * summary: Live size 6748/0. Counted recurrence does not unroll. L160 slot/digits and L99/L100 probes inert or worse. Next: shared 24C lui and blez delay of i=0.
 * PLATEAU-HANDOFF:func_overlay_052_F000063C_189ACAC:end
 */
