#include "PR/ultratypes.h"

/* Tier B: all three signed-halfword menu views use the same character order.
 * The chooser initializes zero and wraps its selector over 0..9; the low-slot
 * repair keeps its chosen values below six. Existing valid-state/index
 * preconditions remain. Tier D: original name/declaration are inferred. */
s16 o57CharacterOrder[10] = {0, 3, 4, 5, 1, 2, 6, 7, 8, 9};
