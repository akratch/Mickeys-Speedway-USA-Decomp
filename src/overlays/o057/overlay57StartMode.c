#include "PR/ultratypes.h"

typedef struct Overlay57Object {
    s32 value;
} Overlay57Object;

typedef struct Overlay57Result {
    u8 pad0[0x16];
    u8 flags;
} Overlay57Result;

extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay84GetActive(s32 value);
extern void func_8005055C(u8 id);
extern void animseqStartPath(u8 id);
extern Overlay57Result *func_800508B4(u8 id);
extern void overlay57ApplyTable(void);
extern void func_overlay_057_F0001020_18A4C18(s32 value);

/* DKR v77/v80 and JFG have no matching donor for this mode transition. */
void overlay57StartMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay84GetActive(value) == NULL) {
        object = &gOverlay57Object;
        gOverlay57Timer = 16;
        object->value = 68;
        gOverlay57State = 5;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    overlay57ApplyTable();
    func_overlay_057_F0001020_18A4C18(value);
}
