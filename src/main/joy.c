#include "ultra64.h"
#include "game/saves.h"

/*
 * Resident controller input, ROM 0x25C20-0x263F0.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/joy.c and built src/controller.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern u16 D_8007A0C8;
extern s32 D_8007A0C0;
extern OSMesgQueue D_800CF340;
extern OSMesg D_800CF358;
extern OSMesg D_800CF35C;
extern OSContStatus D_800CF360[];

/* PROVENANCE: field roles adapted from JFG src/joy.c; Mickey layout is decisive. */
typedef struct JoyPad {
    u16 button;
    s8 stick_x;
    s8 stick_y;
    u8 errno;
    u8 unused;
} JoyPad;

extern void osContGetReadData(JoyPad *);
extern JoyPad D_800CF370[];
extern JoyPad D_800CF388[];
extern s8 D_800CF372[];
extern s8 D_800CF373[];
extern u16 D_800CF3A0[];
extern u16 D_800CF3A8[];
typedef u8 JoyControllerMap[4];
JoyControllerMap D_800CF3B0;
u8 D_800CF3B4[4];
u8 D_800CF3B8[4];
s32 D_800CF3BC;
extern s32 func_8003A550(void);
extern void TrapDanglingJump();
extern s32 osContInit(OSMesgQueue *, u8 *, OSContStatus *);
extern s32 osContStartReadData(OSMesgQueue *);
extern void joyResetMap(void);
extern void rumbleTick(s32);
extern u8 D_800D3128[];

#define joySaveActionA func_8002C94C
#define joySaveActionB func_8002CB18
#define joySaveActionC func_8002CD6C
#define joySaveActionD func_8002CE54
#define joySaveActionE func_8002CF0C
#define joySaveActionF func_8002CF6C
#define joySaveActionG func_8005807C
#define joySaveActionH func_800580F0
extern void joySaveActionA(s32);
extern void joySaveActionB(void);
extern void joySaveActionC(void);
extern void joySaveActionD(void *);
extern void joySaveActionE(void *);
extern void joySaveActionG(void);
extern void joySaveActionH(s32);

#define joyCurrentPads D_800CF370
#define joyPreviousPads D_800CF388
#define joyButtonsPressed D_800CF3A0
#define joyButtonsReleased D_800CF3A8
#define joyEnabledPads D_800CF3B4
#define joyConnectedPads D_800CF3B8
#define joyConnectedCount D_800CF3BC
#define joySecurityMask D_8007A0C8
#define joyDisableAll D_8007A0C0
#define joySaveState D_800D3128

s8 joyClamp(s8 stickMag);

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
OSMesgQueue *joyMessageQ(void) {
    return &D_800CF340;
}

// PROVENANCE: adapted from jfg src/joy.c:joyInit.
/*
 * The contiguous enabled-pad view and do-loop reproduce the target's 83
 * instructions and -0x28 frame. The Makefile's metadata-only normalization
 * preserves the retail endpoint and interior-byte relocation identities.
 */
s32 joyInit(void) {
    s32 i;
    u8 bitpattern;

    osCreateMesgQueue(&D_800CF340, &D_800CF358, 1);
    osSetEventMesg(5, &D_800CF340, D_800CF35C);
    osContInit(&D_800CF340, &bitpattern, D_800CF360);
    osContStartReadData(&D_800CF340);
    joyResetMap();

    for (i = 0; i < 4; i++) {
        D_800CF3B4[i] = TRUE;
        D_800CF3B8[i] = FALSE;
    }

    D_800CF3BC = 0;
    for (i = 0; i < 4; i++) {
        if ((bitpattern & (1 << i)) && !(D_800CF360[i].errno & 8)) {
            D_800CF3B8[i] = TRUE;
            D_800CF3BC++;
        }
    }

    if (D_800CF3B8[0] != 0) {
        return 0;
    }
    i = 0;
    do {
        D_800CF3B4[i] = FALSE;
        i++;
    } while (i < 4);
    D_8007A0C0 = 1;
    return -1;
}

/*
 * PROVENANCE: body structure adapted from Jet Force Gemini src/joy.c::joyRead;
 * Mickey's save-flag calls and byte identity are decisive.
 *
 * Matched 2026-10-02 (lane w2-front) by three source facts, none of them
 * allocator work:
 *  - osContGetReadData has a real `void` prototype. Without one IDO treats
 *    the call as returning int, and the controller-array cursor web, which
 *    spans all three loops, is no longer offered v0 (38 -> 0 words).
 *  - func_8002CD6C takes no arguments (its definition in saves.c is void);
 *    the a0/a1/a2 the target leaves at that call are the second loop's
 *    leftovers, not arguments.
 *  - `i` is assigned again inside the receive block. A later definition of
 *    the counter in the same region keeps uopt from rewriting the second
 *    loop's `<` exit test into `!=`; without it the function is 4 bytes
 *    short.
 */
