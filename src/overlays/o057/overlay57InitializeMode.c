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
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay84GetActive(s32 value);
extern void func_8005055C(u8 id);
extern void animseqStartPath(u8 id);
extern Overlay57Result *func_800508B4(u8 id);
extern void func_overlay_057_F0001020_18A4C18(s32 value);

/* DKR v77/v80 and JFG checks found no matching mode-initialization donor. */
void overlay57InitializeMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 1;
    if (overlay84GetActive(value) == NULL) {
        object = &gOverlay57Object;
        gOverlay57Timer = 20;
        object->value = 84;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    func_overlay_057_F0001020_18A4C18(value);
}
