#include "PR/ultratypes.h"

/* Tier B: the four-selector update masks each selector to 0..15, averages
 * these float factors, and uses the separate byte map to select one of the
 * acquired configuration record's four factors. The two arrays are
 * distinct initialized objects; they are not an encoded byte payload. */
f32 o8MeanFactorsBySelector[16] = {
    1.0f, 0.75f, 0.25f, 1.0f,
    0.9f, 0.8f, 1.0f, 0.9f,
    0.75f, 1.0f, 0.9f, 0.8f,
    0.96f, 0.92f, 0.75f, 1.0f,
};

u8 o8RecordFactorIndicesBySelector[16] = {
    0, 0, 0, 0,
    1, 2, 0, 2,
    2, 0, 2, 3,
    0, 0, 2, 0,
};
