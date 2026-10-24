#include "overlays/overlay_057.h"

extern const char o57FirstResultLabel[];
extern const char o57SecondResultLabel[];
extern const char o57ThirdResultLabel[];
extern char o57FourthResultLabel[];
extern char o57FifthResultLabel[];
extern char o57SixthResultLabel[];

/* Tier B: six actual result-label pointer cells and the eleven signed-halfword
 * positions read by the current renderer. Only labels0..2 have active current
 * reads. Tier D: original names/declarations and capacity remain inferred. */
const char *o57ResultLabelPointers[6] = {
    o57FirstResultLabel, o57SecondResultLabel, o57ThirdResultLabel,
    o57FourthResultLabel, o57FifthResultLabel, o57SixthResultLabel
};
s16 o57ResultColumns[11] = {122, 138, 154, 178, 192, 206, 214, 228, 242, 250, 264};

#ifdef TARGET_N64
typedef char O57ResultPointerWidth[(sizeof(const char *) == 4) ? 1 : -1];
typedef char O57ResultLabelsWidth[(sizeof(o57ResultLabelPointers) == 24) ? 1 : -1];
#endif
typedef char O57ResultColumnsWidth[(sizeof(o57ResultColumns) == 22) ? 1 : -1];
