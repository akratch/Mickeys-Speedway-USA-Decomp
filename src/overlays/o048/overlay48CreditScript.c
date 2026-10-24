#include "PR/ultratypes.h"

/* Tier B: the update loop consumes a variable-length signed-halfword stream.
 * LOAD waits until prior fades settle, then takes a count and lifetime; HOLD
 * and RELEASE each take one entry index. FINISH starts the title transition.
 * Names describe those actual consumers, not original source spelling. */
enum Overlay48CreditCommand {
    O48_CREDITS_LOAD = -1,
    O48_CREDITS_HOLD = -2,
    O48_CREDITS_RELEASE = -3,
    O48_CREDITS_FINISH = -4
};

s16 o48CreditCommands[128] = {
    O48_CREDITS_LOAD, 1, 120,
    O48_CREDITS_LOAD, 1, 360,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_LOAD, 2, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 4, 180,
    O48_CREDITS_HOLD, 0,
    O48_CREDITS_LOAD, 3, 180,
    O48_CREDITS_RELEASE, 0,
    O48_CREDITS_LOAD, 1, 360,
    O48_CREDITS_FINISH,
};
