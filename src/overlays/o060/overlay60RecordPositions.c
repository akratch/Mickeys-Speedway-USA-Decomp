#include "PR/ultratypes.h"

/* Tier B: the result renderer reads thirteen signed column coordinates
 * (two fixed columns and eleven glyph columns) and four row coordinates.
 * The next real row object follows ordinary compiler word alignment; the
 * source owns no fourteenth column, fifth row or following end-tail bytes.
 * Tier D: original names/declarations and constness remain inferred. */
s16 o60RecordColumns[13] = {81, 105, 121, 131, 141, 163, 171, 177, 183, 191, 197, 203, 211};
s16 o60RecordRows[4] = {139, 155, 171, 203};
