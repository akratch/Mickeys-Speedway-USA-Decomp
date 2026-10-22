#ifndef OVERLAY_102_H
#define OVERLAY_102_H

#include "PR/ultratypes.h"

extern u32 *D_8007A204;

void romCopy(s32 size, void *address, s32 alignment);
s32 overlay102CheckSignature(void);

#endif
