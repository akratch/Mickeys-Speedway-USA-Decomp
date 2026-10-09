#include "game/charControl.h"
#include "game/menu.h"
#include "n_audio/mbi.h"
#include "overlays/offset_records.h"
#include "overlays/overlay_045.h"

/* Tier B: overlay54PatchIndices stores resolved resource addresses in the
 * signed words copied by overlay54CopyOffsetRecords. Retain that shared
 * record type and the project's explicit pointer-to-word conversion. */

/* Tier B: overlay 54's runtime records identify the resident calls and the
 * overlay 45/56 exports below. Their source declarations establish the ABI;
 * the *_o054Reloc names are the generated relocation surface. */
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
extern void amTuneSetFade_o054Reloc(f32 fade, u8 volume);
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
 * So this TU defines the overlay's data exactly as overlay54Initialize.c
 * does (same order, bytes and section sizes), with the three slide-limit
 * pairs as separate arrays because the target addresses them through three
 * high halves. overlay54Initialize.c owns the bytes: this object's .data
 * and .bss are dropped at POSTPROCESS and their records rebound to
 * zero-valued bases (mk/overlays.mk). */
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

/* These fields fall in gaps in the partial ControlPlayer header. Their
 * signedness and widths come from this function's own loads. */
#define O54_LAP(p) ((s8) (p)->pad37C[7])
#define O54_PLACE(p) ((p)->pad37C[9])
#define O54_TIME(p) (*(s32 *) ((p)->pad3FC + 4))
#define O54_TIME_DELTA(p) (*(s16 *) (p)->pad454)

/* Independently reconstructed from Mickey-local evidence; no donor body.
 * Matched 2026-10-01 from an 851-word plateau. What it took, in order:
 *   - the TU is built with -Wab,-r4300_mul (mk/overlays.mk); with it IDO
 *     emits the HUD height easing as the shipped rotated, branch-likely
 *     loop from a plain `for` over the global, which no spelling reaches
 *     without the flag (the family of o050..o055 HUD loops shares this);
 *   - the icon call's mode parameter is unsigned (see its prototype);
 *   - the alpha and item-flag rows are indexed at each use, the record
 *     rows stay declared pointers;
 *   - the time-delta arms fill records 0..8 in one loop each, reading the
 *     textures from the resource table at each use, after assigning the
 *     delta; the digit loop runs over records 1..8 of the same array;
 *   - visibility resets inside its arm, and the bar Y is an if/else;
 *   - every local at function scope, in the order the target's frame
 *     ladder reads (register-only locals fill its unused cells). */
