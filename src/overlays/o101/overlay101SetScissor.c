#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

void viGetCurrentSize(s32 *width, s32 *height);

/* DKR and JFG use the same G_SETSCISSOR macro semantics, but neither pinned
 * object set contains this clipping helper as an exact donor. */
void overlay101SetScissor(Overlay101Gfx **displayList, register s32 left, s32 top,
                          s32 right, s32 bottom) {
    s32 width;
    s32 height;

    if ((right >= left) && (bottom >= top)) {
        viGetCurrentSize(&width, &height);
        if ((right > 0) && (left < width) &&
            (bottom > 0) && (top < height)) {
            if (left < 0) {
                left = 0;
            }
            if ((u32)width < (u32)right) {
                right = width - 1;
            }
            if (top < 0) {
                top = 0;
            }
            if ((u32)height < (u32)bottom) {
                bottom = height - 1;
            }
        } else {
            left = 0;
            top = 0;
            right = 0;
            bottom = 0;
        }
    } else {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }

    {
        Overlay101Gfx *command;
        command = (*displayList)++;
        command->w0 = 0xE7000000;
        command->w1 = 0;
    }
    {
        Overlay101Gfx *command;
        command = (*displayList)++;
        command->w0 = 0xED000000 |
                      (((s32)((f32)left * 4.0f) & 0xFFF) << 12) |
                      ((s32)((f32)top * 4.0f) & 0xFFF);
        command->w1 = (((s32)((f32)right * 4.0f) & 0xFFF) << 12) |
                      ((s32)((f32)bottom * 4.0f) & 0xFFF);
    }
}
