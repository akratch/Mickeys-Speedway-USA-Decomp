#include "overlays/overlay_030.h"

/*
 * Overlay 30: one translation unit in ROM order (ADR 0006). Only
 * overlay30Initialize is exported through the ORT; its callee is static,
 * which is what makes the shipped call a JUMP record.
 */

static void overlay30TransposePixels(u8 *data, s32 length);

/* Module initialization reconstructed from Mickey; exact DKR/JFG scans are negative. */
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

/* PROVENANCE: adapted from Diddy Kong Racing, src/objects.c (decrypt_magic_codes). */
static void overlay30TransposePixels(u8 *data, s32 length) {
    s32 i;
    s32 j;
    u8 *cursor = data;
    u8 temp[4];

    for (i = 0; i < (length >> 2); i++) {
        temp[0] = (cursor[0] & 0xC0) |
                  ((cursor[1] & 0xC0) >> 2) |
                  ((cursor[2] & 0xC0) >> 4) |
                  ((cursor[3] & 0xC0) >> 6);
        temp[1] = ((cursor[0] & 0x30) << 2) |
                  (cursor[1] & 0x30) |
                  ((cursor[2] & 0x30) >> 2) |
                  ((cursor[3] & 0x30) >> 4);
        temp[2] = ((cursor[0] & 0x0C) << 4) |
                  ((cursor[1] & 0x0C) << 2) |
                  (cursor[2] & 0x0C) |
                  ((cursor[3] & 0x0C) >> 2);
        temp[3] = ((cursor[0] & 3) << 6) |
                  ((cursor[1] & 3) << 4) |
                  ((cursor[2] & 3) << 2) |
                  (cursor[3] & 3);

        for (j = 0; j < 4; j++) {
            *cursor++ = ((temp[j] & 0xAA) >> 1) |
                        ((temp[j] & 0x55) << 1);
        }
    }
}