void func_overlay_054_F00005AC_189F24C(s32 updateRate) {
    OverlayOffsetRecord *position;
    OverlayOffsetRecord *lap;
    s32 playerIndex;
    s32 i;
    s32 xOffset;
    s32 yOffset;
    s8 *level;
    s32 value;
    ControlPlayer *player;
    ControlActor *actor;
    s32 x;
    s32 actorCount;
    s32 y;
    s32 screenY;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudY;
    s32 visible;
    s32 buttons;
    s32 resetX;
    s32 deltaTime;
    s32 *displayMode;
    ControlActor **actors;
    OverlayOffsetRecord icon[2];
    u32 width;
    u32 height;
    OverlayOffsetRecord *lapCount;
    OverlayOffsetRecord *timer;
    s16 enterX;
    s16 hiddenX;
    s16 leaveX;

    camStandardOrtho_o054Reloc(&D_800D3140_o054Reloc, &D_800D3144_o054Reloc);
    if (sOverlay54Current != NULL) {
        sOverlay54Tail66C += updateRate;
        if (sOverlay54Tail66C >= 61) {
            if (sOverlay54Tail66C >= 241) {
                sOverlay54Tail66E -= updateRate * 4;
                if (sOverlay54Tail66E < 0) {
                    overlay45ReleaseDescriptor_o054Reloc(sOverlay54Current);
                    sOverlay54Current = NULL;
                } else {
                    overlay45SetMode_o054Reloc(sOverlay54Current, sOverlay54Tail66E);
                }
            } else {
                sOverlay54Tail66E += updateRate * 4;
                if (sOverlay54Tail66E >= 256) {
                    sOverlay54Tail66E = 255;
                }
                overlay45SetMode_o054Reloc(sOverlay54Current, sOverlay54Tail66E);
            }
        }
    }
    actors = (ControlActor **) func_80005750_o054Reloc(&actorCount);
    if (D_800C947C_o054Reloc == 0) {
        for (i = 0; i < updateRate; i++) {
            sOverlay54Height += (-11.0f - sOverlay54Height) * 0.125f;
        }
    }
    func_80036544_o054Reloc(D_800D31C8_o054Reloc[2], &sOverlay54Tail2A8,
        20, &D_800D3550_o054Reloc[2].frame, updateRate);
    func_80036544_o054Reloc(D_800D31C8_o054Reloc[40], &sOverlay54Tail2A8,
        20, &D_800D3550_o054Reloc[1].frame, updateRate);
    hudY = (s32) sOverlay54Height;
    viGetCurrentSize_o054Reloc((s32 *) &width, (s32 *) &height);
    sOverlay54Tail2B0[0]++;
    sOverlay54Tail2B0[0] %= 10;

    for (playerIndex = 0; playerIndex < D_8007BEF4_o054Reloc; playerIndex++) {

        actor = actors[playerIndex];
        if (actor == NULL) {
            return;
        }
        player = actor->player;
        position = sOverlay54ListCopyA[playerIndex];
        lap = sOverlay54ListCopyC[playerIndex];
        lapCount = sOverlay54ListCopyD[playerIndex];
        timer = sOverlay54ListCopyE[playerIndex];
        displayMode = &sOverlay54Tail298[playerIndex];
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
        lap[2].metadata = (s32) (sOverlay54Scale * 65536.0f);
        overlay54GetOffsets(playerIndex, 1, &xOffset, &yOffset);
        if ((lap[0].metadata >> 16) == 1) {
            lap[0].x = sOverlay54ListC[0].x + xOffset + 1;
        } else {
            lap[0].x = sOverlay54ListC[0].x + xOffset;
        }
        if ((lap[1].metadata >> 16) == 1) {
            lap[1].x = sOverlay54ListC[1].x + xOffset - 1;
        } else {
            lap[1].x = sOverlay54ListC[1].x + xOffset;
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
            centiseconds += sOverlay54Tail2B0[0];
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
                    timer[i].x = sOverlay54ListE[i].x + xOffset + 1;
                } else {
                    timer[i].x = sOverlay54ListE[i].x + xOffset - 1;
                }
            } else {
                timer[i].x = sOverlay54ListE[i].x + xOffset;
            }
        }
        if (player->flags1A8 & 8) {
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc,
                sOverlay54ListCopyB[playerIndex], 0, hudY, 255, 255, 255, 255);
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
                sOverlay54ListCopyF[playerIndex], 0, hudY, 255, 255, 255, 255);
            break;
        }
        if (player->unk19A != 255) {
            sOverlay54Tail2B4[playerIndex] += updateRate * 16;
            if (sOverlay54Tail2B4[playerIndex] >= 165) {
                sOverlay54Tail2B4[playerIndex] = 164;
            }
        } else {
            sOverlay54Tail2B4[playerIndex] -= updateRate * 8;
            if (sOverlay54Tail2B4[playerIndex] < 0) {
                sOverlay54Tail2B4[playerIndex] = 0;
            }
        }
        if (sOverlay54Tail2B4[playerIndex] > 0) {
            if (player->unk19C != 0) {
                sOverlay54Flags[playerIndex] = 53;
            } else if (player->unk19A != 255) {
                sOverlay54Flags[playerIndex] = D_8007C180_o054Reloc[player->unk19A];
            }
            if (sOverlay54Flags[playerIndex] != -1) {
                if (frontGetScreenMode_o054Reloc() == 1) {
                    x = (playerIndex & 1) ? 276 : 25;
                    y = (playerIndex & 2) ? 209 : 89;
                } else {
                    x = (playerIndex & 1) ? 276 : 25;
                    y = (playerIndex & 2) ? 197 : 89;
                }
                if (sOverlay54Flags[playerIndex] == 53) {
                    x -= 7;
                    y -= 6;
                }
                {
                    icon[0].link = (s32) D_800D31C8_o054Reloc[sOverlay54Flags[playerIndex]];
                    icon[0].value = 0;
                    icon[0].metadata = 0;
                    icon[0].x = 0;
                    icon[0].y = 0;
                    icon[1].link = 0;
                    func_8002FB34_o054Reloc(&D_800D3140_o054Reloc,
                        icon, (f32) x, (f32) y, 0.66f, 0.66f, sOverlay54Tail2B4[playerIndex] | ~255, 1);
                }
                if (sOverlay54Flags[playerIndex] != 53 && player->unk19B >= 2) {
                    sOverlay54ListG[0].metadata = player->unk19B << 16;
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, sOverlay54ListG,
                        x + 18, y + 18, 0, 0, 0, sOverlay54Tail2B4[playerIndex]);
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, sOverlay54ListG,
                        x + 20, y + 20, 0, 0, 0, sOverlay54Tail2B4[playerIndex]);
                    func_8002F618_o054Reloc(&D_800D3140_o054Reloc, sOverlay54ListG,
                        x + 19, y + 19, 255, 255, 255, 255);
                }
            }
        } else {
            sOverlay54Flags[playerIndex] = -1;
        }
        x = playerIndex & 1;
        enterX = sOverlay54EnterX[x];
        hiddenX = sOverlay54HiddenX[x];
        leaveX = sOverlay54LeaveX[x];
        if (player->unk388 != 0) {
            for (i = 0; i < updateRate; i++) {
                sOverlay54Bounds[playerIndex] += (enterX - sOverlay54Bounds[playerIndex]) >> 3;
            }
            visible = 1;
        } else if (hiddenX == sOverlay54Bounds[playerIndex]) {
            visible = 0;
        } else {
            for (i = 0; i < updateRate; i++) {
                sOverlay54Bounds[playerIndex] += (leaveX - sOverlay54Bounds[playerIndex]) >> 3;
            }
            if ((leaveX >> 6) == (sOverlay54Bounds[playerIndex] >> 6)) {
                sOverlay54Bounds[playerIndex] = hiddenX;
                visible = 0;
            } else {
                visible = 1;
            }
        }
        if (visible) {
            sOverlay54State[0].x = sOverlay54Bounds[playerIndex] >> 4;
            if (frontGetScreenMode_o054Reloc() == 1) {
                if (playerIndex < 2) {
                    sOverlay54State[0].y = 40;
                } else {
                    sOverlay54State[0].y = 160;
                }
            } else {
                if (playerIndex < 2) {
                    sOverlay54State[0].y = 45;
                } else {
                    sOverlay54State[0].y = 153;
                }
            }
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, sOverlay54State,
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
                    sOverlay54Values[player->playerIndex] +=
                        (x - sOverlay54Values[player->playerIndex] + 816) >> 3;
                    sOverlay54Sentinels[player->playerIndex] +=
                        (screenY - sOverlay54Sentinels[player->playerIndex] + 560) >> 3;
                }
            } else {
                resetX = x - 1280;
                if (resetX != sOverlay54Values[player->playerIndex]) {
                    if (player->unk456 != -1 && O54_TIME_DELTA(player) < 0) {
                        amSndPlay_o054Reloc(505, NULL);
                        player->unk456 = -1;
                    }
                    for (i = 0; i < updateRate; i++) {
                        sOverlay54Values[player->playerIndex] +=
                            (width * 8 + x - sOverlay54Values[player->playerIndex] + 816) >> 3;
                        sOverlay54Sentinels[player->playerIndex] +=
                            (screenY - sOverlay54Sentinels[player->playerIndex] - 400) >> 3;
                    }
                    if (x + width * 8 + 656 < (u32) sOverlay54Values[player->playerIndex]) {
                        sOverlay54Values[player->playerIndex] = resetX;
                        sOverlay54Sentinels[player->playerIndex] = screenY - 320;
                    }
                }
            }
            if (O54_TIME_DELTA(player) <= 0) {
                sOverlay54Records[0].metadata = 12 << 16;
                deltaTime = -O54_TIME_DELTA(player);
                for (i = 0; i < 9; i++) {
                    sOverlay54Records[i].link = (s32) D_800D31C8_o054Reloc[20];
                    sOverlay54Records[i].value = (s32) D_800D31C8_o054Reloc[21];
                }
            } else {
                sOverlay54Records[0].metadata = 13 << 16;
                deltaTime = O54_TIME_DELTA(player);
                for (i = 0; i < 9; i++) {
                    sOverlay54Records[i].link = (s32) D_800D31C8_o054Reloc[80];
                    sOverlay54Records[i].value = (s32) D_800D31C8_o054Reloc[21];
                }
            }
            overlay56SplitTime_o054Reloc(deltaTime, &minutes, &seconds, &centiseconds);
            sOverlay54Records[1].metadata = (minutes / 10) * 65536;
            sOverlay54Records[2].metadata = (minutes % 10) * 65536;
            sOverlay54Records[4].metadata = (seconds / 10) * 65536;
            sOverlay54Records[5].metadata = (seconds % 10) * 65536;
            sOverlay54Records[7].metadata = (centiseconds / 10) * 65536;
            sOverlay54Records[8].metadata = (centiseconds % 10) * 65536;
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
            func_8002F618_o054Reloc(&D_800D3140_o054Reloc, sOverlay54Records,
                sOverlay54Values[player->playerIndex] >> 4,
                sOverlay54Sentinels[player->playerIndex] >> 4, 255, 255, 255, 255);
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
                if (o001_data_83E0_o054Reloc == 0 && buttons != 0 && sOverlay54Tail2AC == 0) {
                    mainChangeCameras_o054Reloc(1);
                    func_800016EC_o054Reloc(1);
                    func_8003A590_o054Reloc();
                    func_80037414_o054Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o054Reloc(18, 0, 0, 7, 1, 1);
                    amTuneSetFade_o054Reloc(3.0f, 0);
                    sOverlay54Tail2AC = 1;
                }
                break;
            case 4:
                if (o001_data_83E0_o054Reloc == 0 && buttons != 0 && sOverlay54Tail2AC == 0) {
                    mainChangeCameras_o054Reloc(1);
                    func_800016EC_o054Reloc(1);
                    func_8003A590_o054Reloc();
                    func_80037414_o054Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
                    mainChangeLevel_o054Reloc(18, 0, 0, 7, 1, 1);
                    amTuneSetFade_o054Reloc(3.0f, 0);
                    sOverlay54Tail2AC = 1;
                }
                break;
            }
        }
    }
    camSetNo_o054Reloc(0);
}
