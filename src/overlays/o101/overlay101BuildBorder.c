#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101BorderRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101BorderRect;

void overlay101BuildIntensityColorsReloc(s32 intensity, s32 alpha, u32 *full,
                                         u32 *dim, u32 *dimmer, u32 *darkest);
void overlay101BuilderCreateReloc(Overlay101Gfx **displayList, s32 count,
                                  Overlay101BorderRect *rects, s32 flags);

/* The two `if (1)` blocks are uopt region boundaries, and each is load-bearing
 * for the shipped bytes rather than decoration. Only a control-flow construct
 * opens a region: a bare compound statement measures byte-identical to no
 * block at all. The first one renumbers every geometry web -- it is what puts
 * x, x + width, x + 1 and y + height in a0..a3 and lets a2 carry y + 1 once x
 * is dead -- and takes the candidate from 38 differing words to 10. The second
 * is a join point between the two `trailingColor` reads, which is what stops
 * uopt from folding them into one load; that is why trailingColor needs no
 * `volatile` while leadingColor, whose two reads have no join between them,
 * still does. Falsified with each measured: all sixteen commutative operand
 * orders of the four geometry sums are byte-inert, 1,888 statement orders and
 * 297 region spans reach 10 but no lower without the second boundary, and
 * dropping leadingColor's `volatile` folds its pair and costs an instruction.
 * The converse (trailingColor `volatile` too, second block deleted) keeps size
 * and registers but schedules two colour loads late: 4 words (lane c-1). */
void overlay101BuildBorder(Overlay101Gfx **displayList, s32 x, s32 y,
                           s32 width, s32 height, s32 intensity, s32 alpha,
                           s32 swapColors) {
    u32 trailingColor;
    volatile u32 leadingColor;
    volatile u32 interiorColor;
    Overlay101BorderRect *rect;
    Overlay101BorderRect rects[5];

    if (swapColors != 0) {
        overlay101BuildIntensityColorsReloc(
            intensity + 1, alpha, (u32 *)&leadingColor,
            (u32 *)&trailingColor, (u32 *)&interiorColor, NULL);
    } else {
        overlay101BuildIntensityColorsReloc(
            intensity + 1, alpha, (u32 *)&trailingColor,
            (u32 *)&leadingColor, (u32 *)&interiorColor, NULL);
    }

    rect = rects;
    rect[0].x0 = x;
    rect[0].y0 = y;
    rect[0].x1 = x + 1;
    rect[0].y1 = y + height - 1;
    rect[1].x0 = x;
    rects[0].color = leadingColor;
    rect[1].y0 = y + height - 1;
    rect[1].x1 = x + width;
    if (1) {
        rect[1].y1 = y + height;
        rects[1].color = leadingColor;
        rect[2].x0 = x + width - 1;
        rect[2].y0 = y + 1;
    }
    rect[2].x1 = x + width;
    rect[2].y1 = y + height - 1;
    rects[2].color = trailingColor;
    rect[3].x0 = x + 1;
    if (1) {
    }
    rect[3].y0 = y;
    rect[3].x1 = x + width;
    rect[3].y1 = y + 1;
    rects[3].color = trailingColor;

    rect += 4;
    rect->x0 = x + 1;
    rect->y0 = y + 1;
    rect->x1 = x + width - 1;
    rect->y1 = y + height - 1;
    rect->color = interiorColor;

    overlay101BuilderCreateReloc(displayList, 5, rects, 0);
}
