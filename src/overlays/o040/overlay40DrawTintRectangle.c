#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

extern s16 D_800D6C3E, D_800D6C38;
extern s16 gO40TintGreenReloc, gO40TintBlueReloc;
extern Gfx gO40TintDisplayListReloc[];
extern void o40TintCallReloc();

/* Overlay 40 text +0x534..+0x690. */
void overlay40DrawTintRectangle(Gfx **displayList) {
    s32 inverse, red, green, blue;
    s32 x;
    s32 y;

    if (D_800D6C3E != 0) {
        o40TintCallReloc(&x, &y, displayList);
        inverse = 0xFF - D_800D6C3E;
        red = (D_800D6C38 * inverse) >> 8;
        green = (gO40TintGreenReloc * inverse) >> 8;
        blue = (gO40TintBlueReloc * inverse) >> 8;
        gSPDisplayList((*displayList)++, gO40TintDisplayListReloc);
        gDPSetPrimColor((*displayList)++, 0, 0, red, green, blue,
                        D_800D6C3E);
        gDPFillRectangle((*displayList)++, 0, 0, x, y);
        o40TintCallReloc(displayList);
        gDPSetPrimColor((*displayList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
