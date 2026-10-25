#include "PR/ultratypes.h"

/* Tier B: one signed word is read by the controller update and written by
 * its exported setter. Tier D: original name/declaration and source grouping.
 * Explicit initial zero is retained initialized storage, not a BSS reserve. */
s32 o94ControllerValue = 0;
