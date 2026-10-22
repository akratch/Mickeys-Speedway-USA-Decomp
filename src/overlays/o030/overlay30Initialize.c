#include "PR/ultratypes.h"

/* Module initialization reconstructed from Mickey; exact DKR/JFG scans are negative. */
extern void lightDefaultObjectLight(s32, s32, s32, s32, s32);
extern void *mmAllocRegion(s32, s32);
extern void *mmAlloc(s32, s32);
extern void *piRomLoad(s32);
extern s32 piRomGetFileSize(s32);
extern void func_80004340(void);

extern void *D_800C9450;
extern s32 D_800C9454;
extern s32 *D_800C9458;
extern s32 D_800C945C;
extern void *D_800C9460;
extern s32 *gOverlay30PixelHeader;
extern s32 D_800C9468;
extern f32 D_800C946C;
extern void *D_800C9488;
extern s16 *D_800C948C;
extern void *D_800C9494;
extern void *D_800C94A0;
extern void *D_800C94A4;
extern s16 *D_800C94E0;
extern s32 gOverlay30SentinelIndex;
extern void *D_800C94EC;
extern void *D_800C94F4;
extern void *D_800C9500;

extern void overlay30TransposePixels(u8 *data, s32 length);

void overlay30Initialize(void) {
    s32 i;

    lightDefaultObjectLight(0xAA, 0x55, 0, -0x2000, 0);
    D_800C94A0 = mmAllocRegion(0x19000, 0x200);
    D_800C94EC = mmAlloc(0x320, 0x8B);
    D_800C94F4 = mmAlloc(0x20, 0x8B);
    D_800C9500 = mmAlloc(0x80, 0x8B);

    D_800C94E0 = piRomLoad(0x2E);
    gOverlay30SentinelIndex = (piRomGetFileSize(0x2E) >> 1) - 1;
    while (D_800C94E0[gOverlay30SentinelIndex] == 0) {
        gOverlay30SentinelIndex--;
    }

    D_800C9450 = mmAlloc((D_800C9454 = 0x2000), 0x8F);
    D_800C9458 = piRomLoad(0x2C);
    D_800C945C = 0;
    while (D_800C9458[D_800C945C] != -1) {
        D_800C945C++;
    }
    D_800C945C--;

    D_800C9488 = mmAlloc(D_800C945C * 4, 0x8B);
    D_800C948C = mmAlloc(D_800C945C * 2, 0x8B);
    for (i = 0; i < D_800C945C; i++) {
        D_800C948C[i] = 0;
    }

    D_800C9460 = piRomLoad(0x18);
    gOverlay30PixelHeader = piRomLoad(0x19);
    D_800C9468 = 0;
    while (gOverlay30PixelHeader[D_800C9468] != -1) {
        D_800C9468++;
    }

    overlay30TransposePixels((u8 *)D_800C9460 + (gOverlay30PixelHeader[5] * 4),
                             (gOverlay30PixelHeader[6] - gOverlay30PixelHeader[5]) * 4);
    D_800C9494 = mmAlloc(0x800, 0x8B);
    D_800C94A4 = mmAlloc(0x100, 0x8B);
    D_800C946C = 2.0f;
    func_80004340();
}
