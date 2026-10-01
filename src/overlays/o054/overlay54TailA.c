#include "game/charControl.h"
#include "game/menu.h"
#include "n_audio/mbi.h"
#include "overlays/offset_records.h"
#include "overlays/overlay_045.h"

#ifdef NON_MATCHING
/* Tier B: overlay54PatchIndices stores resolved resource addresses in the
 * signed words copied by overlay54CopyOffsetRecords. Retain that shared
 * record type and the project's explicit pointer-to-word conversion. */

/* Tier B: overlay 54's runtime records identify the resident calls and the
 * overlay 45/56 exports below. Their source declarations establish the ABI.
 * These externs retain overlay-specific linkage until the body is exact. */
extern void camStandardOrtho_o054Reloc(MenuCommand **dlist, Mtx **matrix);
extern void camSetNo_o054Reloc(s32 camera);
extern void camSetScissor_o054Reloc(MenuCommand **dlist);
extern void **func_80005750_o054Reloc(s32 *count);
extern u8 *levelGetLevel_o054Reloc(void);
extern s32 func_800290A0_o054Reloc(void);
extern s32 func_8003A7D0_o054Reloc(ControlActor *actor);
extern void viGetCurrentSize_o054Reloc(s32 *width, s32 *height);
extern s32 func_80036544_o054Reloc(u8 *resource, s32 *state,
    s32 animation, f32 *frame, s32 updateRate);
extern void func_8002F618_o054Reloc(MenuCommand **dlist,
    OverlayOffsetRecord *records, s32 x, s32 y, u8 red, u8 green,
    u8 blue, u8 alpha);
/* The last parameter is unsigned: the shipped icon call materializes its
 * literal 1 in a fresh temporary instead of reusing the function's s32
 * constant-1 register, which a signed parameter does (one ring draw, worth
 * about 340 aligned rows of temp-register rotation downstream). */
extern void func_8002FB34_o054Reloc(MenuCommand **dlist,
    OverlayOffsetRecord *records, f32 x, f32 y, f32 scaleX, f32 scaleY,
    s32 colour, u8 mode);
