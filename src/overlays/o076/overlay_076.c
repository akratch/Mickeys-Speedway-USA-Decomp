#include "overlays/overlay_076.h"

/* Tier B: module-local records and section extents identify the index and eight statuses. */
static s32 sNextIndex = 0;
static s32 sStatus[8];

/*
 * Overlay 76 function map: +0x000, +0x038, +0x0D0; text ends at +0x120.
 * Fresh exact scans against DKR v77/v80 and JFG are negative. Two SYMBOL
 * relocations prove the resident calls (sound dispatch and mathRnd); twelve
 * LOCAL records prove one 16-byte data range and the 0x20-byte status BSS.
 */
void overlay76Register(Overlay76Object *object, void *entry) {
    s32 *next = &sNextIndex;
    Overlay76Record *record = object->record;

    record->index = *next;
    sStatus[*next] = 0;
    (*next)++;
}

void overlay76Update(Overlay76Object *object, s32 updateRate) {
    Overlay76Record *record = object->record;
    s32 index = record->index;
    s32 soundId;

    if (sStatus[index] == 1) {
        soundId = (index & 1) ? 0x205 : 0x206;
        amSndPlayXYZ(soundId, object->x, object->y, object->z, 4, 0);
        sStatus[record->index] = 0;
    }
}

void overlay76TriggerRandom(void) {
    s32 count = sNextIndex;

    if (count > 0) {
        sStatus[mathRnd(0, count - 1)] = 1;
    }
}
