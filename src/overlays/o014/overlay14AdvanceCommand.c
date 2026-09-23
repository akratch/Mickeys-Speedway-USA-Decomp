#include "PR/ultratypes.h"

typedef struct O14StateCCRef {
    u8 pad00[0xCC];
    s32 value;
} O14StateCCRef;

typedef struct O14TransitionD8Ref {
    u8 pad00[0xD8];
    s32 value;
} O14TransitionD8Ref;

typedef struct O14PointerE0Ref {
    u8 pad00[0xE0];
    s32 value;
} O14PointerE0Ref;

typedef struct O14ModeE4Ref {
    u8 pad00[0xE4];
    s32 value;
} O14ModeE4Ref;

typedef struct O14ValueECRef {
    u8 pad00[0xEC];
    s32 value;
} O14ValueECRef;

typedef struct O14Value110Ref {
    u8 pad00[0x110];
    s32 value;
} O14Value110Ref;

typedef struct O14Value114Ref {
    u8 pad00[0x114];
    s32 value;
} O14Value114Ref;

extern s32 gOverlay14Transition;
extern s32 gOverlay14Cursor;
extern O14TransitionD8Ref gOverlay14TransitionD8Base;
extern O14PointerE0Ref gOverlay14PointerE0Base;
extern s32 gOverlay14Flag0;
extern s32 gOverlay14Flag4[];
extern O14ModeE4Ref gOverlay14ModeE4;
extern O14ValueECRef gOverlay14ValueEC;
extern O14StateCCRef gOverlay14StateCC;
extern O14Value110Ref gOverlay14Value110;
extern O14Value114Ref gOverlay14Value114;

extern s32 overlay14InitializeMode(s32 mode);
extern void overlay14ResetMode(void);
extern void overlay14ApplyValues(s32 value, s32 extra);

void overlay14AdvanceCommand(s32 context) {
    s32 divisor;
    s32 limit;
    s32 extra;

    divisor = overlay14InitializeMode(2);
    if (gOverlay14Transition != 0) {
        if (gOverlay14Transition < 0) {
            gOverlay14Transition += 1;
            gOverlay14Cursor -= 1;
        } else {
            gOverlay14Transition -= 1;
            gOverlay14Cursor += 1;
        }
    } else if ((gOverlay14PointerE0Base.value != 0) && (gOverlay14Flag0 != 0)) {
        gOverlay14TransitionD8Base.value = (0x58 / divisor) - 1;
    } else {
        limit = (0x58 / divisor) - 1;
        if ((gOverlay14Cursor >= limit) && (gOverlay14Flag4[1] != 0)) {
            gOverlay14TransitionD8Base.value = -limit;
        }
    }

    if ((gOverlay14PointerE0Base.value == 0) && (gOverlay14Flag0 != 0)) {
        if ((gOverlay14ModeE4.value == 4) && (gOverlay14ValueEC.value >= 2)) {
            overlay14ResetMode();
            return;
        }
        if (gOverlay14ModeE4.value == 3) {
            extra = gOverlay14Value114.value;
            if (extra != 0) {
                overlay14ApplyValues(gOverlay14Value110.value, extra);
                return;
            }
        }
        gOverlay14StateCC.value = 1;
        return;
    }
    if ((gOverlay14Flag4[1] != 0) && (gOverlay14ValueEC.value >= 2)) {
        overlay14ResetMode();
    }
}
