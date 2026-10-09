#ifndef _GAME_MEMORY_H_
#define _GAME_MEMORY_H_

#include "PR/ultratypes.h"

/*
 * PROVENANCE: allocator vocabulary and the starting layouts were read from
 * Jet Force Gemini's published src/memory.h. Field widths and offsets below
 * were then checked against Mickey's own allocator accesses. JFG is a
 * permitted public retail-derived decomp under docs/CLEANROOM.md.
 */

typedef enum MemoryPoolIndex {
    MEMORY_POOL_MAIN,
    MEMORY_POOL_OBJECT,
    MEMORY_POOL_UNUSED_2,
    MEMORY_POOL_UNUSED_3,
    MEMORY_POOL_COUNT
} MemoryPoolIndex;

typedef enum MemorySlotFlags {
    MEMORY_SLOT_FREE = 0,
    MEMORY_SLOT_USED = 1,
    MEMORY_SLOT_LOCKED = 2,
    MEMORY_SLOT_SAFEGUARD = 4
} MemorySlotFlags;

typedef struct MemoryPoolSlot {
    /* 0x00 */ u8 *data;
    /* 0x04 */ s32 size;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 colourTagIndex;
    /* 0x0A */ s16 prevIndex;
    /* 0x0C */ s16 nextIndex;
    /* 0x0E */ s16 index;
    /* 0x10 */ u32 colourTag;
} MemoryPoolSlot;

typedef struct MemoryPool {
    /* 0x00 */ s16 maxNumSlots;
    /* 0x02 */ s16 curNumSlots;
    /* 0x04 */ MemoryPoolSlot *slots;
    /* 0x08 */ s32 size;
    /* 0x0C */ s32 freeSize;
} MemoryPool;

void mmInit(void);
u8 mmExtended(void);
void mmSetDelay(s32 state);
s32 mmGetDelay(void);
MemoryPoolSlot *mmGetSlotPtr(MemoryPoolIndex poolIndex);
void mmFree(void *data);
s32 mempool_slot_assign(MemoryPoolIndex poolIndex, s32 slotIndex, s32 size,
                   s32 slotIsTaken, s32 newSlotIsTaken, u32 colourTag);
void *mmAllocAtAddr(s32 size, u8 *address, u32 colourTag);
void mmFreeTick(void);
u8 *align16(u8 *address);
u8 *align8(u8 *address);
u8 *align4(u8 *address);

#endif
