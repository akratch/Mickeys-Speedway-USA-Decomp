#include "PR/ultratypes.h"
#include "PR/os_pi.h"

extern OSPiHandle D_800D7760;
extern u32 D_800D77D8;

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashallerase.c:osFlashAllErase. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
s32 osFlashAllErase(void) {
    s32 status;

    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0x3C000000);
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0x78000000);
    do {
        osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    } while ((status & 2) == 2);
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 8 || (status & 0xFF) == 0x48 || (status & 8) == 8) {
        return 0;
    }
    return -1;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashsectorerase.c:osFlashSectorErase. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
s32 osFlashSectorErase(u32 page_num) {
    s32 status;

    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000,
                 0x4B000000 | page_num);
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0x78000000);
    do {
        osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    } while ((status & 2) == 2);
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 8 || (status & 0xFF) == 0x48 || (status & 8) == 8) {
        return 0;
    }
    return -1;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashsectorerase.c:osFlashWriteBuffer. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
s32 osFlashWriteBuffer(OSIoMesg *mb, s32 priority, void *dramAddr,
                       OSMesgQueue *mq) {
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0xB4000000);
    mb->hdr.pri = priority;
    mb->hdr.retQueue = mq;
    mb->dramAddr = dramAddr;
    mb->devAddr = 0;
    mb->size = 0x80;
    return osEPiStartDma(&D_800D7760, mb, OS_WRITE);
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashsectorerase.c:osFlashWriteArray. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
s32 osFlashWriteArray(u32 page_num) {
    s32 status;

    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000,
                 0xA5000000 | page_num);
    do {
        osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    } while ((status & 1) == 1);
    status = 0xFFFFFF;
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 4 || (status & 0xFF) == 0x44 || (status & 4) == 4) {
        return 0;
    }
    return -1;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashreadarray.c:osFlashReadArray. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
s32 osFlashReadArray(OSIoMesg *mb, s32 priority, u32 page_num, void *dramAddr,
                     u32 n_pages, OSMesgQueue *mq) {
    u32 ret;
    u32 tmp;

    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0xF0000000);
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &tmp);
    mb->hdr.pri = priority;
    mb->hdr.retQueue = mq;
    mb->dramAddr = dramAddr;
    mb->devAddr = page_num * D_800D77D8;
    mb->size = n_pages << 7;
    ret = osEPiStartDma(&D_800D7760, mb, OS_READ);
    return ret;
}
