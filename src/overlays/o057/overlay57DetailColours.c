#include "PR/ultratypes.h"

#define DETAIL_RGB(r, g, b) (((u32)(r) << 24) | ((u32)(g) << 16) | ((u32)(b) << 8))

/* Tier B: detail drawing selects a packed RGB word and ORs its independently
 * computed alpha into the low byte. The adjacent signed flag gates drawing
 * and is written as zero/one. Tier D: original names and qualifiers unknown.
 * The selection's nonnegative domain remains a caller precondition. */
u32 o57DetailColours[5] = {
    DETAIL_RGB(48, 48, 0), DETAIL_RGB(0, 32, 48),
    DETAIL_RGB(48, 0, 0), DETAIL_RGB(0, 48, 0), DETAIL_RGB(48, 24, 0),
};
s32 o57DetailAvailable = 0;
