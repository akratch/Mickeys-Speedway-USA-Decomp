#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101PanelRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101PanelRect;

typedef struct Overlay101Panel {
    u8 pad00[8];
    u8 mode;
    s8 phase;
    s16 currentX;
    s16 currentY;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 intensity;
    u8 alpha;
    void *content;
} Overlay101Panel;

extern u8 D_1F4[];

void overlay101SetScissorReloc(Overlay101Gfx **displayList, s32 left, s32 top,
                               s32 right, s32 bottom);
void overlay101DrawRectReloc(Overlay101Gfx **displayList, s32 x, s32 y,
                             s32 width, s32 height, u32 color);
void overlay101BuildIntensityColorsReloc(s32 intensity, s32 alpha, u32 *full,
                                         u32 *dim, u32 *dimmer, u32 *darkest);
void overlay101BuilderCreateReloc(Overlay101Gfx **displayList, s32 count,
                                  Overlay101PanelRect *rects, s32 flags);
void overlay101BuildBorderReloc(Overlay101Gfx **displayList, s32 x, s32 y,
                                s32 width, s32 height, s32 intensity,
                                s32 alpha, s32 swapColors);
void overlay101SelectElementReloc(s32 element);
void overlay101SetRenderModeReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void overlay101SetColorReloc(s32 red, s32 green, s32 blue, s32 colorAlpha,
                             s32 alpha);
void overlay101DrawElementReloc(Overlay101Gfx **displayList, s32 x, s32 y,
                                void *content, s32 flags);
void overlay101DrawDefaultAssetReloc(Overlay101Gfx **displayList, u8 *asset,
                                     s32 x, s32 y, s32 red, s32 green,
                                     s32 blue, s32 alpha);

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor.
 *
 * Matched 2026-10-01 by discarding the inherited shape rather than tuning it.
 * The old candidate kept the colours, the dimensions and the records in one
 * padded work struct, forced the per-record colour reloads with `volatile`,
 * carried `right` and `bottom` in declared locals and steered uopt with two
 * `if (1)` regions. What the listing asks for is plainer on every point:
 *   - `out = records` is assigned before the BuildIntensityColors call, so
 *     the pointer's value reaches the record stores across a call. uopt then
 *     still folds every store address to a frame offset but no longer proves
 *     the stores miss the four address-taken colours, so each record reloads
 *     its colour with no qualifier, and the last record goes through the
 *     pointer's own register. Assigning it after the call folds the first
 *     three records' reads into one load.
 *   - x, y, width and height are ordinary locals shared by the mode 1/3 arm
 *     and the mode 2 arm, which is what colours the mode 1/3 height.
 *   - the right and bottom edges are the expressions `x + width` and
 *     `y + height` at each use; uopt commons them and spills the one that
 *     crosses calls to a compiler temporary below the record array.
 *   - `out` is declared between the colours and the array, which is the
 *     four-byte cell above the records. */
void overlay101DrawPanel(Overlay101Gfx **displayList, Overlay101Panel *panel) {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u32 full;
    u32 dimmer;
    u32 dim;
    u32 darkest;
    Overlay101PanelRect *out;
    Overlay101PanelRect records[20];

    overlay101SetScissorReloc(displayList, 0, 0, 1000, 1000);

    if ((panel->mode == 1) || (panel->mode == 3)) {
        x = panel->currentX +
            (((panel->x - panel->currentX) * panel->phase) >> 6);
        y = panel->currentY +
            (((panel->y - panel->currentY) * panel->phase) >> 6);
        width = 32 + (((panel->width - 32) * panel->phase) >> 6);
        height = 32 + (((panel->height - 32) * panel->phase) >> 6);
        overlay101DrawRectReloc(displayList, x, y, width, height, 0xC0C0C0FF);
    } else if (panel->mode == 2) {
        out = records;
        x = panel->x;
        y = panel->y;
        width = panel->width;
        height = panel->height;

        overlay101BuildIntensityColorsReloc(panel->intensity + 1, panel->alpha,
                                            &full, &dim, &dimmer, &darkest);

        out->x0 = x + 1;
        out->y0 = y + 12;
        out->x1 = x + 3;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x + 1;
        out->y0 = y + height - 3;
        out->x1 = x + width - 1;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x + width - 3;
        out->y0 = y + 12;
        out->x1 = x + width - 1;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x;
        out->y0 = y + 12;
        out->x1 = x + 1;
        out->y1 = y + height;
        out->color = dim;
        out++;

        out->x0 = x + 1;
        out->y0 = y + height - 1;
        out->x1 = x + width;
        out->y1 = y + height;
        out->color = dim;
        out++;

        out->x0 = x + width - 4;
        out->y0 = y + 12;
        out->x1 = x + width - 3;
        out->y1 = y + height - 3;
        out->color = dim;
        out++;

        out->x0 = x + 3;
        out->y0 = y + 12;
        out->x1 = x + 4;
        out->y1 = y + height - 3;
        out->color = full;
        out++;

        out->x0 = x + 4;
        out->y0 = y + height - 4;
        out->x1 = x + width - 3;
        out->y1 = y + height - 3;
        out->color = full;
        out++;

        out->x0 = x + width - 1;
        out->y0 = y + 12;
        out->x1 = x + width;
        out->y1 = y + height;
        out->color = full;
        out++;

        out->x0 = x + 4;
        out->y0 = y + 12;
        out->x1 = x + width - 4;
        out->y1 = y + height - 4;
        out->color = darkest;
        out++;

        overlay101BuilderCreateReloc(displayList, 10, records, 0);

        overlay101BuildBorderReloc(displayList, x, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorderReloc(displayList, x + 12, y, width - 36, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorderReloc(displayList, x + width - 24, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorderReloc(displayList, x + width - 12, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);

        if (panel->content != NULL) {
            overlay101SetScissorReloc(displayList, x + 14, y + 2,
                                      x + width - 26, y + 10);
            overlay101SelectElementReloc(2);
            overlay101SetRenderModeReloc(0, 0, 0, 0);
            overlay101SetColorReloc(0, 0, 0, 255, panel->alpha);
            overlay101DrawElementReloc(displayList, x + 14, y + 2,
                                       panel->content, 0);
            overlay101SetScissorReloc(displayList, 0, 0, 1000, 1000);
        }
    } else if (panel->mode == 0) {
        overlay101DrawDefaultAssetReloc(displayList, D_1F4, panel->currentX,
                                        panel->currentY, 255, 255, 255,
                                        panel->alpha);
    }
}
