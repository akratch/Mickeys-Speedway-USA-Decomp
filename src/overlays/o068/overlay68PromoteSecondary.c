#include "PR/ultratypes.h"

typedef struct Overlay68Record {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay68Record;

typedef struct Overlay68Entry {
    void *object;
    s8 kind;
    s8 field5;
    s16 generation;
    s16 timer;
    s16 recordCount;
    Overlay68Record *records;
} Overlay68Entry;

extern Overlay68Entry *gOverlay68PrimaryEntry;
extern Overlay68Entry *gOverlay68SecondaryEntry;
extern void *gOverlay68Tertiary;

extern void overlay68ClearNestedFlagPromoteReloc(void *entryOrHandle);
extern void overlay68FinishEntryPromoteReloc(void);

/* Matched 2026-08-29 by a bounded decomp-permuter pass after the prior hand,
 * flag, and allocator-trace probes plateaued on the frame and stack homes.
 * Untouched IDO output preserves the 77-word body, 0x30 frame, and nine
 * relocation sites; the complete linked US ROM is byte-identical.
 *
 * `primaryCopy` (the permuter's `new_var`) is never read, but the chained
 * assignment is load-bearing (lane c-5, 2026-10-09): a plain
 * `primary = gOverlay68PrimaryEntry;` moves the stack homes (10 words), with
 * or without a pad in its place, and testing and passing `primary` instead of
 * re-reading the global differs in 14. */
void overlay68PromoteSecondary(void)
{
  Overlay68Entry *primary;
  Overlay68Record *source;
  Overlay68Record *destination;
  Overlay68Entry *secondary;
  Overlay68Entry *primaryCopy;
  s32 remaining;
  secondary = gOverlay68SecondaryEntry;
  if (secondary != ((void *) 0))
  {
    primary = (primaryCopy = gOverlay68PrimaryEntry);
    if (gOverlay68PrimaryEntry != ((void *) 0))
    {
      overlay68ClearNestedFlagPromoteReloc(gOverlay68PrimaryEntry);
      overlay68ClearNestedFlagPromoteReloc(gOverlay68Tertiary);
      if (secondary->field5 != 0)
      {
        overlay68FinishEntryPromoteReloc();
      }
      if (((secondary->timer != 0) && (secondary->object == ((void *) 0))) && ((secondary->timer < primary->timer) || (secondary->generation != primary->generation)))
      {
        primary->kind = secondary->kind;
        primary->generation = secondary->generation;
        primary->timer = secondary->timer;
        primary->recordCount = secondary->recordCount;
        remaining = secondary->recordCount;
        source = secondary->records;
        destination = primary->records;
        while (remaining--)
        {
          destination->red = source->red;
          destination->green = source->green;
          destination->blue = source->blue;
          destination->x = source->x;
          destination->y = source->y;
          destination->z = source->z;
          destination->alpha = source->alpha;
          source++;
          destination++;
        }

        secondary->timer = 0;
      }
    }
  }
}
