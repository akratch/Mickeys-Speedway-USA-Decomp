#include "PR/ultratypes.h"

extern const s32 gOverlay18ModeScale8[];
extern const s32 gOverlay18ModeScale10[];
extern const s32 gOverlay18ModeScale64[];
extern const s32 gOverlay18ModeScale16[];
extern s32 gOverlay18Mode;
extern u8 *D_800CF510[2];
extern u8 *D_800CF528[2];
extern u8 *D_800CF580[2];
extern u8 *D_800CF598[2];
extern s32 D_800CF51C;
extern s32 D_800CF534;
extern s32 D_800CF58C;
extern s32 D_800CF5A4;

extern void *mmAlloc(s32 size, s32 tag);

/* PROVENANCE: adapted from Diddy Kong Racing src/thread3_main.c
 * (default_alloc_displaylist_heap); Mickey's globals and bytes are authoritative. */
void overlay18InitializeBuffers(void) {
    s32 mode;
    s32 size;

    mode = 3;
    gOverlay18Mode = mode;
    size = (gOverlay18ModeScale8[mode] * 8) +
           (gOverlay18ModeScale64[mode] * 64) +
           (gOverlay18ModeScale10[mode] * 10) +
           (gOverlay18ModeScale16[mode] * 16);
    D_800CF510[0] = mmAlloc(size, 0x87);
    D_800CF528[0] = D_800CF510[0] +
                           (gOverlay18ModeScale8[mode] * 8);
    D_800CF580[0] = D_800CF528[0] +
                           (gOverlay18ModeScale64[mode] * 64);
    D_800CF598[0] = D_800CF580[0] +
                           (gOverlay18ModeScale10[mode] * 10);

    D_800CF510[1] = mmAlloc(size, 0x87);
    D_800CF528[1] = D_800CF510[1] +
                           (gOverlay18ModeScale8[mode] * 8);
    D_800CF580[1] = D_800CF528[1] +
                           (gOverlay18ModeScale64[mode] * 64);
    D_800CF598[1] = D_800CF580[1] +
                           (gOverlay18ModeScale10[mode] * 10);

    D_800CF51C = gOverlay18ModeScale8[mode];
    D_800CF534 = gOverlay18ModeScale64[mode];
    D_800CF5A4 = gOverlay18ModeScale16[mode];
    D_800CF58C = gOverlay18ModeScale10[mode];
}
