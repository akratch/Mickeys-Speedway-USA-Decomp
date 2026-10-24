#include "PR/ultratypes.h"

/* Tier B: these independent signed words are read and updated by the mode,
 * panel, pulse and transition routines. Alpha reflects at 0 and 256 by
 * negating its signed step; the panel approaches -0x1400 or 0. The transition
 * delay eases toward 0xA00 and back to zero. Tier D: original object names
 * and qualifiers are inferred. No unknown neighboring word is adopted. */
s32 o57ModeChoice = 0;
s32 o57PanelPosition = -0x1400;
s32 o57PulseAlpha = 0;
s32 o57PulseAlphaStep = 4;
s32 o57TransitionDelay = 0xA00;
s32 o57TransitionComplete = 0;
