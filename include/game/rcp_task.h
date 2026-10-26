#ifndef GAME_RCP_TASK_H
#define GAME_RCP_TASK_H

#include "PR/ultratypes.h"

/* Submit a mutable command-buffer byte span; dataEnd is exclusive.
 * Both boundaries belong to the same live backing object and are eight-byte
 * aligned. The nonnegative byte distance must be representable and divisible
 * by eight. The task stores a count of eight-byte words, not a byte count.
 * Valid task types are 0, 3, 4 and 5. Completion and resource lifetime remain
 * caller obligations; this interface does not validate capacity or wait.
 */
s32 rcpFast3d(void *dataStart, void *dataEnd, s32 taskType,
              void *framebuffer);

#endif
