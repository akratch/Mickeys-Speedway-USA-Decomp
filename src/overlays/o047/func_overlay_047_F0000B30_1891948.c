/* NON_MATCHING: typed structural reconstruction from Mickey-only call,
 * data and field evidence. The canonical assembly remains the authority. */
#ifdef NON_MATCHING
#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "game/math.h"
#include "game/models.h"
#include "overlays/overlay_045.h"

typedef struct Overlay47TextureScroll {
    u8 pad00[8];
    s16 a, b, c, d;
} Overlay47TextureScroll;

/* Field widths shared with overlay47SpawnObject and the resident animation
 * and object consumers; names beyond those consumers remain structural. */
typedef struct Overlay47Actor {
    s16 rotation;
    u8 pad02[0xA];
    f32 x, y, z;
    u8 pad18[0x10];
    f32 frameValue;
    u8 pad2C[0xF];
    u8 frame;
    u8 pad3C[0xA];
    s16 kind;
    u8 pad48[0x1C];
    Overlay47TextureScroll *scroll;
    void *animationStates;
    u8 pad6C[0x14];
    s32 trigger;
} Overlay47Actor;

typedef struct Overlay47Player {
    f32 x, y, z;
    f32 targetX, targetY, targetZ;
    f32 screenX, screenY;
    s16 rotation, targetRotation;
    Overlay47Actor *actor;
    s16 selector;
    s8 ready, active, leaving;
    u8 pad2D;
    s16 idleTimer;
    void *sound;
} Overlay47Player;

typedef struct Overlay47Position {
    f32 x, y, z;
    s16 rotation;
    u8 pad0E[2];
} Overlay47Position;

typedef struct Overlay47Icon {
    s16 rotationX, rotationY, rotationZ;
    u16 flags;
    f32 scale, x, y, z;
    u8 pad18[0x10];
    f32 selector;
} Overlay47Icon;

typedef struct Overlay47TextureNode {
    void *texture;
    void *alternate;
    u32 packedOffset;
    s16 x, y;
} Overlay47TextureNode;

typedef struct Overlay47Command {
    u32 w0, w1;
} Overlay47Command;

extern Overlay47Player D_800D3058[4];
extern s8 D_800D3190[4];
extern s32 D_8007C1A0;
extern u8 D_8007BF74;
extern char **D_8007C0B8;
extern Overlay47Command *D_800D3140;
extern Mtx *D_800D3144;
extern void *D_800D3148;
extern void *D_800D31C8[];
/* Overlay 47's .data and .bss as TU statics at their recorded offsets, in the
 * layout func_overlay_047_F0000000_1890E18 defines (Tier D, from the LOCAL
 * relocation addends). Only the NON_MATCHING body sees them; a promotion must
 * drop these sections at POSTPROCESS as that TU's rule does. The blend value
 * and its speed (+0x540/+0x544) are two statics: the target forms each address
 * in its own register at the tail (2026-10-08). The colour table is s32 and the
 * colour blocks read it per channel with no `colour` local, so the packet's
 * cursor store blocks uopt's forward and the channels are symbol webs as
 * shipped; the blend reassigns the channels (lane j-9, 2026-10-08). */
static u8 ov47Data_0[0x8C] = { 0 };
static u8 ov47Data_8C[0x8C] = { 0 };
static u8 ov47Data_118[0x80] = { 0 };
static u8 ov47Data_198[0x28] = { 0 };
static u8 ov47Data_1C0[0x68] = { 0 };
static u8 ov47Data_228[0x40] = { 0 };
static u8 ov47Data_268[0x40] = { 0 };
static Overlay47Command ov47Data_2A8[11] = { 0 };
static Overlay47Command ov47Data_300[11] = { 0 };
static s16 D_358[12] = { 0 };
static s16 D_370[2] = { 0 };
static s16 D_374[12] = { 0 };
static void *D_38C[10] = { 0 };
static u8 sO47Data3B4[0x14] = { 0 };
/* Both arms read the 3CC table at row (count - 1): the ready arm's held
 * base and -4 displacement and the unready arm's folded 0x3C8 address are
 * the same expression (lane n-1, 2026-10-08). */
