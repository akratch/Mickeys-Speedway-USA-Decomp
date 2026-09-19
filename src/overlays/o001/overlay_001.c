#include "overlays/overlay_001.h"

/* ---- overlay1PointerWrap ---- */


/* Pinned DKR/JFG scans have no exact donor; these are generic ring helpers. */
extern u8 *gOverlay1End;

u8 *overlay1PreviousPointer(u8 *pointer) {
    pointer -= 0x94;
    if (pointer < gOverlay1Start.bytes) {
        pointer = gOverlay1End;
    }
    return pointer;
}

u8 *overlay1NextPointer(u8 *pointer) {
    pointer += 0x94;
    if (pointer > gOverlay1End) {
        pointer = gOverlay1Start.bytes;
    }
    return pointer;
}

/* ---- overlay1GetEntry ---- */


/* The pinned DKR v77/v80 and JFG object scans contain no exact donor. */
Overlay1Entry *overlay1GetEntry(s32 index) {
    Overlay1Entry *entries;

    entries = gOverlay1Entries;
    return entries != NULL ? &entries[index] : NULL;
}

/* ---- overlay1GetEntryIndex ---- */


s32 overlay1GetEntryIndex(Overlay1Entry *entry) {
    return entry - gOverlay1Entries;
}

/* ---- overlay1PreviousIndex ---- */


s32 overlay1PreviousIndex(s32 index) {
    index--;
    if (index < 0) {
        index = gOverlay1EntryCount - 1;
    }
    return index;
}

/* ---- overlay1NextIndex ---- */


s32 overlay1NextIndex(s32 index) {
    index++;
    if (index >= gOverlay1EntryCount) {
        index = 0;
    }
    return index;
}

/* ---- overlay1WrapOffset ---- */


/* No corresponding DKR/JFG source or object match was found. */
extern s32 gOverlay1SegmentSize;

f32 overlay1WrapOffset(f32 a, f32 b) {
    s32 segmentSize;
    s32 halfSize;
    f32 difference;
    f32 result;

    segmentSize = gOverlay1SegmentSize;
    difference = a - b;
    halfSize = segmentSize >> 1;
    result = difference;
    if (result < (f32)-halfSize) {
        result += (f32)segmentSize;
    }
    if ((f32)halfSize < result) {
        result -= (f32)segmentSize;
    }
    return result;
}

/* ---- overlay1SignedOffset ---- */


typedef struct Overlay1OffsetState {
    u8 pad00[0x383];
    s8 cycle;
    u8 pad384[0x18];
    f32 offset;
} Overlay1OffsetState;

typedef struct Overlay1OffsetObject {
    u8 pad00[0x64];
    Overlay1OffsetState *state;
} Overlay1OffsetObject;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern s32 gOverlay1CycleLength;

f32 overlay1SignedOffset(Overlay1OffsetObject *first,
                         Overlay1OffsetObject *second) {
    Overlay1OffsetState *firstState;
    Overlay1OffsetState *secondState;
    f32 firstOffset;
    f32 secondOffset;

    firstState = first->state;
    secondState = second->state;
    firstOffset = (firstState->cycle * gOverlay1CycleLength) +
                  firstState->offset;
    secondOffset = (secondState->cycle * gOverlay1CycleLength) +
                   secondState->offset;
    return firstOffset - secondOffset;
}

/* ---- overlay1FindType47ByAngle ---- */


typedef struct Overlay1ScanData {
    u8 pad00[8];
    u16 phase;
} Overlay1ScanData;

typedef struct Overlay1ScanObject {
    u8 pad00[0x44];
    s16 type;
    u8 pad46[0x1E];
    Overlay1ScanData *data;
} Overlay1ScanObject;

extern void **func_8000572C(s32 *start, s32 *end);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1ScanLimit;

/* Indexed access lets IDO generate the cursor and retain the array-base web.
 * The phase scale is the authenticated rodata literal, not a cached global;
 * its constant web leaves the incoming angle first in the tied FP priority. */
Overlay1ScanObject *overlay1FindType47ByAngle(f32 angle) {
    s32 start;
    s32 end;
    Overlay1ScanObject **objects;
    Overlay1ScanData *data;
    Overlay1ScanObject *object;
    Overlay1ScanObject *best;
    f32 difference;
    f32 bestDifference;
    s32 index;

    objects = (Overlay1ScanObject **)func_8000572C(&start, &end);
    bestDifference = gOverlay1ScanLimit;
    best = NULL;
    index = start;
    if (start < end) {
        do {
            object = objects[index];
            if (object->type == 0x2F) {
                data = object->data;
                difference = overlay1WrapOffset(
                    (f32)data->phase * 0.1f,
                    angle);
                if ((difference > 0.0f) && (difference < bestDifference)) {
                    bestDifference = difference;
                    best = object;
                }
            }
            index++;
        } while (index < end);
    }
    return best;
}

