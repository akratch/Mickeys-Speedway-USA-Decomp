#include "overlays/overlay019.h"

typedef struct O19BuildGroup {
    u8 pad00[0x14];
    s16 itemCount;
    u8 pad16[0x38];
    s8 gate4E;
} O19BuildGroup;

extern O19Output *mmAlloc2(s32 size, s32 tag);
extern void mmFree(void *value);
extern void *mmGetDelay(void);
extern void mmSetDelay(void *value);
extern void mmAllocAtAddr(s32 size, void *source, s32 tag);

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 19 as none. */
void overlay19BuildOutput(
    O19Context *context, O19BuildGroup *group, O19Output **outputSlot) {
    O19Output *output;
    u8 *cursor;
    void *state;
    s32 used;

    if (group->gate4E != 0 || *outputSlot != 0) {
        return;
    }

    output = mmAlloc2(0x4000, 0x8A);
    if (output == 0) {
        return;
    }

    *outputSlot = output;
    cursor = (u8 *)output + sizeof(O19Output);
    output->records = (O19AdjacencyRecord *)cursor;

    overlay19BuildAdjacency(context, (O19Group *)group, *outputSlot);
    cursor += group->itemCount * sizeof(O19AdjacencyRecord);
    (*outputSlot)->masks = (u32 *)cursor;

    overlay19BuildSpatialMasks(context, (O19Group *)group, *outputSlot);
    cursor += group->itemCount * sizeof(u32);
    (*outputSlot)->planes = (f32 *)cursor;

    cursor += overlay19BuildPlanes(
        context, (O19Group *)group, *outputSlot) * 0x10;
    used = cursor - (u8 *)output;
    if (used >= 0x4001) {
        mmFree(*outputSlot);
        *outputSlot = 0;
        return;
    }

    state = mmGetDelay();
    mmSetDelay(0);
    mmFree(output);
    mmAllocAtAddr(used, output, 0x8A);
    mmSetDelay(state);
}
