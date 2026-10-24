#include "PR/ultratypes.h"

/* Tier B: the constructor and release path consume four handles; the two
 * following words are the input-delay and completion controls. Tier D:
 * original declarations/names remain inferred. The final eight bytes are
 * ordinary compiler end alignment, ending at the authenticated RO origin. */
void *gOverlay11StorageBaseHandles[4] = {0, 0, 0, 0};
s32 gOverlay11StorageInputDelay = 0;
s32 gOverlay11StorageDone = 0;