/* ---- overlay1GetLinkedActive ---- */


typedef struct Overlay1LinkHeader {
    u8 pad0[7];
    u8 active;
} Overlay1LinkHeader;

typedef struct Overlay1LinkObject {
    u8 pad0[0x64];
    Overlay1LinkHeader *header;
} Overlay1LinkObject;

typedef struct Overlay1LinkState {
    u8 pad0[0x382];
    u8 mode;
    u8 pad383[0x11];
    Overlay1LinkObject *linked;
} Overlay1LinkState;

typedef struct Overlay1LinkOwner {
    u8 pad0[0x64];
    Overlay1LinkState *state;
} Overlay1LinkOwner;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
Overlay1LinkObject *overlay1GetLinkedActive(Overlay1LinkOwner *owner) {
    Overlay1LinkState *state;
    Overlay1LinkObject *linked;

    if (owner != NULL && (state = owner->state) != NULL &&
        ((1 << state->mode) & 0x1C) != 0 &&
        (linked = state->linked) != NULL && linked->header->active != 0) {
        return linked;
    }
    return NULL;
}

/* ---- overlay1GetRecord ---- */


/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
Overlay1Record *overlay1GetRecord(s32 index) {
    if (gOverlay1Start.records != NULL && index < gOverlay1EntryCount) {
        return &gOverlay1Start.records[index];
    }
    return NULL;
}

/* ---- overlay1FindType5ByKey ---- */


typedef struct Overlay1SearchRecord {
    u8 pad00[0x44];
    s16 type;
    u8 pad46[0x3E];
    s32 key;
} Overlay1SearchRecord;


/* Mickey-only reconstruction; the pinned DKR/JFG scans found no exact donor.
 * Evaluate the bounds before offsetting the cursor: this orders IDO's initial
 * temporary demands. The single-execution do/while and packed statement lines
 * below are inert source-shaping forms retained for exact compiler output;
 * see docs/cleanup-queue.md for the natural-spelling follow-up. */
Overlay1SearchRecord *overlay1FindType5ByKey(const s8 *key) {
  s32 start;
  s32 end;
  int typeMatches;

  Overlay1SearchRecord *record;
  Overlay1SearchRecord **cursor;
  cursor = ((Overlay1SearchRecord **) func_8000572C(&start, &end));
  if (start < end)
  {
    cursor += start; do
    {
      record = *(cursor++);
      start++;
 do { typeMatches = record->type == 5; if (typeMatches) { ; if ((*key) == (*record).key) { return record; } } } while (0);
    }
    while (end > start);
  }
  return (void *) 0;
}

/* ---- overlay1FindPreviousUsable ---- */


