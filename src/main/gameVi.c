/*
 * Video-interface and framebuffer management -- ROM 0x34180-0x34E60
 * (VRAM 0x80033580-0x80034260).
 *
 * The translation-unit boundary follows Jet Force Gemini's public gameVi.c:
 * Mickey has the same complete, ordered 23-function sequence from viInit to
 * fb_memcpy. The preceding function is the separately measured
 * trapDanglingJump TU; the next function has the structure and call surface of
 * texInitTextures, the first function in JFG's following textures.c TU.
 *
 * PROVENANCE -- the translation-unit name and function identifications used
 * while reconstructing this file come from JFG's public decompilation,
 * src/gameVi.c and src/gameVi.h. Mickey's own bytes decide every boundary,
 * type, body and matching verdict. Unmatched functions retain their Mickey
 * func_ names below; only evidence-backed names are adopted.
 *
 * Flags: -O2 -mips2 -32, the resident game-code group.
 */

#include "game/gameVi.h"
#include "game/font.h"
#include "game/memory.h"
#include "PR/os_message.h"
#include "PR/os_vi.h"

typedef struct ResolutionSettings {
    s32 width;
    s32 height;
    s32 displayWidth;
    s32 displayHeight;
    s32 verticalOffset;
    s32 videoMode;
    char name[16];
} ResolutionSettings;

extern s32 *D_8007A680[4];
extern s32 *D_8007A690[3];
extern f32 D_8007A69C;
extern f32 D_8007A6A0;
extern s32 D_8007A6A4;
extern s32 D_8007A6A8;
extern ResolutionSettings D_8007A6AC[];
extern u8 D_8007A1A0;
extern f32 D_80082490;
extern f32 D_80082494;
extern f32 D_80082498;
extern OSViMode D_80080490;
extern OSViMode D_800804E0;
extern OSViMode D_80080530;
extern OSViMode D_800805D0;
extern OSViMode D_80080620;
extern OSViMode D_800806C0;
extern OSViMode D_80080710;
extern OSViMode D_800807B0;
extern OSViMode D_80080800;
extern OSMesg D_800D2E50[8];
extern OSMesgQueue D_800D2E70;
extern u8 D_800D2F30[];
extern s32 D_800D2F90;
extern s32 *D_800D2FA8;
extern s32 *D_800D2FAC;
extern s8 D_800D2F94;
extern s8 D_800D2F95;
extern s8 D_800D2F96;
extern s8 D_800D2F97;
extern s8 D_800D2F98;
extern s8 D_800D2F99;
extern s8 D_800D2F9A;
extern u8 D_800D2F9B;
extern u8 D_800D2F9C;
extern u8 D_800D2F9D;
extern s32 *D_800D2FA0;
extern s32 *D_800D2FA4;
extern s32 D_800D2FBC;
extern s32 D_800D2FC0;
extern s32 D_800D2FB0;
extern f32 D_800D2FB4;
extern f32 D_800D2FB8;
extern OSViMode D_800D2F40;
extern u32 D_800D2F48;
extern u32 D_800D2F60;
extern OSViFieldRegs D_800D2F68;
extern s32 D_800D3128;

