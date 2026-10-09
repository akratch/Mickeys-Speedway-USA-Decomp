#include "ultra64.h"

typedef struct Overlay55Digit {
    void *resource;
    void *alternate;
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

typedef struct Overlay55Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay55Level;

typedef struct Overlay55Transform {
    void *resource;
    s32 unk04;
    s32 unk08;
    s16 x;
    s16 y;
    s32 unk10;
} Overlay55Transform;

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

typedef struct Overlay55DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay55DisplayCommand;

extern Overlay55Digit gOverlay55TimeDigits[4][10];
extern Overlay55Digit gOverlay55ClockDigits[4][2];
extern Overlay55Digit gOverlay55TimeTemplate[];
extern s32 gOverlay55TransitionDone;
extern s8 gOverlay55BlinkCounter;
extern s32 gOverlay55IconAlpha[];
extern s8 gOverlay55Items[];
extern f32 gOverlay55HudHeight;

extern u8 D_8007BEF4;
extern s16 D_8007C180[];
extern s32 D_800C947C;
extern Overlay55DisplayCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D31C8[];
extern MenuCurrentObject D_800D3550[];
extern s32 ext_o1_83e0;

extern u8 *func_80028F54(void);
extern void camStandardOrtho(Overlay55DisplayCommand **, void **);
extern Overlay55Object **func_80005750(s32 *);
extern void viGetCurrentSize(s32 *, s32 *);
extern void camSetNo(s32);
extern void camSetScissor(Overlay55DisplayCommand **);
extern void overlay56SplitTime(s32, s32 *, s32 *, s32 *);
extern Overlay55Level *levelGetLevel(void);
extern s32 func_800290A0(void);
extern void overlay55GetOffsets(s32, s32, s32 *, s32 *);
extern void func_80034920(Overlay55DisplayCommand **);
extern void func_8002F618(Overlay55DisplayCommand **, Overlay55Digit *, s32, s32, u8, u8, u8, u8);
extern void func_80039E34(s32);
extern s32 frontGetScreenMode(void);
extern void func_8002FB34(Overlay55DisplayCommand **, Overlay55Transform *, f32, f32, f32, f32, s32, u8);
extern s32 mainGetMode(void);
extern void mainChangeCameras(s32);
extern void func_800016EC(u8);
extern void func_8003A590(void);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void amTuneSetFade(f32, u8);

/* Overlay 55's HUD update: the four-player sibling of overlay 53's
 * func_overlay_053_F0000240_189DBE8 and of overlay52TailB, written the same
 * way. Callees and resident data are the ones this overlay's relocation
 * records name, with overlay52TailB's prototypes (u8 colour and mode
 * arguments; func_8002FB34's last argument is u8). Built with -Wab,-r4300_mul
 * (mk/overlays.mk) for the rotated easing loop. The declarations are the
 * shipped frame: `item` and `clock` sit where the frame has otherwise unused
 * cells. The clock row's address is taken after the digit row's (that orders
 * its spill cell), the item is read once at the icon's join, the counter
 * increment is a (u8) truncation and the icon's 53 arm masks its two
 * coordinates (each one ring draw). The arm's two stores and the packet's
 * two words share a line each, as a macro expansion would (L59). */
void func_overlay_055_F000031C_18A1E34(s32 updateRate) {
    s32 i;
    s32 player;
    s32 digitX;
    s32 digitY;
    Overlay55Object **objects;
    Overlay55PlayerState *state;
    Overlay55Digit *digits;
    Overlay55Level *level;
    s32 objectCount;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudOffset;
    s32 iconX;
    s32 iconY;
    Overlay55DisplayCommand *command;
    s32 pad1;
    s32 item;
    Overlay55Transform transform;
    s32 width;
    s32 height;
    u8 *gameState;
    Overlay55Digit *clock;

    gameState = func_80028F54();
    camStandardOrtho(&D_800D3140, &D_800D3144);
    objects = func_80005750(&objectCount);
    if (D_800C947C == 0) {
        for (i = 0; i < updateRate; i++) {
            gOverlay55HudHeight += (-11.0f - gOverlay55HudHeight) * 0.125f;
        }
    }
    hudOffset = (s32)gOverlay55HudHeight;
    viGetCurrentSize(&width, &height);
    gOverlay55BlinkCounter = (u8)(gOverlay55BlinkCounter + 1);
    gOverlay55BlinkCounter %= 10;

    for (player = 0; player < D_8007BEF4; player++) {
        if (objects[player] == NULL) {
            return;
        }
        state = objects[player]->state;
        camSetNo(player);
        camSetScissor(&D_800D3140);
        if (*gameState == 6) {
            digits = gOverlay55TimeDigits[player];
            clock = gOverlay55ClockDigits[player];
            overlay56SplitTime(state->time, &minutes, &seconds, &centiseconds);
            level = levelGetLevel();
            if (D_800C947C == 0 && state->racerIndex != level->laps &&
                func_800290A0() == 0 && state->time != 0x83D60) {
                centiseconds -= centiseconds % 10;
                centiseconds += gOverlay55BlinkCounter;
            }
            digits[0].value = (minutes / 10) << 16;
            digits[1].value = (minutes % 10) << 16;
            digits[3].value = (seconds / 10) << 16;
            digits[4].value = (seconds % 10) << 16;
            digits[6].value = (centiseconds / 10) << 16;
            digits[7].value = (centiseconds % 10) << 16;
            overlay55GetOffsets(player, 0, &digitX, &digitY);
            for (i = 0; i < 8; i++) {
                if ((digits[i].value >> 16) == 1) {
                    if (i == 0 || i == 3 || i == 6) {
                        digits[i].x = gOverlay55TimeTemplate[i].x + digitX + 1;
                    } else {
                        digits[i].x = gOverlay55TimeTemplate[i].x + digitX - 1;
                    }
                } else {
                    digits[i].x = gOverlay55TimeTemplate[i].x + digitX;
                }
            }
            func_80034920(&D_800D3140);
            func_8002F618(&D_800D3140, digits, 0, hudOffset, 255, 255, 255, 255);
            func_80034920(&D_800D3140);
            overlay55GetOffsets(player, 0, &digitX, &digitY);
            D_800D3550[4].unkC = digitX - 0xAD;
            D_800D3550[4].unk10 = (-digitY - hudOffset) + 0x74;
            D_800D3550[4].unk4 = state->time * -65536 / 300;
            func_80039E34(4);
            func_8002F618(&D_800D3140, clock, 0, hudOffset, 255, 255, 255, 255);
        }
        if (state->character != 255) {
            gOverlay55IconAlpha[player] += updateRate * 16;
            if (gOverlay55IconAlpha[player] >= 0xA5) {
                gOverlay55IconAlpha[player] = 0xA4;
            }
        } else {
            gOverlay55IconAlpha[player] -= updateRate * 8;
            if (gOverlay55IconAlpha[player] < 0) {
                gOverlay55IconAlpha[player] = 0;
            }
        }
        if (gOverlay55IconAlpha[player] > 0) {
            if (state->effectTimer != 0) {
                gOverlay55Items[player] = 53;
            } else if (state->character != 255) {
                gOverlay55Items[player] = D_8007C180[state->character];
            }
            if (gOverlay55Items[player] != -1) {
                if (frontGetScreenMode() == 1) {
                    iconX = (player & 1) ? 0x1A2 : 0x25;
                    iconY = (player < 2) ? 0x86 : 0x13C;
                } else {
                    iconX = (player & 1) ? 0x1A2 : 0x25;
                    iconY = (player < 2) ? 0x86 : 0x12A;
                }
                item = gOverlay55Items[player];
                transform.x = iconX;
                transform.y = iconY;
                if (item == 53) {
                    transform.x = (iconX - 7) & 0xFFFF; transform.y = (iconY - 6) & 0xFFFF;
                }
                transform.resource = D_800D31C8[item];
                transform.unk08 = 0;
                transform.unk04 = 0;
                transform.unk10 = 0;
                command = D_800D3140++;
                command->w0 = 0xFA000000; command->w1 = 0xFFFFFFFF;
                func_8002FB34(&D_800D3140, &transform, 0.0f, 0.0f, 0.66f, 0.66f,
                              gOverlay55IconAlpha[player] | ~0xFF, 1);
            }
        } else {
            gOverlay55Items[player] = -1;
        }
        if (mainGetMode() == 0 && *func_80028F54() == 5 && ext_o1_83e0 == 0 &&
            gOverlay55TransitionDone == 0) {
            mainChangeCameras(1);
            func_800016EC(1);
            func_8003A590();
            func_80037414(2, 4.0f, -1.0f, 0, 0, 0, 0);
            mainChangeLevel(18, 0, 0, 7, 1, 1);
            amTuneSetFade(3.0f, 0);
            gOverlay55TransitionDone = 1;
        }
    }
    camStandardOrtho(&D_800D3140, &D_800D3144);
    camSetNo(0);
}
