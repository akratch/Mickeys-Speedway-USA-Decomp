/* Actual word/character state, with conservative mutable definitions.
 * Text correspondence is Tier C; original separate declarations remain Tier D.
 * This is source storage, not closure of the old imported pointer views. */
#include "PR/ultratypes.h"

s32 overlay63TransitionEnabled = 0;
s32 overlay63InputTimer = 0;
char overlay63CopyrightNotice[] = "< 2000 Disney";
char overlay63LicenseNotice[] = "Licensed to Nintendo";

/* Actual joyGetPressed mask cells. Activation starts at index1 and does not
 * validate the physical first cell. Preserve that behavior; no new guard. */
u32 overlay63ButtonSequence[] = {
    0x100, 0x800, 0x4000, 0x4000, 0x800, 0x200, 0x4000, 0x2000, 0
};
u32 *overlay63SequenceCursor = 0;
s32 overlay63SequenceIndex = 0;
char *overlay63PendingMessage = 0;
s32 overlay63MessageTimer = 0;
char *overlay63ActiveMessage = 0;

#ifdef TARGET_N64
typedef char O63StatePointerWidth[(sizeof(void *) == 4) ? 1 : -1];
typedef char O63StateWordWidth[(sizeof(s32) == 4 && sizeof(u32) == 4) ? 1 : -1];
typedef char O63SequenceSize[(sizeof(overlay63ButtonSequence) == 36) ? 1 : -1];
#endif
