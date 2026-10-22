#include "PR/ultratypes.h"

typedef struct Overlay101BoundsNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 opacity;
    u8 scale;
} Overlay101BoundsNode;

typedef struct Overlay101Pixel {
    u8 value;
} Overlay101Pixel;

typedef struct Overlay101Element {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 count;
    u8 index;
    u8 kind;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 flags;
    Overlay101Pixel *pixels;
} Overlay101Element;

extern u32 gOverlay101RenderFlags;

void fontUseFont(s32 kind, s32 alpha, Overlay101Pixel *pixels);
void fontBackground(s32, s32, s32, s32);
void fontColour(s32 red, s32 green, s32 blue, s32 alpha,
                             s32 opacity);
void overlay101GetBounds(Overlay101BoundsNode *node, s32 *left, s32 *top,
                              s32 *right, s32 *bottom);
void overlay101SetScissor(s32 displayList, s32 left, s32 top, s32 right,
                               s32 bottom);
void fontPrintXY(s32 displayList, s32 x, s32 y,
                                Overlay101Pixel *pixels,
                                s32 flags);
void overlay101SetScaleReloc(s32 displayList, s32 x, s32 y, s32 width,
                             s32 height);

/* DKR v77/v80 and JFG contain related sprite/HUD draw pipelines but no exact
 * object donor for this element renderer. */
void overlay101DrawElement(s32 displayList, Overlay101BoundsNode *node,
                           Overlay101Element *element) {
    s32 x;
    s32 y;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    u8 saved0;
    u8 saved1;
    Overlay101Pixel *pixels;
    Overlay101Pixel *savedPixel;

    if ((node->type == 2) || (node->type == 4)) {
        if ((element->index > 0) && (element->alpha > 0) &&
            ((pixels = element->pixels) != 0)) {
            savedPixel = 0;
            x = node->x + element->x;
            y = node->y + element->y;
            if (element->index < element->count) {
                savedPixel = pixels + element->index;
                saved0 = savedPixel[0].value;
                saved1 = savedPixel[1].value;
                if (gOverlay101RenderFlags & 8) {
                    savedPixel[0].value = 0x24;
                    savedPixel[1].value = 0;
                } else {
                    savedPixel[0].value = 0;
                }
            } else {
                savedPixel = 0;
            }
            red = (element->red * node->opacity) / 255;
            green = (element->green * node->opacity) / 255;
            blue = (element->blue * node->opacity) / 255;
            alpha = (element->alpha * node->scale) / 255;

            fontUseFont(element->kind, element->alpha,
                                         pixels);
            fontBackground(0, 0, 0, 0);
            fontColour(red, green, blue, 0xFF, alpha);
            overlay101GetBounds(node, &left, &top, &right, &bottom);
            overlay101SetScissor(displayList, left, top, right, bottom);
            fontPrintXY(displayList, x, y, pixels,
                                       element->flags);
            overlay101SetScaleReloc(displayList, 0, 0, 1000, 1000);

            if (savedPixel != 0) {
                savedPixel[0].value = saved0;
                savedPixel[1].value = saved1;
            }
        }
    }
}
