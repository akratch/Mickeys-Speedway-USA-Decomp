#ifndef _GAME_PI_H_
#define _GAME_PI_H_

#include "PR/ultratypes.h"

/* Cartridge asset DMA, defined in src/main/pi.c. Asset IDs and ROM offsets
 * are distinct from the caller-owned RAM buffers used by the copy APIs. */
void piInit(void);
u32 *piRomLoad(u32 assetIndex);
u8 *piRomLoadCompressed(u32 assetIndex, s32 extraMemory);
s32 piRomLoadSection(u32 assetIndex, void *address, s32 assetOffset, s32 size);
/* ROM offset value only; this does not return a RAM allocation. */
u8 *piRomGetSectionPtr(u32 assetIndex, u32 assetOffset);
s32 piRomGetFileSize(u32 assetIndex);
void romCopy(u32 romOffset, void *ramAddress, s32 numBytes);

#endif
