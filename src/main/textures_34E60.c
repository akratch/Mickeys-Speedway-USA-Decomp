#include "PR/ultratypes.h"

/*
 * PROVENANCE: the texture-TU placement and the small mode setter below were
 * compared with Jet Force Gemini's public src/textures.c. Mickey's globals,
 * boundaries, and compiler output remain authoritative.
 */

extern s32 D_8007BD90;

typedef struct TextureRomTable {
    s32 *first;
    s32 *second;
} TextureRomTable;

extern void *mmAlloc(s32 size, s32 tag);
extern s32 *piRomLoad(s32 assetId);
extern TextureRomTable D_800D2FD0;
extern s32 *D_800D2FD8;
extern s32 *D_800D2FDC;
extern s32 D_800D2FE0;
extern s32 D_800D2FE8[2];
extern s32 D_800D2FF0;
extern s32 *D_800D2FF4;
extern s32 *D_800D2FF8;
extern s32 *D_800D2FFC;
extern s32 *D_800D3000;
extern s32 D_800D3004;
extern s32 D_800D3008;
extern s32 *D_800D301C;

void texInitTextures(void) {
    s32 i;

    D_800D2FD8 = mmAlloc(0x15E0, 0x90);
    D_800D2FDC = mmAlloc(0x280, 0x90);
    D_800D2FE0 = 0;
    D_800D2FF0 = 0;
    D_800D2FD0.first = piRomLoad(3);
    D_800D2FD0.second = piRomLoad(1);
    i = 0;
    for (; D_800D2FD0.first[i] != -1; i++) {
    }
    D_800D2FE8[0] = --i;
    for (i = 0; D_800D2FD0.second[i] != -1; i++) {
    }
    D_800D2FE8[1] = --i;
    D_800D2FFC = mmAlloc(0x320, 0x90);
    D_800D3000 = mmAlloc(0x200, 0x90);
    D_800D3008 = 0;
    D_800D2FF8 = piRomLoad(0x16);
    D_800D3004 = 0;
    while (D_800D2FF8[D_800D3004] != -1) {
        D_800D3004++;
    }
    D_800D3004--;
    D_800D301C = mmAlloc(0x28, 0x90);
    D_800D2FF4 = 0;
}

void texDisableModes(s32 flags) {
    D_8007BD90 |= flags;
}