extern MemoryPoolSlot *mmAlloc(s32 size, u32 colourTag);
extern void osScAddClient(void *scheduler, void *client, OSMesgQueue *queue, s32 id);
extern void func_800336A8(s32 videoMode);
extern void viAllocateZBuffer(s32 width, s32 height);
extern void viFreeZBuffer(s32 width, s32 height);
extern void viGetCurrentSize(s32 *width, s32 *height);
extern void viSetTiming(void);
extern void func_80034018(u8 *src, s32 length);
extern OSViMode *func_80034094(s32 videoMode);
extern void fb_swap(void);
extern void osWritebackDCacheAll(void);
extern void osViSetMode(OSViMode *mode);
extern void osViSetSpecialFeatures(u32 features);
extern void osViBlack(u8 active);
extern void osViSwapBuffer(void *framebuffer);

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viInit. */
void viInit(void *scheduler) {
    if (osTvType == OS_TV_TYPE_PAL) {
        D_800D2FB0 = 50;
        D_800D2FB4 = D_80082490;
        D_800D2FB8 = D_80082494;
    } else {
        D_800D2FB0 = 60;
        D_800D2FB4 = D_80082498;
        D_800D2FB8 = 1.0f;
    }
    osCreateMesgQueue(&D_800D2E70, D_800D2E50, 8);
    osScAddClient(scheduler, D_800D2F30, &D_800D2E70, 2);
    D_8007A680[0] = (s32 *) mmAlloc(0x4B030, 0x92);
    D_800D2F9A = 0;
    D_800D2F95 = 0;
    D_800D2F96 = 0;
    D_800D2F97 = 1;
    D_800D2FC0 = 0;
    func_800336A8(0);
    osViBlack(0);
    osViSwapBuffer(D_800D2FA8);
    D_8007A6A8 = 1;
    D_800D2F99 = 1;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/gameVi.c:viChangeMode (efd5abb). */
void func_800336A8(s32 videoMode) {
    s32 bufferSize;
    ResolutionSettings *resolution;

    D_800D2F98 = (videoMode & 3) & ~D_8007A6A8;
    D_8007A6A8 = 0;
    if (osTvType == OS_TV_TYPE_MPAL) {
        D_800D2F98 += 4;
    } else if ((osTvType == OS_TV_TYPE_PAL) && (D_8007A6A4 == 0)) {
        D_800D2F98 = 14;
    } else if (osTvType == OS_TV_TYPE_PAL) {
        D_800D2F98 += 8;
    }
    resolution = &D_8007A6AC[D_800D2F98];
    D_8007A69C = resolution->width / 320.0f;
    D_8007A6A0 = resolution->height / 240.0f;
    fontSetWindow0(resolution->width, resolution->height);
    if (D_800D2FC0 == 0) {
        viFreeZBuffer(resolution->width, resolution->height);
    }
    if (D_8007A680[1] != NULL) {
        mmFree(D_8007A680[1]);
        D_8007A680[1] = NULL;
    }
    if (D_8007A680[2] != NULL) {
        mmFree(D_8007A680[2]);
        D_8007A680[2] = NULL;
    }
    bufferSize = resolution->width * resolution->height * 2;
    D_800D2F94 = 0;
    D_8007A690[0] = (s32 *) (((s32) D_8007A680[0] + 0x3F) & ~0x3F);
    D_8007A690[1] = NULL;
    D_8007A690[2] = NULL;
    /* JFG tests the two modes with `!=` and opens the then-arm with an empty
     * `if (1) {}`; the switch gives the same blocks without it. */
    switch (D_800D2F98 & 3) {
        case 2:
        case 3:
            D_8007A680[1] = (s32 *) mmAlloc(bufferSize + 0x30, 0x92);
            D_8007A690[1] = (s32 *) (((s32) D_8007A680[1] + 0x3F) & ~0x3F);
            break;
        default:
            D_8007A690[1] = (s32 *) ((u8 *) D_8007A690[0] + bufferSize);
            break;
    }
    if (D_800D2F96) {
        D_8007A680[2] = (s32 *) mmAlloc(bufferSize + 0x30, 0x92);
        D_8007A690[2] = (s32 *) (((s32) D_8007A680[2] + 0x3F) & ~0x3F);
    }
    D_800D2F95 = D_800D2F96;
    if (D_800D2F97) {
        func_80034018((u8 *) D_8007A690[0], bufferSize);
        func_80034018((u8 *) D_8007A690[1], bufferSize);
        if (D_800D2F95) {
            func_80034018((u8 *) D_8007A690[2], bufferSize);
        }
    }
    D_800D2F97 = 1;
    if (D_800D2FC0 == 0) {
        viAllocateZBuffer(resolution->width, resolution->height);
    }
    viFrameRateReset();
    fb_swap();
    viSetTiming();
    D_8007A6A4 = 1;
    D_800D2FC0 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viReset. */
void func_800339B4(void) {
    s32 *screen;
    u32 screenSize;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    screen = (s32 *) 0x80380000;
    screenSize = (u32) (width * height) >> 1;
    while (screenSize--) {
        *screen++ = 0;
    }
    osWritebackDCacheAll();
    osViSwapBuffer((void *) 0x80380000);
    if ((D_800D2F98 != 0) && (D_800D2F98 != 4)) {
        if (osTvType == OS_TV_TYPE_PAL) {
            D_800D2F98 = 14;
        } else if (osTvType == OS_TV_TYPE_MPAL) {
            D_800D2F98 = 13;
        } else {
            D_800D2F98 = 12;
        }
        D_800D2F9A = 0;
        viSetTiming();
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viAllocateZBuffer. */
void viAllocateZBuffer(s32 width, s32 height) {
    D_800D2F90 = (width * height * 2) + 0x30;
    D_8007A680[3] = (s32 *) mmAlloc(D_800D2F90, 0x92);
    D_800D2FAC = (s32 *) (((s32) D_8007A680[3] + 0x3F) & ~0x3F);
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viFreeZBuffer. */
void viFreeZBuffer(s32 width, s32 height) {
    if (D_8007A680[3] != NULL) {
        mmFree(D_8007A680[3]);
        D_8007A680[3] = NULL;
        D_800D2FAC = NULL;
        D_800D2F90 = 0;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viSetTiming. */
void viSetTiming(void) {
    OSViMode *viMode;
    s32 verticalOffset;
    s32 displayHeight;
    s32 i;
    ResolutionSettings *resolution;
    s32 verticalAdjust;

    resolution = &D_8007A6AC[D_800D2F98];
    viMode = func_80034094(resolution->videoMode);
    fb_memcpy((u8 *) viMode, (u8 *) &D_800D2F40, sizeof(OSViMode));
    D_800D2F48 = resolution->width;
    D_800D2F60 = (resolution->width << 9) / resolution->displayWidth;
    verticalOffset = resolution->verticalOffset;
    displayHeight = resolution->displayHeight;
    if ((D_800D2F98 & 1) && (((u32) (D_800D3128 * 2) >> 31) != 0)) {
        verticalOffset += D_800D2F9A;
    }
    verticalAdjust = -verticalOffset - displayHeight;
    verticalAdjust += 240;
    for (i = 0; i < 2; i++) {
        D_800D2F40.fldRegs[i].origin = resolution->width * 2;
        D_800D2F40.fldRegs[i].yScale =
            (resolution->height << 10) / displayHeight;
        D_800D2F40.fldRegs[i].vStart += verticalOffset << 17;
        D_800D2F40.fldRegs[i].vStart -= (u32) verticalAdjust << 1;
    }
    if (D_8007A6A4 != 0) {
        osViSetMode(&D_800D2F40);
    }
    osViSetSpecialFeatures(0x52);
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viGetCurrentSize. */
void viGetCurrentSize(s32 *width, s32 *height) {
    *width = D_8007A6AC[D_800D2F98].width;
    *height = D_8007A6AC[D_800D2F98].height;
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viConvertXY. */
void viConvertXY(s32 *x, s32 *y) {
    *x = *x * D_8007A69C;
    *y = *y * D_8007A6A0;
}
/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viGetScaleXY. Mickey has no same-address caller evidence and the exact
 * skeleton is below the tier-A uniqueness threshold, so the name is not
 * adopted.
 */
void func_80033D58(f32 *hScale, f32 *vScale) {
    *hScale = D_8007A69C;
    *vScale = D_8007A6A0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viFrameRateReset(void) {
    D_800D2F9D = 1;
    D_800D2F9B = 0;
    D_800D2F9C = 2;
    D_800D2FBC = 0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viFrameSync. */
s32 viFrameSync(s32 mesg) {
    u8 tempUpdateRate;

    tempUpdateRate = 1;
    if (D_800D2F99) {
        D_800D2F99--;
    }
    if (mesg != 8) {
        fb_swap();
    }
    while (osRecvMesg(&D_800D2E70, NULL, OS_MESG_NOBLOCK) != -1) {
        tempUpdateRate++;
    }
    if (D_800D2F9D == 0) {
        if (tempUpdateRate >= D_800D2F9C) {
            D_800D2F9B = 0;
            if ((D_800D2F9C < tempUpdateRate) && (D_800D2F9C < 4)) {
                D_800D2F9C++;
            }
        } else if (D_800D2F9C >= 3) {
            D_800D2F9B += tempUpdateRate;
            if (D_800D2F9B >= 30) {
                D_800D2F9B = 0;
                D_800D2F9C--;
            }
        }
    }
    if (D_800D2FBC) {
        D_800D2F9C = 1;
    }
    while (tempUpdateRate < D_800D2F9C) {
        osRecvMesg(&D_800D2E70, NULL, OS_MESG_BLOCK);
        tempUpdateRate++;
    }
    osViSwapBuffer(D_800D2FA8);
    osRecvMesg(&D_800D2E70, NULL, OS_MESG_BLOCK);
    D_800D2F9D = 0;
    return tempUpdateRate;
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viGetVideoMode(void) {
    return D_800D2F98 & 3;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s8 viGetWideAdjust(void) {
    return D_800D2F9A;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viSetWideAdjust(s32 offset) {
    if (offset < -30) {
        offset = -30;
    }
    if (offset > 30) {
        offset = 30;
    }
    D_800D2F9A = offset;
    viSetTiming();
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viSetTrippleBuffer(s32 resolutionIndex) {
    D_800D2F96 = resolutionIndex & 1;
}

/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viGetTrippleBuffer. No same-address Mickey caller pins that public name, so
 * the canonical function retains its address label.
 */
s8 func_80033FB8(void) {
    return D_800D2F95;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viChangeBuffers(void) {
    return D_800D2F95 != D_800D2F96;
}

/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viNoClear. No same-address Mickey caller pins that public name, so the
 * canonical function retains its address label.
 */
void func_80033FE0(void) {
    D_800D2F97 = 0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viDisplayingScreen0(void) {
    if (D_8007A690[0] == D_800D2FA8) {
        return 1;
    }
    return 0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void func_80034018(u8 *src, s32 length) {
    s32 *dest;
    s32 fourByteLength;

    dest = (s32 *) src;
    fourByteLength = length >> 2;
    if (D_8007A1A0 != 0) {
        while (fourByteLength--) {
            *dest++ = 0;
        }
    } else {
        while (fourByteLength--) {
            *dest++ = -1;
        }
    }
    osWritebackDCacheAll();
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:viGetOsViMode. */
OSViMode *func_80034094(s32 videoMode) {
    switch (videoMode) {
        case 0:
            return &D_80080530;
        case 1:
            return &osViModeNtscLan1;
        case 2:
            return &D_800805D0;
        case 3:
            return &D_80080620;
        case 4:
            return &D_800804E0;
        case 5:
            return &osViModeMpalLan1;
        case 6:
            return &D_800806C0;
        case 7:
            return &D_80080710;
        case 8:
            return &D_80080490;
        case 9:
            return &osViModePalLan1;
        case 10:
            return &D_800807B0;
        case 11:
            return &D_80080800;
        default:
            return NULL;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c:fb_swap. */
void fb_swap(void) {
    D_800D2FA8 = D_8007A690[D_800D2F94];
    D_800D2F94++;
    if (D_800D2F95) {
        if (D_800D2F94 >= 3) {
            D_800D2F94 = 0;
        }
        D_800D2FA4 = D_8007A690[D_800D2F94];
        if (D_800D2F94 < 2) {
            D_800D2FA0 = D_8007A690[D_800D2F94 + 1];
        } else {
            D_800D2FA0 = D_8007A690[0];
        }
    } else {
        if (D_800D2F94 >= 2) {
            D_800D2F94 = 0;
        }
        D_800D2FA0 = D_8007A690[D_800D2F94];
        D_800D2FA4 = D_800D2FA0;
    }
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void fb_memcpy(u8 *src, u8 *dest, s32 len) {
    while (len--) {
        *dest++ = *src++;
    }
}
