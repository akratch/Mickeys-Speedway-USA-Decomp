#include "PR/ultratypes.h"

typedef struct Overlay101BoundsNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} Overlay101BoundsNode;

void viGetCurrentSize(s32 *width, s32 *height);

/* DKR v77/v80 and JFG have viewport-clipping relatives but no exact donor. */
void overlay101GetBounds(Overlay101BoundsNode *node, s32 *leftOut,
                         s32 *topOut, s32 *rightOut, s32 *bottomOut) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 valid;
    s32 screenWidth;
    s32 screenHeight;

    valid = 0;
    if ((node->type == 2) || (node->type == 4)) {
        viGetCurrentSize(&screenWidth, &screenHeight);
        left = node->x;
        top = node->y;
        right = node->x + node->width;
        bottom = node->y + node->height;
        if (node->type != 4) {
            left += 4;
            top += 0xC;
            right -= 4;
            bottom -= 4;
        }
        if ((left < screenWidth) && (top < screenHeight) && (right >= 0) &&
            (bottom >= 0)) {
            if (left < 0) {
                left = 0;
            }
            if ((u32)right >= (u32)screenWidth) {
                right = screenWidth - 1;
            }
            if (top < 0) {
                top = 0;
            }
            if ((u32)bottom >= (u32)screenHeight) {
                bottom = screenHeight - 1;
            }
            *leftOut = left;
            valid = 1;
            *topOut = top;
            *rightOut = right;
            *bottomOut = bottom;
        }
    }
    if (valid == 0) {
        *leftOut = 0;
        *topOut = 0;
        *rightOut = 0;
        *bottomOut = 0;
    }
}
