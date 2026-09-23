#include "PR/ultratypes.h"

typedef struct O14ValueC0Ref {
    u8 pad00[0xC0];
    s32 value;
} O14ValueC0Ref;

typedef struct O14ValueDCRef {
    u8 pad00[0xDC];
    s32 value;
} O14ValueDCRef;

typedef struct O14HandleE0Ref {
    u8 pad00[0xE0];
    s32 value;
} O14HandleE0Ref;

typedef struct O14ModeE4Ref {
    u8 pad00[0xE4];
    s32 value;
} O14ModeE4Ref;

typedef struct O14ValueECRef {
    u8 pad00[0xEC];
    s32 value;
} O14ValueECRef;

typedef struct O14ValueF8Ref {
    u8 pad00[0xF8];
    s32 value;
} O14ValueF8Ref;

typedef struct O14Value114Ref {
    u8 pad00[0x114];
    s32 value;
} O14Value114Ref;

extern O14ValueC0Ref gOverlay14ValueC0Base;
extern O14ValueDCRef gOverlay14ValueDCBase;
extern O14HandleE0Ref gOverlay14HandleE0;
extern O14ModeE4Ref gOverlay14ModeE4;
extern O14ValueECRef gOverlay14ValueEC;
extern O14ValueF8Ref gOverlay14ValueF8Base;
extern O14Value114Ref gOverlay14Value114;
extern s32 gOverlay14Args2C;
extern s32 gOverlay14Args30;
extern s32 gOverlay14DataBase;

extern void overlay14BuildPanel(s32 context, void *base, s32 width, s32 height,
                                s32 extra0, s32 extra1, s32 extra2);
extern s32 overlay14CreateHandle(s32 context, s32 valueF8, s32 valueDC);
extern void overlay14DrawPrimitive();

void func_overlay_014_F00013F4_1870CCC(s32 context) {
    s32 mode;

    overlay14BuildPanel(context, &gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0Base.value * 0xA0) >> 8);
    gOverlay14HandleE0.value =
        overlay14CreateHandle(context, gOverlay14ValueF8Base.value,
                              gOverlay14ValueDCBase.value);
    overlay14DrawPrimitive(0, 0, 0, 0);
    overlay14DrawPrimitive(0xFF, 0xC0, 0, 0xFF,
                           (gOverlay14ValueC0Base.value * 0xFF) >> 8);
    if (gOverlay14ValueDCBase.value != 0) {
        overlay14DrawPrimitive(context, 0xC4, 0x14, &gOverlay14Args2C, 0xC);
    }
    if ((gOverlay14HandleE0.value != 0) ||
        ((mode = gOverlay14ModeE4.value, mode == 4) &&
         (gOverlay14ValueEC.value >= 2)) ||
        ((mode == 3) && (gOverlay14Value114.value != 0))) {
        overlay14DrawPrimitive(context, 0xC4, 0x6C, &gOverlay14Args30, 0xC);
    }
}
