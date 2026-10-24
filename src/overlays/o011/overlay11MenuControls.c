#include "PR/ultratypes.h"

/* Tier B: direct float/word/halfword readers and the six-handle create and
 * release paths establish these views. Only six consumed handle slots are
 * defined; no larger original allocation is claimed. All definitions are
 * conservatively mutable. Short-object spacing is ordinary compiler alignment.
 * Tier D: original declarations/names and legacy alias unification are open. */
f32 gOverlay11StorageFade = 0.0f;
s32 gOverlay11StorageGroupsActive = 0;
s16 gOverlay11StorageHighlight = 0;
s16 gOverlay11StorageHighlightStep = 6;
s32 gOverlay11StorageDataMode = 0;
s32 gOverlay11StorageAction = 0;
void *gOverlay11StorageOptionHandles[6] = {0, 0, 0, 0, 0, 0};
