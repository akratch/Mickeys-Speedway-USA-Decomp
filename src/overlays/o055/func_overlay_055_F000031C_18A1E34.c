#include "ultra64.h"

typedef struct Overlay55Digit {
    u8 pad00[8];
    s32 value;
    s16 x;
    s16 y;
} Overlay55Digit;

typedef struct Overlay55PlayerState {
    u8 pad000[0x19A];
    u8 character;
    u8 pad19B;
    s32 effectTimer;
    u8 pad1A0[0x383 - 0x1A0];
    s8 racerIndex;
    u8 pad384[0x400 - 0x384];
    s32 time;
} Overlay55PlayerState;

typedef struct Overlay55Object {
    u8 pad00[0x64];
    Overlay55PlayerState *state;
} Overlay55Object;

typedef struct Overlay55Transform {
    s32 resource;
    s32 unk04;
    s32 unk08;
    s16 x;
    s16 y;
    s32 unk10;
} Overlay55Transform;

typedef struct Overlay55MenuPlacement {
    u8 pad00[0x84];
    s16 angle;
    u8 pad86[6];
    f32 x;
    f32 y;
} Overlay55MenuPlacement;

typedef struct Overlay55DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay55DisplayCommand;

extern Overlay55Digit D_20[];
extern Overlay55Digit D_50[];
extern Overlay55Digit D_80[];
extern Overlay55Digit D_A0[];
extern Overlay55Digit D_280[];
extern s8 D_F4;
extern s32 D_F8[];
extern s8 D_304[];
extern f32 D_308;
extern s16 gOverlay55CharacterIcons[];
extern void *gOverlay55IconObjects[];

extern u8 D_8007BEF4;
extern s16 D_8007C180[];
extern s32 D_800C947C;
extern Overlay55DisplayCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D31C8[];
extern Overlay55MenuPlacement D_800D3550[];
extern s32 gOverlay1TransitionStateReloc;

extern u8 *func_80028F54(void);
extern void func_80022A50();
extern Overlay55Object **func_80005750(s32 *count);
extern void viGetCurrentSize();
extern void camSetNo();
extern void func_80022610();
extern void overlay56SplitTime();
extern u8 *levelGetLevel(void);
extern s32 func_800290A0(void);
extern void overlay55GetOffsets();
extern void func_80034920();
extern void func_8002F618();
extern void func_80039E34();
extern s32 frontGetScreenMode(void);
extern void func_8002FB34(void *, Overlay55Transform *, s32, s32, f32, f32,
                         s32, s32);
extern s32 mainGetMode(void);
extern void mainChangeCameras();
extern void func_800016EC();
extern void func_8003A590(void);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel();
extern void func_800005CC(f32, s32);

#ifdef NON_MATCHING
/* Workbench plateau: mixed structure/schedule/register, 575/581 instructions, exact 0xE0
 * frame, 445 positional words, first +0x2C. Typed func_80005750(s32 *count)/objectCount
 * ABI probe emitted no change; the display/transition lifetime web remains. */
/* Mickey-local reconstruction. The display-list and transition call roles
 * are established by this overlay's relocation records; the object layout is
 * shared with the resident player-control code. */
