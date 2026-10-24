#include "PR/ultratypes.h"

/* Tier B: the three existing signed-byte walks visit twelve fixed labels,
 * then four labels controlled by each independently computed unlock flag.
 * Tier D: original declarations/names/qualifiers remain inferred. The old
 * consumers' separate endpoint aliases are not unified by this storage step. */
s8 o57AlwaysEnabledLabels[12] = {14, 12, 11, 16, 24, 15, 9, 2, 8, 10, 13, 18};
s8 o57FirstUnlockLabels[4] = {27, 19, 20, 21};
s8 o57SecondUnlockLabels[4] = {25, 17, 28, 29};
