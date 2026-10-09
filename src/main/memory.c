/*
 * Resident allocator -- ROM 0x2BCD0-0x2C8C0 (VRAM 0x8002B0D0).
 *
 * The linked pre-split object owns exactly 0xBF0 bytes. The last function ends
 * at ROM 0x2C8B4 and the final 0xC bytes are 16-byte alignment padding; the
 * next linked object begins at ROM 0x2C8C0.
 *
 * PROVENANCE: the candidate names recorded beside the still-unmatched stubs
 * were read from Jet Force Gemini's published src/memory.c and src/memory.h.
 * They are not adopted as Mickey symbols merely from source order. JFG is a
 * permitted public retail-derived decomp under docs/CLEANROOM.md.
 */

#include "game/memory.h"

extern u8 D_8007A274;
MemoryPool D_800D1C60[MEMORY_POOL_COUNT];
s32 D_800D1CA0;
s32 sMemoryPoolCountPadding;
void *D_800D1CA8[256];
u8 D_800D20A8[256];
s32 D_800D21A8;
s32 D_800D21AC;
s32 D_800D21B0;
s32 D_800D21B4;
#define D_800D1C64 (D_800D1C60[0].slots)
extern u8 D_800D8750[];

MemoryPoolSlot *mempool_init(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots);

