#include "overlays/overlay_039.h"

/*
 * Overlay 39, ADR 0006 consolidation: one translation unit for the whole
 * module (overlay39Write at +0x000, overlay39Reset/overlay39Read at
 * +0x0C8). DKR v77/v80 exact-object and source-signature scans are
 * negative for every function in this module.
 */

void overlay39Write(u8 *source, struct OSMesgQueue_s *mq) {
    /* Preserves the original local-slot allocation; emits no instructions. */
    volatile s32 reserved;
    u8 *buffer;
    u32 offset;
    u32 fill;

    buffer = D_8007A204;
    for (offset = 0, fill = 0xFF; ;) {
        buffer[offset++] = fill;
        if (offset >= 0x2E) {
            break;
        }
    }
    buffer[0x2E] = 0xF;
    buffer[0x2F] = 0xF;
    for (offset = 0; offset < 0x10; offset++) {
        buffer[offset + 0x30] = source[offset];
    }
    __osSiGetAccess();
    __osContLastCmd = 0xFE;
    osWritebackDCache(buffer, 0x40);
    __osSiRawStartDma(1, buffer);
    __osSiRelAccess();
}

void overlay39Reset(struct OSMesgQueue_s *mq) {
    __osSiGetAccess();
    __osContLastCmd = 0xFE;
    osInvalDCache(D_8007A204, 0x40);
    __osSiRawStartDma(0, D_8007A204);
    __osSiRelAccess();
}

void overlay39Read(u8 *destination) {
    u8 *buffer = D_8007A204;
    s32 offset;

    for (offset = 0; offset < 0x10; offset++) {
        destination[offset] = buffer[offset + 0x30];
    }
}
