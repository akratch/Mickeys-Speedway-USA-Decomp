#ifndef OVERLAYS_OVERLAY1_ACTION_MODES_H
#define OVERLAYS_OVERLAY1_ACTION_MODES_H

#include <PR/ultratypes.h>

/* Current source-owned action records; original capacity and TU remain Tier D. */
#define O1_ACTION_MODE_COUNT 6

typedef struct O1ActionModeData {
    s32 (*test)(void);
    void (*update)(void);
    u16 mask;
} O1ActionModeData;

/* Type only: the producer's closed source proof requires one definition and
 * no duplicate extern declaration of that array in its effective input. */

#endif
