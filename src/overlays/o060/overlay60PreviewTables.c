#include "overlays/overlay_060.h"

/* Tier B: four actor cells and two four-point placement tables are bounded
 * by the initializer/reposition loops. Five preview modes select scales;
 * the four actors select progress masks/shifts. Encoded glyphs are read by
 * the font renderer at the upper/lower preview positions.
 * Tier D: original names/declarations remain inferred. The next real scalar
 * bounds the four-byte rank-frame object, not the range of spare - 1: the
 * existing negative/out-of-range indexing obligation remains open.
 * Initializer Overlay60Object and updater ControlActor views also remain
 * separate legacy declarations of these actual object pointers. */
char o60UpperPreviewGlyph[] = "\x80\x42";
char o60LowerPreviewGlyph[] = "\x80\x44";
Overlay60Object *o60PreviewObjects[4] = {NULL, NULL, NULL, NULL};
Overlay60Point o60PreviewPositions[4] = {{0,-42},{50,-42},{0,-110},{50,-110}};
Overlay60Point o60SplitPreviewPositions[4] = {{1,-42},{67,-42},{1,-110},{67,-110}};
f32 o60PreviewScales[5] = {0.3f,0.3f,0.25f,0.3f,0.3f};
s32 o60ProgressMasks[4] = {7,56,448,3584};
s32 o60ProgressShifts[4] = {0,3,6,9};
s8 o60RankFrames[4] = {2,1,0,3};