static s8 ov47Data_3C8[1][4] = { 0 };
static s8 ov47Data_3CC[4][4] = { 0 };
static s32 ov47Data_3DC[5] = { 0 };
static f32 ov47Data_3F0[5] = { 0 };
static f32 D_404[15] = { 0 };
static s8 D_440[12] = { 0 };
static s8 D_44C[12] = { 0 };
static s8 D_458[12] = { 0 };
static s8 D_464[12] = { 0 };
static u8 sO47Data470[4] = { 0 };
static s8 ov47Data_474[12] = { 0 };
static s8 ov47Data_480[12] = { 0 };
static u16 ov47Data_48C[10] = { 0 };
static u16 ov47Data_4A0[10] = { 0 };
static u16 ov47Data_4B4[10] = { 0 };
static s8 ov47Data_4C8[10][4] = { 0 };
static Overlay47TextureNode ov47Data_4F0 = { 0 };
static u8 sO47Data500[0x10] = { 0 };
static s16 ov47Data_510[10] = { 0 };
static s8 ov47Data_524[12] = { 0 };
static s32 ov47Data_530[4] = { 0 };
static f32 ov47Data_540 = 0;
static f32 ov47Data_544 = 0;
static s32 ov47Data_548 = 0;
static f32 ov47Data_54C = 0;
static s32 ov47Data_550 = 0;
static s32 ov47Data_554 = 0;
static u8 sO47Data558[8] = { 0 };

static s8 ov47Bss_0;
static Overlay47Icon ov47Bss_8[10];
static Overlay47TextureNode ov47Bss_1C0[5];
static Overlay47Position ov47Bss_210[7];
static Overlay47Position ov47Bss_280[7];
static Overlay47Position ov47Bss_2F0;
static s8 ov47Bss_300[10];
static s8 ov47Bss_30A;
static s8 ov47Bss_30B;
static void *ov47Bss_30C;
static void *ov47Bss_310;
static Overlay45ResourceDescriptor *ov47Bss_314;
static Overlay45ResourceDescriptor *ov47Bss_318;
static Overlay45ResourceDescriptor *ov47Bss_31C;
static Overlay45ResourceDescriptor *ov47Bss_320;
static s32 ov47Bss_324;
static s32 ov47Bss_328[4];
static s32 ov47Bss_338;
static s32 ov47Bss_33C;

extern u16 joyGetPressed(s32 player);
extern void amSndPlay(u16 soundId, void **handle);
extern void amSndStop(void *handle);
extern void func_80006EA0(void *object);
extern void func_8005AD64(void *object, s32 frame, s32 index, f32 value);
extern s32 func_8005ABA8(void *object, f32 speed, f32 updateRate);
extern s32 func_800246B0(f32 x, f32 y, f32 z, f32 *screenX, f32 *screenY, u8 mode);
extern s32 func_8001398C(f32 x, f32 z, s32 flags, f32 ***height);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern f32 camGetFOV(void);
extern void func_80021504(f32 fov, s32 force);
extern void func_800221E8(Overlay47Command **commands, Mtx **matrices);
extern void camStandardOrtho(Overlay47Command **commands, Mtx **matrices);
extern void func_800349A4(Overlay47Command **commands, void *texture, s32 flags, s32 offset);
extern void func_80034920(Overlay47Command **commands);
extern void func_8002FB34(Overlay47Command **commands, Overlay47TextureNode *textures,
                        f32 x, f32 y, f32 scaleX, f32 scaleY, s32 mode, s32 flags);
extern void func_8002A82C(MtxF matrix);
extern void matrixTranslate(f32 x, f32 y, f32 z, MtxF matrix);
extern void matrixScale(f32 x, f32 y, f32 z, MtxF matrix);
extern void func_8002A604(s16 rotation, MtxF matrix);
extern void func_80024978(MtxF matrix);
extern void mtxf_mul(MtxF lhs, MtxF rhs, MtxF dest);
extern void mtxf_to_mtx(MtxF source, Mtx *dest);
extern void func_80023F84(Overlay47Command **commands, Mtx **matrices, void **vertices,
                        void *transform, void *sprite, s32 flags, u8 alpha);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 blend);
