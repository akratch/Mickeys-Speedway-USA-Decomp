#include "overlays/overlay_101.h"

/* Tier B: both resident texture renderers scan 16-byte nodes until a null
 * texture. The two loaded slots at initialized +0x1D4/+0x1F4 each precede
 * a separate null sentinel. Dispatch/reset and the live counters establish
 * the remaining fields. Tier D: this is one bounded storage view, not a
 * claim that the original enclosing declaration was recovered. */
typedef struct Overlay101State {
    s32 textNodeCount;
    RcpTextureNode backdrop[2];
    RcpTextureNode panel[2];
    s32 blinkClock;
    s32 presentationDone;
    s32 activeEntryCount;
    Overlay101Event *events;
    s32 eventIndex;
    s32 eventClock;
} Overlay101State;

typedef char O101StatePointerWidth[(sizeof(void *) == 4) ? 1 : -1];
typedef char O101StateIntegerWidth[(sizeof(s32) == 4) ? 1 : -1];
typedef char O101StateTextureNodeSize[(sizeof(RcpTextureNode) == 16) ? 1 : -1];
typedef char O101StateSize[(sizeof(Overlay101State) == 0x5C) ? 1 : -1];

/* Explicit initialization is the actual writable .data storage. Compiler
 * section alignment supplies the final four bytes before the clock owner. */
Overlay101State o101State = {
    0,
    {{NULL, NULL, 0, 0, 0}, {NULL, NULL, 0, 0, 0}},
    {{NULL, NULL, 0, 0, 0}, {NULL, NULL, 0, 0, 0}},
    0, 0, 0, NULL, 0, 0,
};
