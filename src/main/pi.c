/*
 * Cartridge asset DMA -- ROM 0x2ECA0-0x2F0D0.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/pi.c. Mickey's seven-function order and PI/DMA
 * call graph establish the correspondence. Adapted C bodies are identified in
 * docs/modules.md; all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "PR/os_pi.h"
#include "game/pi.h"

typedef struct AssetLookupTable {
    u32 fileCount;
    u32 offsets[1];
} AssetLookupTable;

extern AssetLookupTable *D_800D2470;
extern OSMesg D_800D23B8;
extern OSIoMesg D_800D23A0;
extern OSMesgQueue D_800D23C0;
extern OSMesg D_800D23D8[];
extern OSMesgQueue D_800D2458;
extern u8 D_86640[];
extern u8 D_86760[];
extern s32 D_8007A320;

void romCopy(u32 romOffset, u32 ramAddress, s32 numBytes);
void *mmAlloc();
void func_8004D5E0(OSPri priority, OSMesgQueue *queue, OSMesg *messages,
                   s32 count);
void mainPreNMI(void);
s32 byteswap32(void *address);
void func_8004D7E0(void *compressed, void *output);
void mmFree(void *address);

/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/pi.c:piInit. */
void piInit(void) {
    u32 assetTableSize;

    osCreateMesgQueue(&D_800D2458, D_800D23D8, 0x20);
    osCreateMesgQueue(&D_800D23C0, &D_800D23B8, 1);
    func_8004D5E0(0x96, &D_800D2458, D_800D23D8, 0x20);
    assetTableSize = D_86760 - D_86640;
    D_800D2470 = mmAlloc(assetTableSize, 0x84);
    romCopy((u32) D_86640, (u32) D_800D2470, assetTableSize);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomLoad. */
u32 *piRomLoad(u32 assetIndex) {
    u32 *index;
    u32 *out;
    s32 size;
    u32 start;

    if (assetIndex > D_800D2470->fileCount) {
        return NULL;
    }
    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0];
    size = index[1] - start;
    if (size == 0) {
        return NULL;
    }
    out = mmAlloc(size, 0x84);
    if (out == NULL) {
        return NULL;
    }
    romCopy((u32) (start + D_86760), (u32) out, size);
    return out;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/pi.c:piRomLoadCompressed. */
u8 *piRomLoadCompressed(u32 assetIndex, s32 extraMemory) {
    s32 size;
    s32 start;
    s32 totalSpace;
    u8 *gzipHeaderRamPos;
    u8 *out;

    if (assetIndex > D_800D2470->fileCount) {
        return NULL;
    }
    assetIndex++;
    out = (u8 *) (assetIndex + D_800D2470->offsets - 1);
    start = ((s32 *) out)[0];
    size = ((s32 *) out)[1] - start;
    gzipHeaderRamPos = mmAlloc(8, 0x84);
    romCopy((u32) (start + D_86760), (u32) gzipHeaderRamPos, 8);
    totalSpace = byteswap32(gzipHeaderRamPos) + extraMemory;
    mmFree(gzipHeaderRamPos);
    out = mmAlloc(totalSpace + extraMemory, 0x84);
    if (out == NULL) {
        return NULL;
    } else {
        /* The donors (JFG, DKR) write an empty `if (1) {}` after this
         * assignment. The else arm gives it the same block of its own, which
         * is what lets IDO compute the copy target straight into a1. */
        gzipHeaderRamPos = (out + totalSpace) - size;
    }
    romCopy((u32) (start + D_86760), (u32) gzipHeaderRamPos, size);
    func_8004D7E0(gzipHeaderRamPos, out);
    return out;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomLoadSection. */
s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size) {
    u32 *index;
    s32 start;

    if (size == 0 || D_800D2470->fileCount < assetIndex) {
        return 0;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0] + assetOffset;
    romCopy((u32) (start + D_86760), address, size);
    return size;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomGetSectionPtr. */
u8 *piRomGetSectionPtr(u32 assetIndex, u32 assetOffset) {
    u32 *index;
    u32 start;

    if (assetIndex > D_800D2470->fileCount) {
        return NULL;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0] + assetOffset;
    return start + D_86760;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomGetFileSize. */
s32 piRomGetFileSize(u32 assetIndex) {
    u32 *index;
    s32 size;

    if (assetIndex > D_800D2470->fileCount) {
        return 0;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    size = index[1] - index[0];
    return size;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/pi.c:romCopy, with Mickey's 0x400-byte transfer chunks. */
void romCopy(u32 romOffset, u32 ramAddress, s32 numBytes) {
    OSMesg dmaMessage;
    s32 transferSize;

    osInvalDCache((void *) ramAddress, numBytes);
    transferSize = 0x400;
    while (numBytes > 0) {
        if (numBytes < transferSize) {
            transferSize = numBytes;
        }
        osPiStartDma(&D_800D23A0, OS_MESG_PRI_NORMAL, OS_READ, romOffset,
                     (void *) ramAddress, transferSize, &D_800D23C0);
        osRecvMesg(&D_800D23C0, &dmaMessage, OS_MESG_BLOCK);
        if (D_8007A320 != 0) {
            mainPreNMI();
        }
        numBytes -= transferSize;
        romOffset += transferSize;
        ramAddress += transferSize;
    }
}
