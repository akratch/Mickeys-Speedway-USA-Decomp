#ifndef OVERLAY_030_H
#define OVERLAY_030_H

#include "PR/ultratypes.h"
#include "game/lights.h"
#include "game/memory.h"

extern MemoryPoolSlot *mmAllocRegion(s32, s32);
extern void *mmAlloc(s32, u32);
extern u32 *piRomLoad(u32);
extern s32 piRomGetFileSize(u32);
extern void func_80004340(void);

extern void *D_800C9450;
extern s32 D_800C9454;
extern s32 *D_800C9458;
extern s32 D_800C945C;
extern void *D_800C9460;
extern s32 *D_800C9464;
extern s32 D_800C9468;
extern f32 D_800C946C;
extern void *D_800C9488;
extern s16 *D_800C948C;
extern void *D_800C9494;
extern void *D_800C94A0;
extern void *D_800C94A4;
extern s16 *D_800C94E0;
extern s32 D_800C94E4;
extern void *D_800C94EC;
extern void *D_800C94F4;
extern void *D_800C9500;

#endif
