#include "PR/ultratypes.h"

extern const char o58FirstRankLabel[];
extern const char o58SecondRankLabel[];
extern const char o58ThirdRankLabel[];
extern const char o58FourthRankLabel[];
extern const char o58FifthRankLabel[];
extern const char o58SixthRankLabel[];

/* Tier B: six rank labels pass through the font's separate scratch conversion;
 * the saved-record output reads glyph positions zero through ten. This is the
 * proved read view, not a claim that the legacy twelfth slot was never usable. */
typedef struct {
    const char *labels[6];
    s16 glyphX[11];
} RankTextLayout;

typedef char O58RankTextLayoutSize[(sizeof(RankTextLayout) == 48) ? 1 : -1];

RankTextLayout o58RankTextLayout = {
    {o58FirstRankLabel, o58SecondRankLabel, o58ThirdRankLabel,
     o58FourthRankLabel, o58FifthRankLabel, o58SixthRankLabel},
    {122, 138, 154, 186, 200, 214, 222, 236, 250, 258, 272}
};
