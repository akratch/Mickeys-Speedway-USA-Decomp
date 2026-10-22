#include "PR/ultratypes.h"

/* DKR v77/v80 object and source checks found no corresponding state setter. */
extern s16 D_800D6C38;
extern s16 gOverlay40Value1;
extern s16 D_800D6C44;
extern s16 D_800D6C3E;
extern s16 gOverlay40Value4;
extern s16 gOverlay40Value5;
extern s16 gOverlay40Value6;
extern s16 gOverlay40Value7;

void overlay40SetValues(s32 value0, s32 value1, s32 value2, s32 value3,
                        s32 value4) {
    D_800D6C38 = value0;
    gOverlay40Value1 = value1;
    gOverlay40Value4 = value2;
    gOverlay40Value5 = D_800D6C3E;
    gOverlay40Value6 = value3;
    gOverlay40Value7 = value4;
    D_800D6C44 = value4;
}