/* PROVENANCE: adapted from JFG src/memory.c:mmInit. */
void mmInit(void) {
    D_800D1CA0 = -1;
    if (D_8007A274) {
        D_800D21B4 = 0x80600000;
    } else {
        D_800D21B4 = 0x80400000;
    }
    mempool_init((MemoryPoolSlot *)D_800D8750,
                  D_800D21B4 - (s32)D_800D8750, 0x640);
    mmSetDelay(2);
    D_800D21A8 = 0;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmExtended. */
u8 mmExtended(void) {
    return D_8007A274;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAllocRegion. */
void *mmAlloc(s32 size, u32 colourTag);

MemoryPoolSlot *mmAllocRegion(s32 poolDataSize, s32 numSlots) {
    s32 size;
    MemoryPoolSlot *slots;
    s32 pad;
    MemoryPoolSlot *newPool;

    size = poolDataSize + (numSlots * sizeof(MemoryPoolSlot));
    slots = (MemoryPoolSlot *)mmAlloc(size, 0x95);
    newPool = mempool_init(slots, size, numSlots);
    return newPool;
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_init. */
MemoryPoolSlot *mempool_init(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots) {
    MemoryPoolSlot *firstSlot;
    s32 poolCount;
    s32 i;
    s32 firstSlotSize;

    poolCount = ++D_800D1CA0;
    firstSlotSize = poolSize - (numSlots * sizeof(MemoryPoolSlot));
    D_800D1C60[poolCount].maxNumSlots = numSlots;
    D_800D1C60[poolCount].curNumSlots = 0;
    D_800D1C60[poolCount].slots = slots;
    D_800D1C60[poolCount].size = poolSize;
    D_800D1C60[poolCount].freeSize = firstSlotSize;
    firstSlot = slots;
    for (i = 0; i < D_800D1C60[poolCount].maxNumSlots; i++) {
        firstSlot->index = i;
        firstSlot++;
    }
    firstSlot = &D_800D1C60[poolCount].slots[0];
    slots += numSlots;
    if ((s32)slots & 0xF) {
        firstSlot->data = (u8 *)(((s32)slots & ~0xF) + 0x10);
    } else {
        firstSlot->data = (u8 *)slots;
    }
    firstSlot->size = firstSlotSize;
    firstSlot->flags = MEMORY_SLOT_FREE;
    firstSlot->colourTagIndex = 0x95;
    firstSlot->prevIndex = -1;
    firstSlot->nextIndex = -1;
    D_800D1C60[poolCount].curNumSlots++;
    if (poolCount == MEMORY_POOL_MAIN) {
        D_800D21B0 = firstSlotSize;
    }
    return D_800D1C60[poolCount].slots;
}

extern s32 D_8007A270;
extern s32 D_8007A278;
extern s32 D_8007A27C;

s32 runlinkGetAddressInfo(u32 address, s32 *moduleId, s32 *moduleAddress, u32 **symbolName);
void *mempool_slot_find(MemoryPoolIndex poolIndex, s32 size, u32 colourTag);

/* PROVENANCE: adapted from JFG src/memory.c:mmAlloc. */
void *mmAlloc(s32 size, u32 colourTag) {
    struct {
        volatile s32 address;
        s32 moduleAddress;
        s32 moduleId;
        s32 pad;
    } stack;

    stack.address = 0x666;
    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(stack.address - 8, &stack.moduleId, &stack.moduleAddress, NULL);
        colourTag = (stack.moduleId << 24) | stack.moduleAddress;
    }
    return mempool_slot_find(MEMORY_POOL_MAIN, size, colourTag);
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAlloc2. */
void *mmAlloc2(s32 size, u32 colourTag) {
    struct {
        volatile s32 address;
        s32 moduleAddress;
        s32 moduleId;
        s32 pad;
    } stack;

    stack.address = 0x666;
    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(stack.address - 8, &stack.moduleId, &stack.moduleAddress, NULL);
        colourTag = (stack.moduleId << 24) | stack.moduleAddress;
    }
    return mempool_slot_find(MEMORY_POOL_MAIN, size, colourTag);
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_slot_find. */
void *mempool_slot_find(MemoryPoolIndex poolIndex, s32 size, u32 colourTag) {
    s32 slotSize;
    MemoryPoolSlot *slot;
    volatile s32 pad;
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    s16 nextIndex;
    s32 currIndex;

    pool = &D_800D1C60[poolIndex];
    if (pool->maxNumSlots == pool->curNumSlots + 1) {
        return NULL;
    }
    currIndex = -1;
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }
    slotSize = 0x7FFFFFFF;
    slots = pool->slots;
    nextIndex = 0;
    do {
        slot = (MemoryPoolSlot *)((u8 *)slots + (nextIndex << 4) + (nextIndex << 2));
        if (slot->flags == MEMORY_SLOT_FREE) {
            if (slot->size >= size && slot->size < slotSize) {
                slotSize = slot->size;
                currIndex = nextIndex;
            }
        }
        nextIndex = slot->nextIndex;
    } while (nextIndex != -1);

    if (currIndex != -1) {
        mempool_slot_assign(poolIndex, currIndex, size, TRUE, FALSE, colourTag);
        return (currIndex + slots)->data;
    }
    return NULL;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAllocR. */
void *mmAllocR(MemoryPoolSlot *slots, s32 size) {
    s32 i;

    for (i = D_800D1CA0; i != 0; i--) {
        if (slots == D_800D1C60[i].slots) {
            return mempool_slot_find(i, size, 0);
        }
    }
    return NULL;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAllocAtAddr. Mickey's globals,
 * pool/slot layouts, absent diagnostic calls, and linked bytes are authoritative. */
/*
 * Matched 2026-09-11 by deleting the `data` carrier and writing its three uses
 * as unsigned arithmetic on the field itself.  The last two words were never an
 * allocator question: every declared local consumes a four-byte cell from the
 * frame top downward in declaration order -- register class, unreferenced and
 * block-scoped locals included -- and the compiler temporary area sits below
 * the last cell, so a seventh local pushed the slot-pointer save one word low.
 * The carrier existed only to split the field's single web into the ROM's two
 * live ranges; spelling the two range tests and the split-call offset in `u32`
 * does the same split through two expression webs instead, which costs no cell.
 * The equality test and the direct return keep reading the field as a pointer,
 * which is what keeps those two webs apart.
 */
void *mmAllocAtAddr(s32 size, u8 *address, u32 colourTag) {
    s32 slotIndex;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *slots;
    s32 moduleId;
    s32 moduleAddress;
    volatile s32 callerAddress = 0x666;

    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(callerAddress - 8, &moduleId, &moduleAddress, NULL);
        colourTag = (moduleId << 24) | moduleAddress;
    }

    if (D_800D1C60[MEMORY_POOL_MAIN].curNumSlots + 1 ==
        D_800D1C60[MEMORY_POOL_MAIN].maxNumSlots) {
        return NULL;
    }
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }

    slots = D_800D1C60[MEMORY_POOL_MAIN].slots;
    for (slotIndex = 0; slotIndex != -1; slotIndex = slot->nextIndex) {
        slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
        if (slot->flags == MEMORY_SLOT_FREE) {
            if ((u32)address >= (u32)slot->data &&
                (u32)address + size <= (u32)slot->data + slot->size) {
                if (address == slot->data) {
                    mempool_slot_assign(MEMORY_POOL_MAIN, slotIndex, size, TRUE, FALSE, colourTag);
                    return slot->data;
                }
                slotIndex = mempool_slot_assign(MEMORY_POOL_MAIN, slotIndex,
                                          (u32)address - (u32)slot->data,
                                          FALSE, TRUE, colourTag);
                mempool_slot_assign(MEMORY_POOL_MAIN, slotIndex, size, TRUE, FALSE, colourTag);
                return *(u8 **)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
            }
        }
    }

    return NULL;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmSetDelay. */
void mmSetDelay(s32 state) {
    D_800D21AC = state;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmFlushFreeStack. */
void mempool_free_addr(u8 *address);

void mmFlushFreeStack(void) {
    while (D_800D21A8 > 0) {
        mempool_free_addr(D_800D1CA8[--D_800D21A8]);
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mmFree. */
void mempool_free_queue(void *dataAddress);

void mmFree(void *data) {
    volatile s32 callerAddress = 0x666;

    if (D_800D21AC == 0) {
        mempool_free_addr(data);
    } else {
        mempool_free_queue(data);
    }
}

/*
 * PROVENANCE: adapted from JFG src/memory.c:mmFreeTick. Mickey's low-memory
 * link-slot release and absent diagnostic print are authoritative differences.
 */
void ReleaseUnusedLinkSlots(void);

/*
 * The second low-memory tier is JFG's: `mmFreeTick` re-reads FreeRAM under a
 * 0xC000 guard and calls into a module Mickey never links, so the guarded
 * block is empty here. The read is not decoration -- referencing D_800D21B0
 * twice is what makes IDO materialize its address into a callee-saved register
 * and load through it (`lui`/`addiu`/`lw 0(reg)`), where a single reference
 * folds the %lo into the load and loses one instruction. Recorded in
 * docs/cleanup-queue.md; seek a natural spelling with the same 63 words.
 *
 * The counter reset is on its own line, not in a `for` header, so `as1`
 * schedules it ahead of the loop preheader's hoisted D_800D20A8 address
 * instead of into the guard's delay slot (docs/ido-learnings.md, the source
 * line stamped on each emitted record).
 */
void mmFreeTick(void) {
    s32 i;

    if (D_800D21B0 < 0x14000) {
        ReleaseUnusedLinkSlots();
        if (D_800D21B0 < 0xC000) {
        }
    }
    i = 0;
    while (i < D_800D21A8) {
        D_800D20A8[i]--;
        if (D_800D20A8[i] == 0) {
            mempool_free_addr(D_800D1CA8[i]);
            D_800D1CA8[i] = D_800D1CA8[D_800D21A8 - 1];
            D_800D20A8[i] = D_800D20A8[D_800D21A8 - 1];
            D_800D21A8--;
        } else {
            i++;
        }
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_free_addr. */
s32 mempool_get_pool(u8 *address);
void mempool_slot_clear(MemoryPoolIndex poolIndex, s32 slotIndex);

void mempool_free_addr(u8 *address) {
    s16 slotIndex;
    s32 poolIndex;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;

    poolIndex = mempool_get_pool(address);
    slots = *(MemoryPoolSlot **)((u8 *)&D_800D1C64 + (poolIndex << 4));
    for (slotIndex = 0; slotIndex != -1; slotIndex = slot->nextIndex) {
        slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
        if (address == slot->data) {
            if (slot->flags == MEMORY_SLOT_USED || slot->flags == MEMORY_SLOT_SAFEGUARD) {
                mempool_slot_clear(poolIndex, slotIndex);
            }
            break;
        }
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_free_queue. */
void mempool_free_queue(void *dataAddress) {
    D_800D1CA8[D_800D21A8] = dataAddress;
    D_800D20A8[D_800D21A8] = D_800D21AC;
    D_800D21A8++;
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_get_pool. */
s32 mempool_get_pool(u8 *address) {
    s32 i;
    MemoryPool *pool;

    for (i = D_800D1CA0; i > 0; i--) {
        pool = &D_800D1C60[i];
        if ((u8 *)pool->slots >= address) {
            continue;
        }
        if (address < pool->size + (u8 *)pool->slots) {
            break;
        }
    }
    return i;
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_slot_clear. */
void mempool_slot_clear(MemoryPoolIndex poolIndex, s32 slotIndex) {
    s16 nextIndex;
    s16 prevIndex;
    s16 tempNextIndex;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *nextSlot;
    MemoryPoolSlot *prevSlot;

    slots = D_800D1C60[poolIndex].slots;
    slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
    nextIndex = slot->nextIndex;
    prevIndex = slot->prevIndex;
    nextSlot = (MemoryPoolSlot *)((u8 *)slots + (nextIndex << 4) + (nextIndex << 2));
    prevSlot = (MemoryPoolSlot *)((u8 *)slots + (prevIndex << 4) + (prevIndex << 2));
    slot->flags = MEMORY_SLOT_FREE;
    if (poolIndex == MEMORY_POOL_MAIN) {
        D_800D21B0 += slot->size;
    }
    D_800D1C60[poolIndex].freeSize += slot->size;
    if (nextIndex != -1 && nextSlot->flags == MEMORY_SLOT_FREE) {
        slot->size += nextSlot->size;
        tempNextIndex = nextSlot->nextIndex;
        slot->nextIndex = tempNextIndex;
        if (tempNextIndex != -1) {
            ((MemoryPoolSlot *)((u8 *)slots + (tempNextIndex << 4) +
                                (tempNextIndex << 2)))->prevIndex = slotIndex;
        }
        D_800D1C60[poolIndex].curNumSlots--;
        slots[D_800D1C60[poolIndex].curNumSlots].index = nextIndex;
    }
    if (prevIndex != -1 && prevSlot->flags == MEMORY_SLOT_FREE) {
        prevSlot->size += slot->size;
        tempNextIndex = slot->nextIndex;
        prevSlot->nextIndex = tempNextIndex;
        if (tempNextIndex != -1) {
            ((MemoryPoolSlot *)((u8 *)slots + (tempNextIndex << 4) +
                                (tempNextIndex << 2)))->prevIndex = prevIndex;
        }
        D_800D1C60[poolIndex].curNumSlots--;
        slots[D_800D1C60[poolIndex].curNumSlots].index = slotIndex;
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mmGetSlotPtr. */
MemoryPoolSlot *mmGetSlotPtr(MemoryPoolIndex poolIndex) {
    return *(MemoryPoolSlot **) ((u8 *) &D_800D1C64 + (poolIndex * sizeof(MemoryPool)));
}

/* PROVENANCE: adapted from JFG src/memory.c:mmGetDelay. */
s32 mmGetDelay(void) {
    return D_800D21AC;
}

/*
 * PROVENANCE: adapted from JFG src/memory.c:mempool_slot_assign. Mickey's
 * pool accounting, byte-sized slot fields, globals, and bytes are authoritative.
 * Canonical -O2/-mips2 C is exact for all 72 frameless words and all eight
 * relocation tuples. Reusing dead incoming/local carriers preserves the
 * allocator's slot-count and remainder-link webs without artificial code.
 */
s32 mempool_slot_assign(MemoryPoolIndex poolIndex, s32 slotIndex, s32 size,
                   s32 slotIsTaken, s32 newSlotIsTaken, u32 colourTag) {
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;
    s32 index;
    s32 slotSize;

    if (slotIsTaken == TRUE) {
        if (poolIndex == MEMORY_POOL_MAIN) {
            D_800D21B0 -= size;
        }
        pool = (MemoryPool *)((u8 *)D_800D1C60 + (poolIndex << 4));
        pool->freeSize -= size;
    }

    pool = (MemoryPool *)((u8 *)D_800D1C60 + (poolIndex << 4));
    slots = pool->slots;
    slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
    slot->flags = slotIsTaken;
    slot->colourTagIndex = D_8007A270;
    slotSize = slot->size;
    slot->size = size;
    slot->colourTag = colourTag;
    if (size < slotSize) {
        slotIsTaken = pool->curNumSlots;
        index = ((MemoryPoolSlot *)((slotIsTaken * sizeof(MemoryPoolSlot)) +
                                    (u8 *)slots))->index;
        pool->curNumSlots = slotIsTaken + 1;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->data =
            slot->data + size;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->size =
            slotSize - size;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->flags =
            newSlotIsTaken;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) +
                            (index << 2)))->colourTagIndex = D_8007A270;
        {
            slotSize = slot->nextIndex;

            ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->prevIndex =
                slotIndex;
            ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->nextIndex =
                slotSize;
            slot->nextIndex = index;
            if (slotSize != -1) {
                ((MemoryPoolSlot *)((u8 *)slots + (slotSize << 4) +
                                    (slotSize << 2)))->prevIndex = index;
            }
        }
        return index;
    }
    return slotIndex;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAlign16. */
u8 *align16(u8 *address) {
    s32 remainder = (s32) address & 0xF;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 16);
    }
    return address;
}

/* PROVENANCE: derived from JFG src/memory.c's mmAlign16/mmAlign4 family. */
u8 *align8(u8 *address) {
    s32 remainder = (s32) address & 7;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 8);
    }
    return address;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAlign4. */
u8 *align4(u8 *address) {
    s32 remainder = (s32) address & 3;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 4);
    }
    return address;
}