extern void func_8004B0A4(s32 font);
extern void func_8004B0F8(Overlay47Command **commands, s32 x, s32 y, char *text, s32 flags);
extern void func_800367A4(void *texture, s32 *state, s32 speed, f32 *frame, s32 updateRate);
extern void mainChangeLevel(s32 level, s32 character, s32 animGroup, s32 mode, s32 arg4, s32 arg5);
extern void func_overlay_047_F0002D10_1893B28(Overlay47Player *player);

/* Same command fields used by the existing frontend and RCP TUs. */
#define O47_COMMAND(a, b) { \
    Overlay47Command *command = D_800D3140++; \
    command->w0 = (u32)(a); \
    command->w1 = (u32)(b); \
}
/* The same command with its second word written first: per command site,
 * the order fixes which word's operand ugen materialises first (measured
 * 2026-10-07, six sites). */
#define O47_COMMAND_W1(a, b) { \
    Overlay47Command *command = D_800D3140++; \
    command->w1 = (u32)(b); \
    command->w0 = (u32)(a); \
}
#define O47_PHYSICAL(p) ((u32)((u8 *)(p) + 0x80000000))
#define O47_VERTICES(p, n) \
    O47_COMMAND(0x04000000 | (((((n) << 3) | (O47_PHYSICAL(p) & 6)) & 0xFF) << 16) | ((n) * 10 + 8), O47_PHYSICAL(p))
#define O47_RECTANGLE(x, y) \
    O47_COMMAND(0xF6000000 | ((((x) + 6) & 0x3FF) << 14) | ((((y) + 6) & 0x3FF) << 2), \
                (((x) & 0x3FF) << 14) | (((y) & 0x3FF) << 2))

