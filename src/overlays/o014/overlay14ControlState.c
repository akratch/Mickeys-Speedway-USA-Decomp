#include "overlays/overlay_014.h"

/* Tier B: mutable menu controls and CPU pointers written by the initializer,
 * rule loader and command updater. The address words retain their legacy
 * target-width representations; other offset-based views are still open.
 * The final eight bytes are ordinary compiler alignment, not capacity. */
s32 o14Opacity = 0;
s32 o14Active = 0;
s32 o14State = 0;
s32 o14ExitRequested = 0;
s32 o14PendingValue = -1;
s32 o14PendingMode = -1;
s32 o14Transition = 0;
s32 o14Cursor = 0;
s32 o14CommandAddress = 0;
s32 o14Mode = 3;
s32 o14ValueCount = 0;
s32 o14CommandCount = 0;
Overlay14RuleGroup * o14RuleGroups = 0;
u32 * o14Ranges = 0;
s32 o14ResultAddress = 0;
s32 o14SelectionAddress = 0;
Overlay14Owner * o14Owner = 0;
s32 o14Current = -1;
s32 o14Next = -1;
s32 o14Context = 0;
s32 o14RuleField110 = 0;
s32 o14RuleField114 = 0;
