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
extern s8 ov47Bss_0;
extern Overlay47Icon ov47Bss_8[];
extern Overlay47TextureNode ov47Bss_1C0[];
extern Overlay47Position ov47Bss_210[], ov47Bss_280[], ov47Bss_2F0;
extern s8 ov47Bss_300[10];
extern s8 ov47Bss_30A, ov47Bss_30B;
extern void *ov47Bss_310;
extern Overlay45ResourceDescriptor *ov47Bss_314, *ov47Bss_318;
extern Overlay45ResourceDescriptor *ov47Bss_31C, *ov47Bss_320;
extern s32 ov47Bss_324, ov47Bss_328[4], ov47Bss_338;
extern u8 ov47Data_0[], ov47Data_8C[], ov47Data_118[];
extern u8 ov47Data_198[], ov47Data_1C0[], ov47Data_228[], ov47Data_268[];
extern Overlay47Command ov47Data_2A8[], ov47Data_300[];
extern s8 ov47Data_3C8[][4];
extern s8 ov47Data_3CC[][4];
extern u32 ov47Data_3DC[5];
extern f32 ov47Data_3F0[6];
extern s8 ov47Data_474[], ov47Data_480[];
extern u16 ov47Data_48C[10], ov47Data_4A0[10], ov47Data_4B4[10];
extern s8 ov47Data_4C8[][4];
extern Overlay47TextureNode ov47Data_4F0;
extern s16 ov47Data_510[10];
extern s8 ov47Data_524[10];
extern s32 ov47Data_530[4];
extern f32 ov47Data_540, ov47Data_544;
extern s32 ov47Data_548;
extern f32 ov47Data_54C;
extern s32 ov47Data_550, ov47Data_554;

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
    s32 textX;
    s32 barX, barY;
    s32 stat;
    s32 colour;
    s32 x, y;
    s32 red, green, blue;
    s32 showMode;
    f32 movement;
    s32 back;
    f32 oldFrame;
    f32 frame;
    Overlay47Player *p2;
    Overlay47Player *p3;
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
    Overlay47Actor *actor;
    Overlay47TextureScroll *scroll;
    Overlay47Icon *icon;

    rate = updateRate;
    ov47Bss_30B = 0;
    showMode = 0;
    allReady = 1;
    start = 0;
    activeCount = 0;
    back = 0;
    p3 = D_800D3058;
    do {
        if (p3->active != 0) {
            activeCount++;
            if (p3->ready == 0) {
                allReady = 0;
                ov47Bss_338 = 0;
            }
        }
        p3++;
    } while (p3 < D_800D3058 + 4);
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
                    actor = player->actor;
                    if (actor != NULL) {
                        if (actor->kind != ov47Data_510[player->selector]) {
                            func_80006EA0(actor);
                            func_overlay_047_F0002D10_1893B28(player);
                        } else {
                            func_8005AD64(actor, 1, -1, 0.0f);
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
                    (actor = player->actor) != NULL && !ov47Bss_324) {
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
                    actor = player->actor;
                    func_8005AD64(actor, 2, -1, 0.0f);
                } else if ((joyGetPressed(controller) & 0x4000) && !player->leaving) {
                    actor = player->actor;
                    if (actor != NULL && !ov47Bss_324 && !start) {
                        if (player->ready) {
                            player->ready = 0;
                            allReady = 0;
                            func_8005AD64(actor, 0, -1, 0.0f);
                            if (player->sound != NULL) amSndStop(player->sound);
                            amSndPlay(ov47Data_4A0[ov47Data_524[player->selector]], &player->sound);
                        } else if (D_8007C1A0 >= 2) {
                            player->active = 0;
                            ov47Bss_300[player->selector] = 0;
                            actor = player->actor;
                            if (actor != NULL) {
                                player->leaving = 1;
                                func_8005AD64(actor, 5, -1, 0.0f);
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
                    player->targetX = ov47Bss_210[ov47Data_3C8[ov47Bss_30A][slot]].x;
                    player->targetY = ov47Bss_210[ov47Data_3C8[ov47Bss_30A][slot]].y;
                    player->targetZ = ov47Bss_210[ov47Data_3C8[ov47Bss_30A][slot]].z;
                    player->targetRotation = ov47Bss_210[ov47Data_3C8[ov47Bss_30A][slot]].rotation;
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
            actor = player->actor;
            oldFrame = actor->frameValue;
            func_8005ABA8(actor, ov47Data_3F0[(s8)actor->frame], rate);
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
            actor = player->actor;
            if (actor != NULL) {
                actor->trigger = 0;
                actor = player->actor;
                switch (actor->frame) {
                    case 1:
                        if (0.3f <= actor->frameValue && actor->frameValue < 0.65f) {
                            actor->trigger = 15;
                            actor = player->actor;
                        }
                        if (oldFrame < 0.3f && 0.3f <= actor->frameValue) {
                            amSndPlay(26, NULL);
                            actor = player->actor;
                        }
                        if (actor->frameValue == 1.0f) {
                            func_8005AD64(actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                            actor = player->actor;
                        }
                        break;
                    case 3:
                    case 4:
                        if (actor->frameValue == 1.0f) {
                            func_8005AD64(actor, 0, 0, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                            actor = player->actor;
                        }
                        break;
                    case 0:
                        if (player->idleTimer > 0) {
                            player->idleTimer -= updateRate;
                        } else if (actor->frameValue < 0.02f) {
                            func_8005AD64(player->actor, mathRnd(3, 4), -1, 0.0f);
                        }
                        actor = player->actor;
                        break;
                    case 5:
                        actor->trigger = 15;
                        actor = player->actor;
                        if (0.8f < actor->frameValue && player->leaving) {
                            player->leaving = 0;
                            slot--;
                            ov47Bss_30A--;
                            actor = player->actor;
                        }
                        break;
                    case 2:
                        if (actor->frameValue == 1.0f) {
                            func_8005AD64(actor, 0, -1, 0.0f);
                            player->idleTimer = mathRnd(30, 300);
                            actor = player->actor;
                        }
                        break;
                }
                if (!player->leaving && !player->active) {
                    if ((player->x - ov47Bss_2F0.x) * (player->x - ov47Bss_2F0.x) +
                        (player->z - ov47Bss_2F0.z) * (player->z - ov47Bss_2F0.z) < 1000.0f) {
                        func_80006EA0(actor);
                        player->actor = NULL;
                        actor = NULL;
                    }
                }
                if (actor != NULL) partUpdateTriggers(actor, updateRate);
            }
        }
    }
    ov47Data_550 = ov47Data_550 + ov47Data_554 * updateRate;
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
                for (j = 0; j < updateRate; j++) {
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
        O47_COMMAND(0x01000040, O47_PHYSICAL(D_800D3144));
        D_800D3144++;
        colour = ov47Data_3DC[colourIndex];
        O47_COMMAND(0x06000000, ov47Data_300);
        O47_COMMAND(0xFA000000, ((u32)(colour >> 24) << 24) |
            (((colour >> 16) & 255) << 16) | (((colour >> 8) & 255) << 8) | 255);
        O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
        O47_VERTICES(ov47Data_198, 4);
        O47_COMMAND(0x05100020, O47_PHYSICAL(ov47Data_1C0));
        camStandardOrtho(&D_800D3140, &D_800D3144);
        func_80034920(&D_800D3140);
        O47_COMMAND(0xFA000000, -1);
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
            colour = ov47Data_3DC[selected];
            red = (colour >> 24) & 255;
            green = (colour >> 16) & 255;
            blue = (colour >> 8) & 255;
            O47_COMMAND(0xFA000000,
                ((u32)(s32)(red + (255 - red) * ov47Data_540) << 24) |
                (((s32)(green + (255 - green) * ov47Data_540) & 255) << 16) |
                (((s32)(blue + (255 - blue) * ov47Data_540) & 255) << 8) | 255);
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
    p3 = D_800D3058;
    for (controller = 0; controller != 4; controller++, p3++) {
        if (p3->active && p3->actor != NULL) {
            if (ov47Bss_30A == 4) {
                textX = ov47Data_530[controller] + p3->screenX;
            } else {
                textX = p3->screenX - 20.0f;
            }
            barY = 116;
            for (stat = 0; stat != 4; stat++) {
                barX = textX;
                if (labelCount < 4) {
                    if (ov47Bss_30A != 4) {
                        func_8004B0F8(&D_800D3140, textX - 6, barY + 2, D_8007C0B8[145 + stat], 9);
                    } else {
                        func_8004B0F8(&D_800D3140, 160, barY + 2, D_8007C0B8[145 + stat], 12);
                    }
                    labelCount++;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xEF002C0F, 0x00504340);
                O47_COMMAND(0xB6000000, 0x00010001);
                O47_COMMAND(0xFCFFFFFF, 0xFFFDF6FB);
                O47_COMMAND(0xFA000000, ov47Data_3DC[controller]);
                count = ov47Data_4C8[ov47Data_524[p3->selector]][stat];
                while (count--) {
                    O47_RECTANGLE(barX, barY);
                    barX += 8;
                }
                O47_COMMAND(0xE7000000, 0);
                O47_COMMAND(0xFA000000, (ov47Data_3DC[controller] & ~0xFF) | 0x40);
                count = 5 - ov47Data_4C8[ov47Data_524[p3->selector]][stat];
                while (count--) {
                    O47_RECTANGLE(barX, barY);
                    barX += 8;
                }
                barY += 8;
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
 * score: 1580 differing words
 * frame: 0x280
 * relocations: 316
 * first-mismatch: +0x4
 * summary: Pair 1 +0x4 line 199 rate=updateRate is move_to_dest. register, same-line fold, dying s32: stall, still +4/1580.
 * PLATEAU-HANDOFF:func_overlay_047_F0000B30_1891948:end
 */