void func_overlay_047_F0000B30_1891948(s32 updateRate) {
    s32 j;
    s32 controller;
    s32 i;
    s32 slot;
    s32 selected;
    s32 unready;
    s32 colourIndex;
    s32 activeCount;
    s32 speed;
    s32 allReady;
    s32 count;
    s32 start;
    s32 rotationStep;
    s32 labelCount;

    s32 barX;
    s32 stat;

    s32 x, y;
    s32 red, green, blue;
    s32 showMode;
    f32 movement;
    s32 back;
    f32 oldFrame;
    f32 frame;
    Overlay47Player *p2;
    Overlay47Player *p3; /* unreferenced; its frame cell is measured */
    f32 scale;
    f32 rate;
    f32 oldFov;
    MtxF localMatrix;
    MtxF cameraMatrix;
    MtxF resultMatrix;
    Mtx *savedMatrix;
    f32 oldSelector;
    f32 **height;
    Overlay47Player *player;
    Overlay47TextureScroll *scroll;
    Overlay47Icon *icon;
    s32 pad;

    rate = updateRate;
    ov47Bss_30B = 0;
    showMode = 0;
    allReady = 1;
    start = 0;
    activeCount = 0;
    back = 0;
    /* Both player-array loops outside the main loop are indexed by
     * controller: the target's first-loop and label-loop cursors are one
     * strength-reduced s8 cursor with their own address pairs (measured
     * 2026-10-07). */
    for (controller = 0; controller < 4; controller++) {
        if (D_800D3058[controller].active != 0) {
            activeCount++;
            if (D_800D3058[controller].ready == 0) {
                allReady = 0;
                ov47Bss_338 = 0;
            }
        }
    }
    slot = 0;
    if (allReady && ((activeCount == 1) || (activeCount == 4))) {
        ov47Bss_338 = 1;
    }
    player = D_800D3058;
    icon = ov47Bss_8;
    for (controller = 0; controller < 4; controller++, player++) {
        switch (player->active) {
            case 0:
                if ((joyGetPressed(controller) & 0x9000) && !player->leaving &&
                    !ov47Bss_324 && !start) {
                    player->active = 1;
                    player->x = ov47Bss_2F0.x;
                    player->y = ov47Bss_2F0.y;
                    player->z = ov47Bss_2F0.z;
                    player->rotation = ov47Bss_2F0.rotation;
                    i = 0;
                    while (ov47Bss_300[player->selector]) {
                        player->selector++;
                        if (player->selector >= 10) player->selector = 0;
                    }
                    allReady = 0;
                    ov47Bss_300[player->selector] = 1;
                    D_8007C1A0++;
                    ov47Bss_30A++;
                    amSndPlay(12, NULL);
                    amSndPlay(25, NULL);
                    if (player->actor != NULL) {
                        if (player->actor->kind != ov47Data_510[player->selector]) {
                            func_80006EA0(player->actor);
                            func_overlay_047_F0002D10_1893B28(player);
                        } else {
                            func_8005AD64(player->actor, 1, -1, 0.0f);
                        }
                    }
                    if (ov47Bss_0 > 0) {
                        j = i;
                        do {
                            if ((f32)player->selector == icon->selector) {
                                ov47Bss_328[controller] = ((s32)icon->x + 160) << 4;
                            }
                            icon++;
                        } while (++j < ov47Bss_0);
                        icon = ov47Bss_8;
                    }
                }
                break;
            default:
                if (allReady && !ov47Bss_338) {
                    showMode = 1;
                    if (D_800D3190[controller] < -16) {
                        if (D_8007BF74 != 0) {
                            amSndPlay(14, NULL);
                        } else {
                            ov47Bss_30B = 1;
                            D_8007BF74 = 1;
                        }
                    }
                    if (D_800D3190[controller] >= 17) {
                        if (D_8007BF74 == 0) {
                            amSndPlay(14, NULL);
                        } else {
                            ov47Bss_30B = 1;
                            D_8007BF74 = 0;
                        }
                    }
                }
                if ((joyGetPressed(controller) & 0x9000) && !player->leaving &&
                    player->actor != NULL && !ov47Bss_324) {
                    if (allReady) {
                        if (!ov47Bss_338) {
                            ov47Bss_338 = 1;
                            amSndPlay(12, NULL);
                        } else {
                            start = 1;
                            if (player->sound != NULL) amSndStop(player->sound);
                            amSndPlay(ov47Data_4B4[ov47Data_524[player->selector]], &player->sound);
                        }
                    } else if (!player->ready) {
                        player->ready = 1;
                        if (player->sound != NULL) amSndStop(player->sound);
                        amSndPlay(ov47Data_48C[ov47Data_524[player->selector]], &player->sound);
                    }
                    func_8005AD64(player->actor, 2, -1, 0.0f);
                } else if ((joyGetPressed(controller) & 0x4000) && !player->leaving) {
                    if (player->actor != NULL && !ov47Bss_324 && !start) {
                        if (player->ready) {
                            player->ready = 0;
                            allReady = 0;
                            func_8005AD64(player->actor, 0, -1, 0.0f);
                            if (player->sound != NULL) amSndStop(player->sound);
                            amSndPlay(ov47Data_4A0[ov47Data_524[player->selector]], &player->sound);
                        } else if (D_8007C1A0 >= 2) {
                            player->active = 0;
                            ov47Bss_300[player->selector] = 0;
                            if (player->actor != NULL) {
                                player->leaving = 1;
                                func_8005AD64(player->actor, 5, -1, 0.0f);
                                amSndPlay(24, NULL);
                            }
                            D_8007C1A0--;
                        } else {
                            back = 1;
                        }
                    }
                }
                break;
        }
        if (player->active && player->actor == NULL) {
            func_overlay_047_F0002D10_1893B28(player);
        }
        if (player->actor != NULL) {
            speed = updateRate * 10000;
            func_800246B0(player->actor->x, player->actor->y, player->actor->z,
                         &player->screenX, &player->screenY, 1);
            if (!player->leaving && !player->active) {
                movement = 0.015f;
                player->targetX = ov47Bss_2F0.x;
                player->targetY = ov47Bss_2F0.y;
                player->targetZ = ov47Bss_2F0.z;
                player->targetRotation = ov47Bss_2F0.rotation;
            } else {
                movement = 0.125f;
                if (player->ready) {
                    player->targetX = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].x;
                    player->targetY = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].y;
                    player->targetZ = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].z;
                    player->targetRotation = ov47Bss_280[ov47Data_3CC[ov47Bss_30A - 1][slot]].rotation;
                    slot++;
                } else {
                    player->targetX = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].x;
                    player->targetY = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].y;
                    player->targetZ = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].z;
                    player->targetRotation = ov47Bss_210[ov47Data_3CC[ov47Bss_30A - 1][slot]].rotation;
                    slot++;
                }
            }
            for (i = 0; i < updateRate; i++) {
                player->x += (player->targetX - player->x) * movement;
                player->y += (player->targetY - player->y) * movement;
                player->z += (player->targetZ - player->z) * movement;
                player->rotation += (player->targetRotation - player->rotation) * movement;
                player->actor->x = player->x;
                player->actor->y = player->y;
                player->actor->z = player->z;
                player->actor->rotation = player->rotation;
            }
            if (func_8001398C(player->x, player->z, 0x1000, &height)) {
                player->y = **height;
            }
            oldFrame = player->actor->frameValue;
            func_8005ABA8(player->actor, ov47Data_3F0[(s8)player->actor->frame], rate);
            scroll = player->actor->scroll;
            for (i = 0; i < 4; i++) {
                (&scroll->a)[i] += speed;
            }
            if (!player->ready && !player->leaving && player->active && !start) {
                if (D_800D3190[controller] < -16 && !ov47Bss_324) {
                    ov47Bss_300[player->selector] = 0;
                    player->selector--;
                    if (player->selector < 0) player->selector = 9;
                    while (ov47Bss_300[player->selector]) {
                        player->selector--;
                        if (player->selector < 0) player->selector = 9;
                    }
                    ov47Bss_300[player->selector] = 1;
                    ov47Bss_30B = 1;
                    func_80006EA0(player->actor);
                    player->actor = NULL;
                    func_overlay_047_F0002D10_1893B28(player);
                } else if (D_800D3190[controller] >= 17 && !ov47Bss_324) {
                    ov47Bss_300[player->selector] = 0;
                    player->selector++;
                    if (player->selector >= 10) player->selector = 0;
                    while (ov47Bss_300[player->selector]) {
                        player->selector++;
                        if (player->selector >= 10) player->selector = 0;
                    }
                    ov47Bss_300[player->selector] = 1;
                    ov47Bss_30B = 1;
                    func_80006EA0(player->actor);
                    player->actor = NULL;
                    func_overlay_047_F0002D10_1893B28(player);
                }
            }
            if (player->actor != NULL) {
                player->actor->trigger = 0;
                switch (player->actor->frame) {
                    case 1:
                        if (0.3f <= player->actor->frameValue && player->actor->frameValue < 0.65f) {
                            player->actor->trigger = 15;
                        }
                        if (oldFrame < 0.3f && 0.3f <= player->actor->frameValue) {
                            amSndPlay(26, NULL);
                        }
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                    case 3:
                    case 4:
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, 0, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                    case 0:
                        if (player->idleTimer > 0) {
                            player->idleTimer -= updateRate;
                        } else if (player->actor->frameValue < 0.02f) {
                            func_8005AD64(player->actor, mathRnd(3, 4), -1, 0.0f);
                        }
                        break;
                    case 5:
                        player->actor->trigger = 15;
                        if (0.8f < player->actor->frameValue && player->leaving) {
                            player->leaving = 0;
                            slot--;
                            ov47Bss_30A--;
                        }
                        break;
                    case 2:
                        if (player->actor->frameValue == 1.0f) {
                            func_8005AD64(player->actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                        }
                        break;
                }
                if (!player->leaving && !player->active) {
                    if ((player->x - ov47Bss_2F0.x) * (player->x - ov47Bss_2F0.x) +
                        (player->z - ov47Bss_2F0.z) * (player->z - ov47Bss_2F0.z) < 1000.0f) {
                        func_80006EA0(player->actor);
                        player->actor = NULL;
                    }
                }
                if (player->actor != NULL) partUpdateTriggers(player->actor, updateRate);
            }
        }
    }
    ov47Data_550 += ov47Data_554 * updateRate;
    if (ov47Data_550 < 0) {
        ov47Data_550 = -ov47Data_550;
        ov47Data_554 = -ov47Data_554;
    } else if (ov47Data_550 >= 256) {
        ov47Data_550 = 510 - ov47Data_550;
        ov47Data_554 = -ov47Data_554;
    }
    if (showMode) {
        overlay45ConfigureLayout(ov47Bss_318, 160, 160, 0x104);
        overlay45ConfigureLayout(ov47Bss_31C, 120, 190, 0x104);
        overlay45ConfigureLayout(ov47Bss_320, 200, 190, 0x104);
        if (D_8007BF74) {
            overlay45SetField22(ov47Bss_31C, ov47Data_550);
            overlay45SetField22(ov47Bss_320, 0);
        } else {
            overlay45SetField22(ov47Bss_31C, 0);
            overlay45SetField22(ov47Bss_320, ov47Data_550);
        }
    } else {
        overlay45ConfigureLayout(ov47Bss_318, 160, -40, 0x104);
        overlay45ConfigureLayout(ov47Bss_31C, 120, 260, 0x104);
        overlay45ConfigureLayout(ov47Bss_320, 200, 260, 0x104);
    }
    oldFov = camGetFOV();
    func_80021504(52.0f, 1);
    func_800221E8(&D_800D3140, NULL);
    camStandardOrtho(&D_800D3140, &D_800D3144);
    func_800349A4(&D_800D3140, NULL, 16, 0);
    O47_COMMAND(0xFA000000, -1);
    O47_VERTICES(ov47Data_228, 6);
    O47_COMMAND(0x05300040, O47_PHYSICAL(ov47Data_268));
    O47_COMMAND(0xE7000000, 0);
    if (allReady && ov47Bss_338) {
        overlay45ConfigureLayout(ov47Bss_314, 160, 178, 0x104);
    } else {
        overlay45ConfigureLayout(ov47Bss_314, 160, 272, 0x104);
    }
    for (controller = 0; controller < ov47Bss_0; controller++, icon++) {
        colourIndex = 4;
        unready = 0;
        selected = -1;
        count = 0;
        p2 = D_800D3058;
        for (stat = 0; stat < 4; stat++, p2++) {
            if ((f32)p2->selector == icon->selector && p2->active) {
                colourIndex = stat;
                if (!p2->ready) unready = 1;
                for (selected = 0; selected < updateRate; selected++) {
                    ov47Bss_328[stat] +=
                        ((((s32)icon->x + 160) << 4) - ov47Bss_328[stat]) >> 2;
                }
                ov47Bss_1C0[count].texture = D_800D31C8[stat + 13];
                ov47Bss_1C0[count].x = 312 - (ov47Bss_328[stat] >> 4);
                count++;
                selected = stat;
            }
        }
        ov47Bss_1C0[count].texture = NULL;
        func_8002FB34(&D_800D3140, ov47Bss_1C0, 320.0f, 0, 1.0f, 1.0f, -2, 0x1003);
        func_8002A82C(localMatrix);
        matrixTranslate(icon->x, icon->y, 0.0f, localMatrix);
        scale = icon->scale / 1.16f;
        matrixScale(scale, scale, scale, localMatrix);
        func_8002A604(icon->rotationZ, localMatrix);
        func_80024978(cameraMatrix);
        mtxf_mul(localMatrix, cameraMatrix, resultMatrix);
        mtxf_to_mtx(resultMatrix, D_800D3144);
        savedMatrix = D_800D3144;
        /* Word 0 first: the w0 store separates the savedMatrix load from the
         * operand's reload, so unready keeps its register (lane k-8). */
        O47_COMMAND(0x01000040, O47_PHYSICAL(D_800D3144));
        D_800D3144++;
        red = ov47Data_3DC[colourIndex] >> 24;
        green = ov47Data_3DC[colourIndex] >> 16;
        blue = ov47Data_3DC[colourIndex] >> 8;
        O47_COMMAND(0x06000000, ov47Data_300);
        O47_COMMAND(0xFA000000, ((red & 255) << 24) | ((green & 255) << 16) | ((blue & 255) << 8) | 255);
        O47_COMMAND_W1(0xFCFFFFFF, 0xFFFDF6FB);
        O47_VERTICES(ov47Data_198, 4);
        O47_COMMAND_W1(0x05100020, O47_PHYSICAL(ov47Data_1C0));
        camStandardOrtho(&D_800D3140, &D_800D3144);
        func_80034920(&D_800D3140);
        O47_COMMAND_W1(0xFA000000, -1);
        func_80023F84(&D_800D3140, &D_800D3144, &D_800D3148, icon, D_800D31C8[11], 0, 255);
        if (selected != -1 && !unready) {
            oldSelector = icon->selector;
            icon->selector = selected;
            func_80023F84(&D_800D3140, &D_800D3144, &D_800D3148, icon, D_800D31C8[124], 0, 255);
            icon->selector = oldSelector;
        }
        O47_COMMAND(0x01000040, O47_PHYSICAL(savedMatrix));
        if (selected != -1 && !D_800D3058[selected].ready) {
            O47_COMMAND(0x06000000, ov47Data_2A8);
            red = ov47Data_3DC[selected] >> 24;
            green = (ov47Data_3DC[selected] >> 16) & 255;
            blue = ov47Data_3DC[selected] >> 8;
            /* Red and blue masked in place, green masked at the load: the
             * masks are ring temporaries and green keeps the shipped copy. */
            red &= 255;
            red = red + (255 - red) * ov47Data_540;
            green = green + (255 - green) * ov47Data_540;
            blue &= 255;
            blue = blue + (255 - blue) * ov47Data_540;
            O47_COMMAND(0xFA000000, ((red & 255) << 24) | ((green & 255) << 16) | ((blue & 255) << 8) | 255);
            O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
            O47_VERTICES(ov47Data_0, 14);
            O47_COMMAND(0x05710080, O47_PHYSICAL(ov47Data_118));
            O47_VERTICES(ov47Data_8C, 14);
            O47_COMMAND(0x05710080, O47_PHYSICAL(ov47Data_118));
        }
        for (stat = 0; stat < updateRate; stat++) {
            if (icon->rotationZ < 0) {
                rotationStep = (icon->rotationZ + 0x1000) / 5;
            } else {
                rotationStep = (0x1000 - icon->rotationZ) / 5;
            }
            if (rotationStep < 51) rotationStep = 50;
            if (!ov47Data_474[controller]) {
                icon->rotationZ += rotationStep;
                if (selected == -1 && icon->rotationZ >= 0 &&
                    icon->rotationZ - rotationStep <= 0) {
                    icon->rotationZ = 0;
                }
                if (icon->rotationZ > 0x1000) {
                    icon->rotationZ = 0x2000 - icon->rotationZ;
                    ov47Data_474[controller] = 1;
                }
            } else {
                if (icon->rotationZ < 0) {
                    ov47Data_480[controller]++;
                    if (ov47Data_480[controller] >= 101) ov47Data_480[controller] = 100;
                } else {
                    ov47Data_480[controller]--;
                    if (ov47Data_480[controller] < 0) ov47Data_480[controller] = 0;
                }
                icon->rotationZ -= rotationStep;
                if (selected == -1 && icon->rotationZ <= 0 &&
                    icon->rotationZ + rotationStep >= 0) {
                    icon->rotationZ = 0;
                }
                if (icon->rotationZ < -0x1000) {
                    icon->rotationZ = -0x2000 - icon->rotationZ;
                    ov47Data_474[controller] = 0;
                }
            }
        }
    }
    fontColour(255, 255, 255, 255, 255);
    func_8004B0A4(2);
    labelCount = 0;
    for (controller = 0; controller != 4; controller++) {
        if (D_800D3058[controller].active && D_800D3058[controller].actor != NULL) {
            /* The label x is `unready` again: one symbol for the ready flag
             * and the label column, so it keeps unready's s8 (lever_sweep
             * merge_locals, lane p-4: 615 at -4 to 455 at size 0). */
            if (ov47Bss_30A == 4) {
                unready = ov47Data_530[controller] + D_800D3058[controller].screenX;
            } else {
                unready = D_800D3058[controller].screenX - 20.0f;
            }
            /* The row counter is `selected` and the bar row is `stat`: one
             * symbol each across the icon and label loops, which puts both
             * webs across calls and gives the target's s1 and s4 (lane k-8). */
            stat = 116;
            for (selected = 0; selected != 4; selected++) {
                barX = unready;
                if (labelCount < 4) {
                    if (ov47Bss_30A != 4) {
                        func_8004B0F8(&D_800D3140, unready - 6, stat + 2, D_8007C0B8[145 + selected], 9);
                    } else {
                        func_8004B0F8(&D_800D3140, 160, stat + 2, D_8007C0B8[145 + selected], 12);
                    }
                    labelCount++;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xEF002C0F, 0x00504340);
                O47_COMMAND(0xB6000000, 0x00010001);
                O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
                count = ov47Data_4C8[ov47Data_524[D_800D3058[controller].selector]][selected];
                O47_COMMAND(0xFA000000, ov47Data_3DC[controller]);
                while (count--) {
                    O47_RECTANGLE(barX, stat);
                    barX += 8;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xFA000000, (ov47Data_3DC[controller] & ~0xFF) | 0x40);
                count = 5 - ov47Data_4C8[ov47Data_524[D_800D3058[controller].selector]][selected];
                while (count--) {
                    O47_RECTANGLE(barX, stat);
                    barX += 8;
                }
                stat += 8;
            }
        }
    }
    overlay45ReadPair((Overlay45PairOwner *)ov47Bss_314, &x, &y, 0);
    func_8002FB34(&D_800D3140, &ov47Data_4F0, (f32)(x - 32), (f32)(y - 6), 1.0f, 1.0f, -2, 3);
    func_800367A4(ov47Data_4F0.texture, &ov47Data_548, 2, &ov47Data_54C, updateRate);
    ov47Data_4F0.packedOffset = (s32)(ov47Data_54C * 65536.0f);
    if (start) {
        mainChangeLevel(12, 0, 0, 10, 1, 0);
        ov47Bss_324 = 1;
    } else if (back) {
        amSndPlay(13, NULL);
        mainChangeLevel(12, 0, 0, 4, 1, 0);
        ov47Bss_324 = 1;
    }
    func_80021504(oldFov, 1);
    func_800221E8(&D_800D3140, NULL);
    ov47Data_540 += ov47Data_544 * rate;
    if (ov47Data_540 > 1.0f) {
        ov47Data_544 = -ov47Data_544;
        ov47Data_540 = 2.0f - ov47Data_540;
    } else if (ov47Data_540 < 0.0f) {
        ov47Data_540 = -ov47Data_540;
        ov47Data_544 = -ov47Data_544;
    }
    if (ov47Bss_30B) {
        if (ov47Bss_310 != NULL) amSndStop(ov47Bss_310);
        amSndPlay(15, &ov47Bss_310);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o047/func_overlay_047_F0000B30_1891948/func_overlay_047_F0000B30_1891948.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_047_F0000B30_1891948:start
 * symbol: func_overlay_047_F0000B30_1891948
 * score: 721/2168 words
 * frame: 0x280
 * relocations: 315
 * first-mismatch: +0x4
 * summary: 455 aligned at size 0 (masked 721): the label x written into unready keeps s8; the +4 word is gone. Open: 3CC remainder piece, first colour block.
 * PLATEAU-HANDOFF:func_overlay_047_F0000B30_1891948:end
 */