/* The pinned DKR v77/v80 and JFG object scans contain no exact donor. */
#ifdef NON_MATCHING
Overlay1RingRecord *overlay1FindPreviousUsable(s32 index, s32 *selectedIndex) {
    s32 count;
    s32 remaining;
    s32 wrapCount;
    Overlay1RingRecord *record;
    Overlay1RingRecord *records;
    u16 flags;

    records = gOverlay1Start.rings;
    if (records != NULL) {
        count = gOverlay1EntryCount;
        if (index < count) {
            remaining = count;
            wrapCount = count;
            if (remaining != 0) {
                remaining--;
                do {
                    index--;
                    if (index < 0) {
                        index = wrapCount - 1;
                    }
                    record = &records[index];
                    flags = record->flags;
                    if (!(flags & 4) && !(flags & 8)) {
                        *selectedIndex = index;
                        return record;
                    }
                } while (remaining--);
            }
        }
    }
    *selectedIndex = -1;
    return NULL;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001/func_overlay_001_F0000414_184C7F4.s")
#endif

/* ---- overlay1ActivateObject ---- */


typedef struct Overlay1OwnerState {
    u8 pad000[0x37C];
    s16 recordIndex;
    u8 selector;
} Overlay1OwnerState;

typedef struct Overlay1Owner {
    u8 pad000[0x64];
    Overlay1OwnerState *state;
} Overlay1Owner;

typedef struct Overlay1Sample {
    f32 x;
    f32 y;
    u8 pad008[4];
    s8 enabled;
} Overlay1Sample;

extern void *D_1D58;
extern Overlay1Sample *D_1D60;
extern Overlay1Sample *volatile D_1D68;
extern Overlay1Sample *D_1D68Read;
extern Overlay1Sample *D_1D6C;
extern Overlay1Sample *D_0208;
extern Overlay1Sample *D_020C;
extern Overlay1Sample *D_0210;
extern Overlay1Sample *D_0214;
extern s32 D_0;
extern f32 D_B0;

extern void *overlay1Chain0ContextReloc(void *source, void *context);
extern void *overlay1Chain0Reloc(void *source);
extern void *overlay1Chain40Reloc(void *source);
extern f32 overlay1InterpolateReloc(f32 first, f32 second, s32 third,
                                   s32 fourth, f32 weight);

/* Allocation residual is three words at +0x6C, +0x70 and +0x90: the address of
 * D_1D68 wants ugen's v0 and takes a scratch temp here.  Two edits took the
 * residual from 47 to 3, both read off the allocator records rather than
 * guessed: splitting the one `state` local into a pre-call `state` and a
 * post-call `current` (one web spanning the three calls is denied v0, so the
 * &D_1DA0 address could not reach v1), and dropping the `index` local so the
 * record index stays an expression temp and the &D_1D68 address becomes the
 * only value competing for a colour. */
#ifdef NON_MATCHING
s32 overlay1ActivateObject(Overlay1Owner *owner) {
    Overlay1Sample *record;
    Overlay1OwnerState *state;
    Overlay1OwnerState *current;

    D_1D9C = 0;
    D_1DA0 = 0;
    if (owner == 0) {
        return 0;
    }
    D_1D9C = owner;
    state = owner->state;
    if (state == 0) {
        return 0;
    }
    D_1DA0 = state;
    if (D_0 == 1) {
        record = (Overlay1Sample *)((u8 *)D_1D58 + (*(Overlay1OwnerState *volatile *)&D_1DA0)->recordIndex * 0x94);
        D_1D68 = record;
        D_1D64 = overlay1Chain0ContextReloc(record, &D_1D9C);
        D_1D60 = overlay1Chain0Reloc(D_1D64);
        D_1D6C = overlay1Chain40Reloc(D_1D68Read);
        current = D_1DA0;
        D_0208 =
            (Overlay1Sample *)((u8 *)D_1D60 +
                current->selector * 0x10 + 0x14);
        D_020C =
            (Overlay1Sample *)((u8 *)D_1D64 +
                current->selector * 0x10 + 0x14);
        D_0210 =
            (Overlay1Sample *)((u8 *)D_1D68Read +
                current->selector * 0x10 + 0x14);
        D_0214 =
            (Overlay1Sample *)((u8 *)D_1D6C +
                current->selector * 0x10 + 0x14);
    }
    return 1;
}

s32 overlay1FindClosestSample(f32 x, f32 y, Overlay1Sample *source,
                              f32 weight) {
    Overlay1Sample *first;
    Overlay1Sample *current;
    Overlay1Sample *third;
    Overlay1Sample *fourth;
    f32 bestDistance;
    s32 bestIndex;
    s32 index;

    bestDistance = D_B0;
    bestIndex = -1;
    first = (Overlay1Sample *)((u8 *)overlay1Chain0Reloc(source) + 0x84);
    current = (Overlay1Sample *)((u8 *)source + 0x84);
    third = (Overlay1Sample *)((u8 *)overlay1Chain40Reloc(source) + 0x84);
    fourth = (Overlay1Sample *)((u8 *)overlay1Chain40Reloc(
                                   (u8 *)third - 0x84) + 0x84);
    index = 7;
    do {
        if (current->enabled != 0) {
            f32 sampleX;
            f32 sampleY;
            f32 dx;
            f32 dy;
            f32 distance;

            sampleX = overlay1InterpolateReloc(first->x, current->x,
                                               *(s32 *)&third->x,
                                               *(s32 *)&fourth->x, weight);
            sampleY = overlay1InterpolateReloc(first->y, current->y,
                                               *(s32 *)&third->y,
                                               *(s32 *)&fourth->y, weight);
            dx = x - sampleX;
            dy = y - sampleY;
            distance = dx * dx + dy * dy;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIndex = index;
            }
        }
        first--;
        current--;
        third--;
        fourth--;
    } while (index--);
    return bestIndex;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001/func_overlay_001_F00004B4_184C894.s")
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001/func_overlay_001_F0000614_184C9F4.s")
#endif

/* ---- overlay1TestDirection ---- */


typedef struct Overlay1Direction {
    f32 x;
    u8 pad4[4];
    f32 z;
    s16 angle;
} Overlay1Direction;

extern s32 overlay1DirectionReloc(f32, f32);
extern s32 overlay1CompareDirectionReloc(s32, s16);

/* Exact at +0x758; DKR v77/v80 and JFG have no exact donor for this predicate. */
s32 overlay1TestDirection(Overlay1Direction *direction, f32 x, f32 z) {
    s32 angle;

    angle = overlay1DirectionReloc(x - direction->x, z - direction->z);
    return overlay1CompareDirectionReloc(angle, direction->angle) > 0;
}

/* PLATEAU-HANDOFF:overlay1FindPreviousUsable:start
 * symbol: overlay1FindPreviousUsable
 * score: 12 differing words
 * frame: frameless
 * relocations: 4
 * first-mismatch: +0x4
 * summary: Two-name CSE emits the delay-slot copy; pointer-form load dest is ugen, remaining folds, dead wrapCount still missing.
 * PLATEAU-HANDOFF:overlay1FindPreviousUsable:end
 */