extern u16 joyGetPressed_o054Reloc(s32 player);
extern void func_80034920_o054Reloc(MenuCommand **dlist);
extern void func_80034DE4_o054Reloc(s32 mode);
extern void func_80039E34_o054Reloc(s32 index);
extern s32 frontGetScreenMode_o054Reloc(void);
extern s32 mainGetMode_o054Reloc(void);
extern void *func_80028F54_o054Reloc(void);
extern void mainChangeCameras_o054Reloc(s32 cameras);
extern void func_800016EC_o054Reloc(u8 mode);
extern void func_8003A590_o054Reloc(void);
extern void func_80037414_o054Reloc(s32 kind, f32 duration, f32 delay,
    s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void mainChangeLevel_o054Reloc(s32 level, s32 character,
    s32 animation, s32 mode, s32 arg4, s32 arg5);
extern void func_800005CC_o054Reloc(f32 fade, u8 volume);
extern void amSndPlay_o054Reloc(u16 sound, void **handle);
extern void overlay45ReleaseDescriptor_o054Reloc(Overlay45ResourceDescriptor *descriptor);
extern void overlay45SetMode_o054Reloc(Overlay45ResourceDescriptor *descriptor, s32 value);
extern void overlay56SplitTime_o054Reloc(s32 time, s32 *minutes,
    s32 *seconds, s32 *centiseconds);
extern void overlay54GetOffsets(s32 player, s32 kind, s32 *x, s32 *y);

/* The current-object layout is shared with src/main/menu.c. */
typedef struct O54MenuObject {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    s16 index;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 frame;
    s8 flags[4];
} O54MenuObject;

extern MenuCommand *D_800D3140_o054Reloc;
extern Mtx *D_800D3144_o054Reloc;
extern void *D_800D31C8_o054Reloc[];
extern O54MenuObject D_800D3550_o054Reloc[];
extern u8 D_8007BEF4_o054Reloc;
extern s32 D_800C947C_o054Reloc;
extern s16 D_8007C180_o054Reloc[];
extern s32 D_8007C1B0_o054Reloc;
extern s32 o001_data_83E0_o054Reloc;

/* Overlay 54 was one translation unit: its runtime records address this
 * function's .data and .bss through LOCAL (section-relative) records, and
 * the shipped code shares one high half between stores to adjacent fields
 * of record 0, which as1 does only for a symbol defined in the same TU.
 * So the candidate defines the overlay's data exactly as
 * overlay54Initialize.c does (same order, bytes and section sizes, checked
 * against that object) with the three slide limits as separate arrays; the
 * target addresses them through three high halves. A promotion must drop
 * these sections and rebind the records, as overlay54Initialize's rule does.
 * The o54* names below are the old section-offset aliases. */
static s16 sOverlay54ResourceIds[18] = {
    2, 38, 39, 25, 26, 20, 21, 30, 31, 22, 40, 23, 24, 53, 80, 100, -1, 0,
};
static s16 sOverlay54PrepareIds[4] = { 4, 2, 3, -1 };
static OverlayOffsetRecord sOverlay54ListA[2] = { { 38, 39, 0, 0, 0 }, };
static OverlayOffsetRecord sOverlay54ListB[2] = { { 38, 39, 0x00060000, 0, 0 }, };
static OverlayOffsetRecord sOverlay54ListC[3] = { { 20, 21, 0, 21, 0 }, { 20, 21, 0, 28, 0 }, };
static OverlayOffsetRecord sOverlay54ListD[3] = { { 30, 31, 0, 0, -3 }, { 20, 21, 0, 32, 1 }, };
static OverlayOffsetRecord sOverlay54ListE[10] = {
    { 20, 21, 0, 0, 0 }, { 20, 21, 0, 7, 0 }, { 20, 21, 0x000B0000, 14, 0 },
    { 20, 21, 0, 20, 0 }, { 20, 21, 0, 27, 0 }, { 20, 21, 0x000A0000, 33, 0 },
    { 20, 21, 0, 40, 0 }, { 20, 21, 0, 47, 0 }, { 23, 24, 0, -25, -8 },
};
static OverlayOffsetRecord sOverlay54ListF[2] = { { 25, 0, 0, -25, -6 }, };
static s16 sOverlay54EnterX[2] = { 0x300, 0xC00 };
static s16 sOverlay54HiddenX[2] = { -0x420, 0x4E0 };
static s16 sOverlay54LeaveX[2] = { 0xC80, 0x1580 };
static s16 sOverlay54Offsets[32] = {
    23, 24, 281, 132, 48, 33, 235, 141, 91, 32, 185, 140, 76, 33, 215, 141,
    23, 12, 281, 132, 48, 25, 235, 145, 91, 24, 185, 144, 76, 25, 215, 145,
};
static OverlayOffsetRecord sOverlay54SourceRecords[10] = {
    { 20, 21, 0, -7, 0 }, { 20, 21, 0, 0, 0 }, { 20, 21, 0, 7, 0 },
    { 20, 21, 0x000B0000, 14, 0 }, { 20, 21, 0, 20, 0 }, { 20, 21, 0, 27, 0 },
    { 20, 21, 0x000A0000, 33, 0 }, { 20, 21, 0, 40, 0 }, { 20, 21, 0, 47, 0 },
};
static OverlayOffsetRecord sOverlay54ListG[2] = { { 20, 21, 0, -3, -4 }, };
static s32 sOverlay54Tail298[4] = { 0 };
static s32 sOverlay54Tail2A8 = 9;
static s32 sOverlay54Tail2AC = 0;
static s8 sOverlay54Tail2B0[4] = { 0 };
static s32 sOverlay54Tail2B4[7] = { 0 };

static OverlayOffsetRecord sOverlay54Records[10];
static OverlayOffsetRecord sOverlay54State[2];
static OverlayOffsetRecord sOverlay54ListCopyA[4][2];
static OverlayOffsetRecord sOverlay54ListCopyB[4][2];
static OverlayOffsetRecord sOverlay54ListCopyC[4][3];
static OverlayOffsetRecord sOverlay54ListCopyD[4][3];
static OverlayOffsetRecord sOverlay54ListCopyE[4][10];
static OverlayOffsetRecord sOverlay54ListCopyF[4][2];
static s16 sOverlay54Values[4];
static s16 sOverlay54Sentinels[4];
static s32 sOverlay54Mode;
static s8 sOverlay54Flags[4];
static f32 sOverlay54Height;
static f32 sOverlay54Scale;
static s16 sOverlay54Bounds[4];
static Overlay45ResourceDescriptor *sOverlay54Current;
static s16 sOverlay54Tail66C;
static s16 sOverlay54Tail66E;

#define o54Bss_0 sOverlay54Records
#define o54Bss_10 (&sOverlay54Records[1])
#define o54Bss_A0 sOverlay54State
#define o54Bss_C0 sOverlay54ListCopyA
#define o54Bss_140 sOverlay54ListCopyB
#define o54Bss_1C0 sOverlay54ListCopyC
#define o54Bss_280 sOverlay54ListCopyD
#define o54Bss_340 sOverlay54ListCopyE
#define o54Bss_5C0 sOverlay54ListCopyF
#define o54Bss_640 sOverlay54Values
#define o54Bss_648 sOverlay54Sentinels
#define o54Bss_654 sOverlay54Flags
#define o54Bss_658 sOverlay54Height
#define o54Bss_65C sOverlay54Scale
#define o54Bss_660 sOverlay54Bounds
#define o54Bss_668 sOverlay54Current
#define o54Bss_66C sOverlay54Tail66C
#define o54Bss_66E sOverlay54Tail66E
#define o54Data_78 sOverlay54ListC[0].x
#define o54Data_88 sOverlay54ListC[1].x
#define o54Data_CC sOverlay54ListE
#define o54Data_18C sOverlay54EnterX
#define o54Data_190 sOverlay54HiddenX
#define o54Data_194 sOverlay54LeaveX
#define o54Data_1E8 (&sOverlay54SourceRecords[1])
#define o54Data_278 sOverlay54ListG
#define o54Data_298 sOverlay54Tail298
#define o54Data_2A8 sOverlay54Tail2A8
#define o54Data_2AC sOverlay54Tail2AC
#define o54Data_2B0 sOverlay54Tail2B0[0]
#define o54Data_2B4 sOverlay54Tail2B4

/* These fields fall in gaps in the partial ControlPlayer header. Their
 * signedness and widths come from this function's own loads. */
#define O54_LAP(p) ((s8) (p)->pad37C[7])
#define O54_PLACE(p) ((p)->pad37C[9])
#define O54_TIME(p) (*(s32 *) ((p)->pad3FC + 4))
#define O54_TIME_DELTA(p) (*(s16 *) (p)->pad454)

void func_overlay_054_F00005AC_189F24C(s32 updateRate) {
    s32 playerIndex;
    s32 xOffset;
    s32 yOffset;
    s32 actorCount;
    s32 screenY;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudY;
    u32 width;
    u32 height;
    s32 i;
    s32 value;
    s32 x;
    s32 y;
    s32 visible;
    s32 buttons;
    s32 texture;
    s32 alternate;
    s16 enterX;
    s16 hiddenX;
    s16 leaveX;
    s32 resetX;
    s32 deltaTime;
    ControlActor **actors;

    camStandardOrtho_o054Reloc(&D_800D3140_o054Reloc, &D_800D3144_o054Reloc);
    if (o54Bss_668 != NULL) {
        o54Bss_66C += updateRate;
        if (o54Bss_66C >= 61) {
            if (o54Bss_66C >= 241) {
                o54Bss_66E -= updateRate * 4;
                if (o54Bss_66E < 0) {
                    overlay45ReleaseDescriptor_o054Reloc(o54Bss_668);
                    o54Bss_668 = NULL;
                } else {
                    overlay45SetMode_o054Reloc(o54Bss_668, o54Bss_66E);
                }
            } else {
                o54Bss_66E += updateRate * 4;
                if (o54Bss_66E >= 256) {
                    o54Bss_66E = 255;
                }
                overlay45SetMode_o054Reloc(o54Bss_668, o54Bss_66E);
            }
        }
    }
    actors = (ControlActor **) func_80005750_o054Reloc(&actorCount);
    if (D_800C947C_o054Reloc == 0) {
        f32 step;
        f32 priorHeight;

        i = 0;
        if (updateRate > 0) {
            s32 remainder;

            remainder = updateRate & 3;
            if (remainder != 0) {
                i++;
                priorHeight = o54Bss_658;
                step = (-11.0f - priorHeight) * 0.125f;
                if (i != remainder) {
                    while (1) {
                        i++;
                        o54Bss_658 = priorHeight + step;
                        priorHeight = o54Bss_658;
                        step = (-11.0f - priorHeight) * 0.125f;
                        if (i == remainder) {
                            break;
                        }
                    }
                }
                o54Bss_658 = priorHeight + step;
            }
            if (i != updateRate) {
                i += 4;
                priorHeight = o54Bss_658;
                step = (-11.0f - priorHeight) * 0.125f;
                if (i != updateRate) {
                    while (1) {
                        i += 4;
                        o54Bss_658 = priorHeight + step;
                        o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
                        o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
                        o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
                        priorHeight = o54Bss_658;
                        step = (-11.0f - priorHeight) * 0.125f;
                        if (i == updateRate) {
                            break;
                        }
                    }
                }
                o54Bss_658 = priorHeight + step;
                o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
                o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
                o54Bss_658 += (-11.0f - o54Bss_658) * 0.125f;
            }
        }
    }
    func_80036544_o054Reloc(D_800D31C8_o054Reloc[2], &o54Data_2A8,
        20, &D_800D3550_o054Reloc[2].frame, updateRate);
    func_80036544_o054Reloc(D_800D31C8_o054Reloc[40], &o54Data_2A8,
        20, &D_800D3550_o054Reloc[1].frame, updateRate);
    hudY = (s32) o54Bss_658;
    viGetCurrentSize_o054Reloc((s32 *) &width, (s32 *) &height);
    o54Data_2B0++;
    o54Data_2B0 %= 10;

    for (playerIndex = 0; playerIndex < D_8007BEF4_o054Reloc; playerIndex++) {
        s8 *level;
        ControlActor *actor;
        ControlPlayer *player;
        OverlayOffsetRecord *position;
        OverlayOffsetRecord *lap;
        OverlayOffsetRecord *lapCount;
        OverlayOffsetRecord *timer;
        const OverlayOffsetRecord *src;
        OverlayOffsetRecord *dst;
        s32 *displayMode;

        actor = actors[playerIndex];
        if (actor == NULL) {
            return;
        }
        player = actor->player;
        position = o54Bss_C0[playerIndex];
        lap = o54Bss_1C0[playerIndex];
        lapCount = o54Bss_280[playerIndex];
        timer = o54Bss_340[playerIndex];
        displayMode = &o54Data_298[playerIndex];
        camSetNo_o054Reloc(playerIndex);
        camSetScissor_o054Reloc(&D_800D3140_o054Reloc);
        value = O54_LAP(player) + 1;
        if (player->unk45C != 0) {
            value++;
        }
        if (value >= 4) {
            value = 3;
        }
        if (value <= 0) {
            value = 1;
        }
        lapCount[1].metadata = value << 16;
        lap[0].metadata = (player->unk192 / 10) << 16;
        lap[1].metadata = (player->unk192 % 10) << 16;
        lap[2].metadata = (s32) (o54Bss_65C * 65536.0f);
        overlay54GetOffsets(playerIndex, 1, &xOffset, &yOffset);
        if ((lap[0].metadata >> 16) == 1) {
            lap[0].x = o54Data_78 + xOffset + 1;
        } else {
            lap[0].x = o54Data_78 + xOffset;
        }
        if ((lap[1].metadata >> 16) == 1) {
            lap[1].x = o54Data_88 + xOffset - 1;
        } else {
            lap[1].x = o54Data_88 + xOffset;
        }
        if (player->unk3BA != 255) {
            position[0].metadata = player->unk3BA * 65536;
        } else {
            position[0].metadata = O54_PLACE(player) << 16;
        }
        overlay56SplitTime_o054Reloc(O54_TIME(player), &minutes, &seconds, &centiseconds);
        level = (s8 *) levelGetLevel_o054Reloc();
        if (D_800C947C_o054Reloc == 0 && level[0x86] != O54_LAP(player) &&
            func_800290A0_o054Reloc() == 0 &&
            func_8003A7D0_o054Reloc(actor) != O54_TIME(player)) {
            centiseconds -= centiseconds % 10;
            centiseconds += o54Data_2B0;
        }
        timer[0].metadata = (minutes / 10) * 65536;
        timer[1].metadata = (minutes % 10) * 65536;
        timer[3].metadata = (seconds / 10) * 65536;
        timer[4].metadata = (seconds % 10) * 65536;
        timer[6].metadata = (centiseconds / 10) * 65536;
        timer[7].metadata = (centiseconds % 10) * 65536;
        overlay54GetOffsets(playerIndex, 3, &xOffset, &yOffset);
        for (i = 0; i < 8; i++) {
            if ((timer[i].metadata >> 16) == 1) {
                if (i == 0 || i == 3 || i == 6) {
                    timer[i].x = o54Data_CC[i].x + xOffset + 1;
                } else {
                    timer[i].x = o54Data_CC[i].x + xOffset - 1;
                }
            } else {
                timer[i].x = o54Data_CC[i].x + xOffset;
            }
        }
        if (player->flags1A8 & 8) {
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc,
                o54Bss_140[playerIndex], 0, hudY, 255, 255, 255, 255);
        } else {
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc,
                position, 0, hudY, 255, 255, 255, 255);
        }
        if (joyGetPressed_o054Reloc(playerIndex) & 2) {
            (*displayMode)++;
            if (*displayMode >= 2) {
                *displayMode = 0;
            }
        }
        func_80034920_o054Reloc(&D_800D3140_o054Reloc);
        switch (*displayMode) {
        case 0:
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, lapCount, 0, hudY, 255, 255, 255, 255);
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, lap, 0, hudY, 255, 255, 255, 255);
            overlay54GetOffsets(playerIndex, 1, &xOffset, &yOffset);
            D_800D3550_o054Reloc[1].x = xOffset - 152;
            D_800D3550_o054Reloc[1].y = -yOffset - hudY + 108;
            func_80034DE4_o054Reloc(0);
            func_80039E34_o054Reloc(1);
            func_80034DE4_o054Reloc(1);
            break;
        case 1:
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, timer, 0, hudY, 255, 255, 255, 255);
            func_80034920_o054Reloc(&D_800D3140_o054Reloc);
            overlay54GetOffsets(playerIndex, 3, &xOffset, &yOffset);
            D_800D3550_o054Reloc[4].x = xOffset - 173;
            D_800D3550_o054Reloc[4].y = -yOffset - hudY + 116;
            D_800D3550_o054Reloc[4].rotationZ = (s32) ((u32) O54_TIME(player) * (u32) -65536) / 300;
            func_80039E34_o054Reloc(4);
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc,
                o54Bss_5C0[playerIndex], 0, hudY, 255, 255, 255, 255);
            break;
        }
        if (player->unk19A != 255) {
            o54Data_2B4[playerIndex] += updateRate * 16;
            if (o54Data_2B4[playerIndex] >= 165) {
                o54Data_2B4[playerIndex] = 164;
            }
        } else {
            o54Data_2B4[playerIndex] -= updateRate * 8;
            if (o54Data_2B4[playerIndex] < 0) {
                o54Data_2B4[playerIndex] = 0;
            }
        }
        if (o54Data_2B4[playerIndex] > 0) {
            if (player->unk19C != 0) {
                o54Bss_654[playerIndex] = 53;
            } else if (player->unk19A != 255) {
                o54Bss_654[playerIndex] = D_8007C180_o054Reloc[player->unk19A];
            }
            if (o54Bss_654[playerIndex] != -1) {
                if (frontGetScreenMode_o054Reloc() == 1) {
                    x = (playerIndex & 1) ? 276 : 25;
                    y = (playerIndex & 2) ? 209 : 89;
                } else {
                    x = (playerIndex & 1) ? 276 : 25;
                    y = (playerIndex & 2) ? 197 : 89;
                }
                if (o54Bss_654[playerIndex] == 53) {
                    x -= 7;
                    y -= 6;
                }
                {
                    OverlayOffsetRecord icon[2];

                    icon[0].link = (s32) D_800D31C8_o054Reloc[o54Bss_654[playerIndex]];
                    icon[0].value = 0;
                    icon[0].metadata = 0;
                    icon[0].x = 0;
                    icon[0].y = 0;
                    icon[1].link = 0;
                    func_8002FB34_o054Reloc(&D_800D3140_o054Reloc,
                        icon, (f32) x, (f32) y, 0.66f, 0.66f, o54Data_2B4[playerIndex] | ~255, 1);
                }
                if (o54Bss_654[playerIndex] != 53 && player->unk19B >= 2) {
                    o54Data_278[0].metadata = player->unk19B << 16;
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, o54Data_278,
                        x + 18, y + 18, 0, 0, 0, o54Data_2B4[playerIndex]);
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, o54Data_278,
                        x + 20, y + 20, 0, 0, 0, o54Data_2B4[playerIndex]);
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, o54Data_278,
                        x + 19, y + 19, 255, 255, 255, 255);
                }
            }
        } else {
            o54Bss_654[playerIndex] = -1;
        }
        x = playerIndex & 1;
        enterX = o54Data_18C[x];
        hiddenX = o54Data_190[x];
        leaveX = o54Data_194[x];
        if (player->unk388 != 0) {
            for (i = 0; i < updateRate; i++) {
                o54Bss_660[playerIndex] += (enterX - o54Bss_660[playerIndex]) >> 3;
            }
            visible = 1;
        } else if (hiddenX == o54Bss_660[playerIndex]) {
            visible = 0;
        } else {
            for (i = 0; i < updateRate; i++) {
                o54Bss_660[playerIndex] += (leaveX - o54Bss_660[playerIndex]) >> 3;
            }
            if ((leaveX >> 6) == (o54Bss_660[playerIndex] >> 6)) {
                o54Bss_660[playerIndex] = hiddenX;
                visible = 0;
            } else {
                visible = 1;
            }
        }
        if (visible) {
            o54Bss_A0[0].x = o54Bss_660[playerIndex] >> 4;
            if (frontGetScreenMode_o054Reloc() == 1) {
                o54Bss_A0[0].y = playerIndex < 2 ? 40 : 160;
            } else {
                o54Bss_A0[0].y = playerIndex < 2 ? 45 : 153;
            }
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, o54Bss_A0,
                0, 0, 255, 255, 255, 192);
        }
        x = x ? 2560 : 256;
        if (frontGetScreenMode_o054Reloc() == 1) {
            screenY = (height >> 1) * ((player->playerIndex >> 1) * 16);
        } else {
            screenY = playerIndex < 2 ? 192 : 1920;
        }
        if (O54_LAP(player) < level[0x86]) {
            if (player->unk456 >= updateRate) {
                player->unk456 -= updateRate;
                for (i = 0; i < updateRate; i++) {
                    o54Bss_640[player->playerIndex] +=
                        (x - o54Bss_640[player->playerIndex] + 816) >> 3;
                    o54Bss_648[player->playerIndex] +=
                        (screenY - o54Bss_648[player->playerIndex] + 560) >> 3;
                }
            } else {
                resetX = x - 1280;
                if (resetX != o54Bss_640[player->playerIndex]) {
                    if (player->unk456 != -1 && O54_TIME_DELTA(player) < 0) {
                        amSndPlay_o054Reloc(505, NULL);
                        player->unk456 = -1;
                    }
                    for (i = 0; i < updateRate; i++) {
                        o54Bss_640[player->playerIndex] +=
                            (width * 8 + x - o54Bss_640[player->playerIndex] + 816) >> 3;
                        o54Bss_648[player->playerIndex] +=
                            (screenY - o54Bss_648[player->playerIndex] - 400) >> 3;
                    }
                    if (x + width * 8 + 656 < (u32) o54Bss_640[player->playerIndex]) {
                        o54Bss_640[player->playerIndex] = resetX;
                        o54Bss_648[player->playerIndex] = screenY - 320;
                    }
                }
            }
            if (O54_TIME_DELTA(player) <= 0) {
                o54Bss_0[0].metadata = 12 << 16;
                deltaTime = -O54_TIME_DELTA(player);
                for (i = 0; i < 9; i++) {
                    o54Bss_0[i].link = (s32) D_800D31C8_o054Reloc[20];
                    o54Bss_0[i].value = (s32) D_800D31C8_o054Reloc[21];
                }
            } else {
                o54Bss_0[0].metadata = 13 << 16;
                deltaTime = O54_TIME_DELTA(player);
                for (i = 0; i < 9; i++) {
                    o54Bss_0[i].link = (s32) D_800D31C8_o054Reloc[80];
                    o54Bss_0[i].value = (s32) D_800D31C8_o054Reloc[21];
                }
            }
            overlay56SplitTime_o054Reloc(deltaTime, &minutes, &seconds, &centiseconds);
            o54Bss_0[1].metadata = (minutes / 10) * 65536;
            o54Bss_0[2].metadata = (minutes % 10) * 65536;
            o54Bss_0[4].metadata = (seconds / 10) * 65536;
            o54Bss_0[5].metadata = (seconds % 10) * 65536;
            o54Bss_0[7].metadata = (centiseconds / 10) * 65536;
            o54Bss_0[8].metadata = (centiseconds % 10) * 65536;
            for (i = 1; i < 9; i++) {
                if ((sOverlay54Records[i].metadata >> 16) == 1) {
                    if (i == 1 || i == 4 || i == 7) {
                        sOverlay54Records[i].x = sOverlay54SourceRecords[i].x + 1;
                    } else {
                        sOverlay54Records[i].x = sOverlay54SourceRecords[i].x - 1;
                    }
                } else {
                    sOverlay54Records[i].x = sOverlay54SourceRecords[i].x;
                }
            }
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, o54Bss_0,
                o54Bss_640[player->playerIndex] >> 4,
                o54Bss_648[player->playerIndex] >> 4, 255, 255, 255, 255);
        }
        if (joyGetPressed_o054Reloc(playerIndex) & 1) {
            D_8007C1B0_o054Reloc ^= 1;
        }
        if (mainGetMode_o054Reloc() == 0) {
            buttons = 0;
            for (i = 0; i < D_8007BEF4_o054Reloc; i++) {
                buttons |= joyGetPressed_o054Reloc(i) & 0x9000;
            }
            switch (*(u8 *) func_80028F54_o054Reloc()) {
            case 3:
                if (o001_data_83E0_o054Reloc == 0 && buttons != 0 && o54Data_2AC == 0) {
                    mainChangeCameras_o054Reloc(1);
                    func_800016EC_o054Reloc(1);
                    func_8003A590_o054Reloc();
                    func_80037414_o054Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o054Reloc(18, 0, 0, 7, 1, 1);
                    func_800005CC_o054Reloc(3.0f, 0);
                    o54Data_2AC = 1;
                }
                break;
            case 4:
                if (o001_data_83E0_o054Reloc == 0 && buttons != 0 && o54Data_2AC == 0) {
                    mainChangeCameras_o054Reloc(1);
                    func_800016EC_o054Reloc(1);
                    func_8003A590_o054Reloc();
                    func_80037414_o054Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o054Reloc(18, 0, 0, 7, 1, 1);
                    func_800005CC_o054Reloc(3.0f, 0);
                    o54Data_2AC = 1;
                }
                break;
            }
        }
    }
    camSetNo_o054Reloc(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o054/overlay54TailA/func_overlay_054_F00005AC_189F24C.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_054_F00005AC_189F24C:start
 * symbol: func_overlay_054_F00005AC_189F24C
 * score: 213 differing words
 * frame: 0x150
 * relocations: 273
 * first-mismatch: +0x0
 * summary: Data into C, per-arm 0..9 record loop, 1..8 digit loop: 213 at delta 0; height loop and frame remain.
 * PLATEAU-HANDOFF:func_overlay_054_F00005AC_189F24C:end
 */
