#include "PR/ultratypes.h"

typedef struct Overlay18Command {
    u32 w0;
    u32 w1;
} Overlay18Command;

extern const s32 gOverlay18ModeScale8[];
extern const s32 gOverlay18ModeScale10[];
extern const s32 gOverlay18ModeScale64[];
extern const s32 gOverlay18ModeScale16[];

extern s32 gOverlay18Mode;
extern s32 D_8007A1B8;
extern u8 *gOverlay18Buffers[2];
extern u8 *gOverlay18Region8[2];
extern u8 *gOverlay18Region64[2];
extern u8 *gOverlay18Region16[2];
extern u8 *gOverlay18Region8Slot0;
extern u8 *gOverlay18Region8Slot1[];
extern u8 *gOverlay18Region64Slot0;
extern u8 *gOverlay18Region64Slot1[];
extern u8 *gOverlay18Region16Slot0;
extern u8 *gOverlay18Region16Slot1[];
extern Overlay18Command *D_800CF518;
extern u8 *D_800CF530;
extern u8 *D_800CF5A0;
extern u8 *D_800CF588;
extern s32 D_800CF51C;
extern s32 D_800CF58C;
extern s32 D_800CF534;
extern s32 D_800CF5A4;

extern void *mmGetDelay(s32 mode);
extern void overlay18SelectReloc(s32 selector);
extern void mmFree(void *buffer);
extern void *mmAllocAtAddr(s32 size, void *oldBuffer, s32 tag);
extern void overlay18FinishReconfigureReloc(void *token);
extern void overlay18InitializeBuffers(void);

/*
 * PROVENANCE: Diddy Kong Racing, src/thread3_main.c
 * (alloc_displaylist_heap); adapted allocation and cursor-update structure.
 */
void overlay18Reconfigure(s32 mode) {
    s32 index;
    s32 size;
    void *token;

    if (mode != gOverlay18Mode) {
        index = mode;
        gOverlay18Mode = mode;
        token = mmGetDelay(mode);
        overlay18SelectReloc(0);

        size = (gOverlay18ModeScale8[index] * 8) +
               (gOverlay18ModeScale64[index] * 64) +
               (gOverlay18ModeScale10[index] * 10) +
               (gOverlay18ModeScale16[index] * 16);

        mmFree(gOverlay18Buffers[0]);
        mmFree(gOverlay18Buffers[1]);
        gOverlay18Buffers[0] =
            mmAllocAtAddr(size, gOverlay18Buffers[0], 0x87);
        gOverlay18Buffers[1] =
            mmAllocAtAddr(size, gOverlay18Buffers[1], 0x87);

        if ((gOverlay18Buffers[0] == 0) ||
            (gOverlay18Buffers[1] == 0)) {
            if (gOverlay18Buffers[0] != 0) {
                mmFree(gOverlay18Buffers[0]);
                gOverlay18Buffers[0] = 0;
            }
            if (gOverlay18Buffers[1] != 0) {
                mmFree(gOverlay18Buffers[1]);
                gOverlay18Buffers[1] = 0;
            }
            overlay18InitializeBuffers();
        }

        gOverlay18Region8Slot0 = gOverlay18Buffers[0] +
                                (gOverlay18ModeScale8[index] * 8);
        gOverlay18Region64Slot0 = gOverlay18Region8[0] +
                                 (gOverlay18ModeScale64[index] * 64);
        gOverlay18Region16Slot0 = gOverlay18Region64[0] +
                                 (gOverlay18ModeScale16[index] * 16);
        gOverlay18Region8Slot1[1] = gOverlay18Buffers[1] +
                                    (gOverlay18ModeScale8[index] * 8);
        gOverlay18Region64Slot1[1] = gOverlay18Region8[1] +
                                     (gOverlay18ModeScale64[index] * 64);
        gOverlay18Region16Slot1[1] = gOverlay18Region64[1] +
                                     (gOverlay18ModeScale16[index] * 16);

        D_800CF51C = gOverlay18ModeScale8[index];
        D_800CF534 = gOverlay18ModeScale64[index];
        D_800CF5A4 = gOverlay18ModeScale16[index];
        D_800CF58C = gOverlay18ModeScale10[index];
        overlay18FinishReconfigureReloc(token);
    }

    D_800CF518 =
        (Overlay18Command *)gOverlay18Buffers[D_8007A1B8];
    D_800CF530 = gOverlay18Region8[D_8007A1B8];
    D_800CF5A0 = gOverlay18Region64[D_8007A1B8];
    D_800CF588 = gOverlay18Region16[D_8007A1B8];

    {
        Overlay18Command *command;

        command = D_800CF518++;
        command->w1 = 0;
        command->w0 = 0xE9000000;
    }
    {
        Overlay18Command *command;

        command = D_800CF518++;
        command->w1 = 0;
        command->w0 = 0xB8000000;
    }
}
