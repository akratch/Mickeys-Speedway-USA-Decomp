#include "PR/ultratypes.h"

typedef struct Overlay57Object {
    s32 value;
} Overlay57Object;

typedef struct Overlay57Result {
    u8 pad0[0x16];
    u8 flags;
} Overlay57Result;

typedef struct Overlay57ModeValue {
    s32 value;
} Overlay57ModeValue;

extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57Timer;
extern s32 gOverlay57Delay;
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay84GetActive(s32 value);
extern void func_8005055C(u8 id);
extern void animseqStartPath(u8 id);
extern Overlay57Result *func_800508B4(u8 id);
extern void overlay94SetValue(s32 value);

/* DKR v77/v80 and JFG have no matching donor; 0xA00 is generic-only. */
void overlay57BeginMode(Overlay57ModeValue value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay84GetActive(value.value) == NULL) {
        gOverlay57Delay = 0xA00;
        object = &gOverlay57Object;
        gOverlay57Timer = 13;
        object->value = 0x3F;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
        value.value = 1;
        overlay94SetValue(value.value);
    }
}
