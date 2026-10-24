#include "overlays/overlay_057.h"

/* Tier B: the status panel and node update extract four consecutive three-bit
 * option fields. The existing callers retain their legacy relocation names;
 * these definitions own the actual initialized storage at those targets. */
u32 o57OptionFieldMasks[4] = {
    7u << 0, 7u << 3, 7u << 6, 7u << 9,
};

u32 o57OptionFieldShifts[4] = { 0, 3, 6, 9 };

/* The initializer creates four objects from these signed x/y positions. */
O57Pair o57InitialSpawnPositions[4] = {
    { -125, 2 },
    { -125, -53 },
    { 125, 2 },
    { 125, -53 },
};

/* Tier B: frontDrawRectangles reads four signed bounds and a packed colour.
 * The panel selects one of these eight-rectangle layouts and replaces the
 * colours of entries 1..7 each frame, so the records are mutable. */
typedef struct O57MenuRectangle {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 colour;
} O57MenuRectangle;

O57MenuRectangle o57FourRowPanelRectangles[8] = {
    { 206, 56, 290, 184, 0 },
    { 212, 58, 284, 59, 0 },
    { 210, 59, 286, 60, 0 },
    { 209, 60, 287, 62, 0 },
    { 208, 62, 288, 178, 0 },
    { 209, 178, 287, 180, 0 },
    { 210, 180, 286, 181, 0 },
    { 212, 181, 284, 182, 0 },
};

O57MenuRectangle o57ThreeRowPanelRectangles[8] = {
    { 206, 71, 290, 169, 0 },
    { 212, 73, 284, 74, 0 },
    { 210, 74, 286, 75, 0 },
    { 209, 75, 287, 77, 0 },
    { 208, 77, 288, 163, 0 },
    { 209, 163, 287, 165, 0 },
    { 210, 165, 286, 166, 0 },
    { 212, 166, 284, 167, 0 },
};