s32 joyRead(s32 saveDataFlags, s32 updateRate) {
    s32 i;
    OSMesg unusedMsg;

    if (osRecvMesg(&D_800CF340, &unusedMsg, OS_MESG_NOBLOCK) == 0) {
        for (i = 0; i < 4; i++) {
            joyPreviousPads[i] = joyCurrentPads[i];
        }
        osContGetReadData(joyCurrentPads);
        joyConnectedCount = 0;
        for (i = 0; i < 4; i++) {
            if (joyCurrentPads[i].errno == 0) {
                joyConnectedPads[i] = TRUE;
                joyConnectedCount++;
            } else {
                joyConnectedPads[i] = FALSE;
            }
        }
        if (saveDataFlags != 0) {
            if (saveDataFlags & 0x80000000) {
                joySaveActionC();
            }
            if (saveDataFlags & 0x40000000) {
                joySaveActionB();
            }
            if (saveDataFlags & 0x20000000) {
                i = saveDataFlags & 0x1F;
                joySaveActionA(i);
            }
            if (saveDataFlags & 0x00800000) {
                joySaveActionF(joySaveState);
            }
            if (saveDataFlags & 0x10000000) {
                joySaveActionD(joySaveState);
            }
            if (saveDataFlags & 0x08000000) {
                joySaveActionE(joySaveState);
            }
            if (saveDataFlags & 0x04000000) {
                joySaveActionG();
            }
            if (saveDataFlags & 0x02000000) {
                joySaveActionH(1);
            }
            if (saveDataFlags & 0x01000000) {
                joySaveActionH(0);
            }
            saveDataFlags = 0;
        }
        rumbleTick(updateRate);
        osContStartReadData(&D_800CF340);
    }
    for (i = 0; i < 4; i++) {
        if ((joyEnabledPads[i] == 0) || (joyDisableAll != 0)) {
            joyCurrentPads[i].button = 0;
            joyCurrentPads[i].stick_x = 0;
            joyCurrentPads[i].stick_y = 0;
        }
        joyButtonsPressed[i] =
            ((joyCurrentPads[i].button ^ joyPreviousPads[i].button) &
             joyCurrentPads[i].button) & joySecurityMask;
        joyButtonsReleased[i] =
            ((joyCurrentPads[i].button ^ joyPreviousPads[i].button) &
             joyPreviousPads[i].button) & joySecurityMask;
    }
    return saveDataFlags;
}

/*
 * PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive.
 * Plateau: a fresh m2c tail-byte anchor still emits 10/9 words, with ten
 * positional differences from +0x0. Original TU-local BSS ownership is needed
 * for the shared HI16 and four distinct LO16 identities.
 */
void joyResetMap(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        D_800CF3B0[i] = i;
    }
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyDisable(s32 player) {
    D_800CF3B4[player & 3] = FALSE;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyEnable(s32 player) {
    D_800CF3B4[player & 3] = TRUE;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyCreateMap(s8 *activePlayers) {
    s32 i;
    s32 temp = 0;

    for (i = 0; i < 4; i++) {
        if (activePlayers[i]) {
            D_800CF3B0[temp++] = i;
        }
    }
    for (i = 0; i < 4; i++) {
        if (!activePlayers[i]) {
            D_800CF3B0[temp++] = i;
        }
    }
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u8 joyGetController(s32 player) {
    return D_800CF3B0[player];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetButtons(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF370[D_800CF3B0[player]].button;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetPressed(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF3A0[D_800CF3B0[player]];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetReleased(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF3A8[D_800CF3B0[player]];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetStickX(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return joyClamp(D_800CF372[D_800CF3B0[player] * 6]);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetAbsX(s32 player) {
    return D_800CF372[D_800CF3B0[player] * 6];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetStickY(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return joyClamp(D_800CF373[D_800CF3B0[player] * 6]);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetAbsY(s32 player) {
    return D_800CF373[D_800CF3B0[player] * 6];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyClamp(s8 stickMag) {
    if ((stickMag < 5) && (stickMag > -5)) {
        return 0;
    }
    if (stickMag > 0) {
        stickMag -= 5;
        if (stickMag > 65) {
            stickMag = 65;
        }
    } else {
        stickMag += 5;
        if (stickMag < -65) {
            stickMag = -65;
        }
    }
    return stickMag;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joySetSecurity(void) {
    D_8007A0C8 = 0;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void arithmeticFunction(u8 *challenge, u8 *response) {
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    TrapDanglingJump(challenge, &D_800CF340);
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    TrapDanglingJump(&D_800CF340);
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    osContStartReadData(&D_800CF340);
    TrapDanglingJump(response);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s32 joyCharVal(void) {
    return 1;
}
