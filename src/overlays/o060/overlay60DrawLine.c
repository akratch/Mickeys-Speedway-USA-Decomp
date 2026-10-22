#include "PR/ultratypes.h"

typedef struct Overlay60Command {
    u32 w0;
    u32 w1;
} Overlay60Command;

extern Overlay60Command *D_800D3140;

#define OVERLAY60_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define OVERLAY60_FILL_RECTANGLE(packet, upperLeftX, upperLeftY, lowerRightX, \
                                 lowerRightY)                              \
    {                                                                      \
        Overlay60Command *macroCommand = (Overlay60Command *)(packet);      \
        macroCommand->w0 = 0xF6000000 |                                    \
                           OVERLAY60_SHIFTL(lowerRightX, 14, 10) |          \
                           OVERLAY60_SHIFTL(lowerRightY, 2, 10);            \
        macroCommand->w1 = OVERLAY60_SHIFTL(upperLeftX, 14, 10) |           \
                           OVERLAY60_SHIFTL(upperLeftY, 2, 10);             \
    }

/* DKR v77/v80 and JFG have no exact donor; DKR borders.c confirms the idiom. */
void overlay60DrawLine(s32 left, s32 top, s32 right, s32 bottom) {
    if (left == right) {
        OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, right + 1,
                                 bottom);
    } else {
        OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, right,
                                 bottom + 1);
    }
}
