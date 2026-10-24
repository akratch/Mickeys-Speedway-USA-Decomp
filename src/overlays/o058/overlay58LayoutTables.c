#include "PR/ultratypes.h"

/* Tier B: the results renderer indexes each signed layout table by the
 * participant count minus one. The six-row capacity is a retail input
 * precondition, not a runtime bounds check. */
s16 o58RowHeightsByParticipantCount[6] = { 27, 64, 45, 39, 27, 27 };
s16 o58RowBasesByParticipantCount[6] = { 54, 85, 75, 62, 54, 54 };
s16 o58ColumnStartsByParticipantCount[6] = { 0, 115, 110, 105, 0, 0 };
s16 o58ColumnStepsByCount[6] = { 0, 80, 60, 45, 0, 0 };
