#include "PR/ultratypes.h"

typedef struct Overlay11Status {
    u8 value0;
    u8 value1;
    u8 value2;
    u8 mode;
} Overlay11Status;

typedef struct Overlay11Resources {
    u8 pad000[0x50];
    void *resource50;
    u8 pad054[0x13C];
    s32 value190;
    char *format194;
    void *resource198;
    void *resource19C;
    void *resource1A0;
    u8 pad1A4[4];
    void *resource1A8;
} Overlay11Resources;

extern Overlay11Resources *gOverlay11Resources;
extern char gOverlay11Format[];
extern void *gOverlay11Created[4];
extern s32 gOverlay11CreatedActive;
extern u16 D_800D31BC;

extern Overlay11Status *func_80028F54(void);
extern s32 sprintf(char *buffer, const char *format, ...);
/* overlay45CreateDescriptor, through the placeholder its siblings use. */
extern void *overlay11CreateReloc(void *resource, s32 x, s32 y, s32 mode);

/* DKR v77/v80 and JFG contain no matching Overlay 11 initializer donor. */
void overlay11InitializeFour(void) {
    char buffer[80];
    Overlay11Status *status;

    status = func_80028F54();
    if (status->mode == 2) {
        sprintf(buffer, gOverlay11Format, gOverlay11Resources->value190);
    } else {
        sprintf(buffer, gOverlay11Resources->format194, status->mode - 1);
    }

    gOverlay11Created[0] = overlay11CreateReloc(
        gOverlay11Resources->resource198, 160, 40, 4);
    gOverlay11Created[1] = overlay11CreateReloc(
        gOverlay11Resources->resource19C, 160, 100, 4);

    if (status->mode == 1) {
        gOverlay11Created[2] = overlay11CreateReloc(
            gOverlay11Resources->resource50, 160, 135, 4);
        gOverlay11Created[3] = 0;
    } else {
        if (D_800D31BC & 0x100) {
            gOverlay11Created[2] = overlay11CreateReloc(
                gOverlay11Resources->resource1A8, 160, 135, 4);
        } else if (status->value1 == 0) {
            gOverlay11Created[2] = overlay11CreateReloc(
                gOverlay11Resources->resource1A0, 160, 135, 4);
        } else {
            gOverlay11Created[2] =
                overlay11CreateReloc(buffer, 160, 135, 4);
        }
        gOverlay11Created[3] = overlay11CreateReloc(
            gOverlay11Resources->resource50, 160, 170, 4);
    }
    gOverlay11CreatedActive = 1;
}
