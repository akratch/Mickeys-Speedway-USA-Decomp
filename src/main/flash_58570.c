#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_pi.h"
#include "PR/rcp.h"

extern OSPiHandle D_800D7760;
extern u32 D_800D7720[4];
extern OSIoMesg D_800D7730;
extern OSMesgQueue D_800D7748;
extern u8 D_800D7774[0x60];
extern OSMesg D_800D77D4[1];
extern u32 D_800D77D8;

void osFlashReadId(s32 *flash_type, s32 *flash_maker);

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashreinit.c. Mickey's handler symbol and output remain
 * authoritative. */
OSPiHandle *osFlashReInit(u8 latency, u8 pulse, u8 pageSize, u8 relDuration,
                          u32 start) {
    D_800D7760.baseAddress = PHYS_TO_K1(start);
    D_800D7760.type++;
    D_800D7760.latency = latency;
    D_800D7760.pulse = pulse;
    D_800D7760.pageSize = pageSize;
    D_800D7760.relDuration = relDuration;
    D_800D7760.domain = PI_DOMAIN2;

    return &D_800D7760;
}

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashinit.c:osFlashInit. Mickey's symbol, linked bytes,
 * and relocations remain authoritative. */
OSPiHandle *osFlashInit(void) {
    s32 flash_type;
    s32 flash_maker;

    osCreateMesgQueue(&D_800D7748, D_800D77D4, 1);

    if (D_800D7760.baseAddress == 0xA8000000) {
        return &D_800D7760;
    }

    D_800D7760.type = 8;
    D_800D7760.baseAddress = 0xA8000000;
    D_800D7760.latency = 5;
    D_800D7760.pulse = 0xC;
    D_800D7760.pageSize = 0xF;
    D_800D7760.relDuration = 2;
    D_800D7760.domain = 1;
    D_800D7760.speed = 0;
    _bzero(D_800D7774, sizeof(__OSTranxInfo));
    osEPiLinkHandle(&D_800D7760);
    osFlashReadId(&flash_type, &flash_maker);

    if (flash_maker == 0xC2001E || flash_maker == 0xC20001 ||
        flash_maker == 0xC20000) {
        D_800D77D8 = 0x40;
    } else {
        D_800D77D8 = 0x80;
    }

    return &D_800D7760;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashreadstatus.c:osFlashReadStatus. Mickey's symbol,
 * linked bytes, and relocations remain authoritative. */
void osFlashReadStatus(u8 *flash_status) {
    u32 status;

    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0xD2000000);
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0xD2000000);
    osEPiReadIo(&D_800D7760, D_800D7760.baseAddress, &status);
    *flash_status = status & 0xFF;
}

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashreadid.c:osFlashReadId. Mickey's symbol, linked
 * bytes, and relocations remain authoritative. */
void osFlashReadId(s32 *flash_type, s32 *flash_maker) {
    u8 tmp;

    osFlashReadStatus(&tmp);
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | 0x10000, 0xE1000000);
    D_800D7730.hdr.pri = 0;
    D_800D7730.hdr.retQueue = &D_800D7748;
    D_800D7730.dramAddr = D_800D7720;
    D_800D7730.devAddr = 0;
    D_800D7730.size = 2 * sizeof(u32);
    osWritebackDCache(D_800D7720, 0x10);
    osEPiStartDma(&D_800D7760, &D_800D7730, OS_READ);
    osRecvMesg(&D_800D7748, NULL, OS_MESG_BLOCK);
    *flash_type = D_800D7720[0];
    *flash_maker = D_800D7720[1];
}