void func_overlay_055_F000031C_18A1E34(s32 updateRate) {
    s32 digitX;
    s32 digitY;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 screenWidth;
    s32 screenHeight;
    s32 heightOffset;
    s32 playerIndex;
    s16 iconX;
    s16 iconY;
    Overlay55Transform transform;
    Overlay55Object **objects;
    Overlay55Object *object;
    Overlay55PlayerState *player;
    Overlay55Digit *digits;
    Overlay55Digit *digit;
    Overlay55Digit *source;
    Overlay55MenuPlacement *placement;
    Overlay55DisplayCommand *command;
    s32 objectCount;
    u8 *gameState;
    u8 *level;
    s8 *icon;
    s32 *alpha;

    gameState = func_80028F54();
    func_80022A50(&D_800D3140, &D_800D3144);
    objects = func_80005750(&objectCount);

    if (D_800C947C == 0) {
        s32 fadeIndex;

        for (fadeIndex = 0; fadeIndex < updateRate; fadeIndex++) {
            D_308 += (-11.0f - D_308) * 0.125f;
        }
    }
    heightOffset = (s32) D_308;
    viGetCurrentSize(&screenWidth, &screenHeight);
    D_F4 += 1;
    D_F4 = (s8) ((s8) D_F4 % 10);

    for (playerIndex = 0; playerIndex < D_8007BEF4; playerIndex++) {
        object = objects[playerIndex];
        if (object == NULL) {
            continue;
        }
        player = object->state;
        icon = &D_304[playerIndex];
        alpha = &D_F8[playerIndex];

        camSetNo(playerIndex);
        func_80022610(&D_800D3140);
        if (gameState[0] == 6) {
            digits = (Overlay55Digit *)
                ((u8 *) D_20 + (playerIndex * 0xA0));
            overlay56SplitTime(player->time, &minutes, &seconds,
                               &centiseconds);
            level = levelGetLevel();
            if ((D_800C947C == 0) &&
                (level[0x86] != (u8) player->racerIndex) &&
                (func_800290A0() == 0) && (player->time != 0x83D60)) {
                centiseconds = (centiseconds - (centiseconds % 10)) + D_F4;
            }

            digits[0].value = (minutes / 10) << 16;
            digits[1].value = (minutes % 10) << 16;
            digits[3].value = (seconds / 10) << 16;
            digits[4].value = (seconds % 10) << 16;
            digits[6].value = (centiseconds / 10) << 16;
            digits[7].value = (centiseconds % 10) << 16;

            overlay55GetOffsets(playerIndex, 0, &digitX, &digitY);
            source = D_20;
            digit = digits;
            do {
                if ((digit->value >> 16) == 1) {
                    if ((source == D_20) || (source == D_50) ||
                        (source == D_80)) {
                        digit->x = source->x + digitX + 1;
                    } else {
                        digit->x = source->x + digitX - 1;
                    }
                } else {
                    digit->x = source->x + digitX;
                }
                source++;
                digit++;
            } while (source != D_A0);

            func_80034920(&D_800D3140);
            func_8002F618(NULL, digits, 0, heightOffset,
                          0xFF, 0xFF, 0xFF, 0xFF);
            func_80034920(&D_800D3140);

            overlay55GetOffsets(playerIndex, 0, &digitX, &digitY);
            placement = &D_800D3550[0];
            placement->x = digitX - 0xAD;
            placement->y = (-digitY - heightOffset) + 0x74;
            placement->angle = (s16) ((player->time * -0x10000) / 300);
            func_80039E34(4);
            func_8002F618(NULL, &D_280[playerIndex * 2], 0, heightOffset,
                          0xFF, 0xFF, 0xFF, 0xFF);
        }

        if (player->character != 0xFF) {
            *alpha += updateRate * 0x10;
            if (*alpha >= 0xA5) {
                *alpha = 0xA4;
            }
        } else {
            *alpha -= updateRate * 8;
            if (*alpha < 0) {
                *alpha = 0;
            }
        }

        if (*alpha > 0) {
            if (player->effectTimer != 0) {
                *icon = 0x35;
            } else if (player->character != 0xFF) {
                *icon = (s8) D_8007C180[player->character];
            }

            if (*icon != -1) {
                if (frontGetScreenMode() == 1) {
                    iconX = (playerIndex & 1) ? 0x1A2 : 0x25;
                    iconY = (playerIndex < 2) ? 0x86 : 0x13C;
                } else {
                    iconX = (playerIndex & 1) ? 0x1A2 : 0x25;
                    iconY = (playerIndex < 2) ? 0x86 : 0x12A;
                }
                if (*icon == 0x35) {
                    iconX -= 7;
                    iconY -= 6;
                }

                command = D_800D3140++;
                command->w0 = 0xFA000000;
                command->w1 = 0xFFFFFFFF;
                transform.resource = (s32) D_800D31C8[*icon];
                transform.unk04 = 0;
                transform.unk08 = 0;
                transform.x = iconY;
                transform.y = iconX;
                transform.unk10 = 0;
                func_8002FB34(NULL, &transform, 0, 0, 0.0f, 0.0f,
                              *alpha | ~0xFF, 1);
            }
        } else {
            *icon = -1;
        }

        if ((mainGetMode() == 0) && (func_80028F54()[0] == 5) &&
            (D_800C947C == 0) && (gOverlay1TransitionStateReloc == 0)) {
            mainChangeCameras(1);
            func_800016EC(1);
            func_8003A590();
            func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
            mainChangeLevel(0x12, 0, 0, 7, 1, 1);
            func_800005CC(3.0f, 0);
            gOverlay1TransitionStateReloc = 1;
        }
    }

    func_80022A50(&D_800D3140, &D_800D3144);
    camSetNo(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o055/func_overlay_055_F000031C_18A1E34/func_overlay_055_F000031C_18A1E34.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_055_F000031C_18A1E34:start
 * symbol: func_overlay_055_F000031C_18A1E34
 * score: 445 differing words
 * frame: 0xE0
 * relocations: 90
 * first-mismatch: +0x3C
 * summary: V0: 575/581 words, exact 0xE0 frame, 445 raw diffs. Relocs 90 versus 102; 34 sites and 20 identities align. Display/transition lifetime web persists.
 * PLATEAU-HANDOFF:func_overlay_055_F000031C_18A1E34:end
 */
