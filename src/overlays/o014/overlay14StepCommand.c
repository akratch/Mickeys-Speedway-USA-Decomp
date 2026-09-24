#include "PR/ultratypes.h"

typedef struct Overlay14CommandHeader {
    s16 count;
    s16 cursor;
} Overlay14CommandHeader;

extern s32 gOverlay14Flag4[];
extern s32 gOverlay14Flag0[];
extern s32 gOverlay14FlagC[];
extern s32 gOverlay14Flag10[];
extern Overlay14CommandHeader *gOverlay14CommandHeaderBase[];

extern void overlay14ResetMode(void);
extern void overlay14DispatchCommand(void);
extern void overlay14MoveCommandCursor(s32 step);

void overlay14StepCommand(s32 context) {
    Overlay14CommandHeader *command;

    if (gOverlay14Flag4[1] != 0) {
        overlay14ResetMode();
        return;
    }
    if (gOverlay14Flag0[0] != 0) {
        overlay14DispatchCommand();
        return;
    }
    if ((gOverlay14FlagC[3] != 0) && (gOverlay14CommandHeaderBase[0x3F]->cursor > 0)) {
        overlay14MoveCommandCursor(-1);
        return;
    }
    if (gOverlay14Flag10[4] != 0) {
        command = gOverlay14CommandHeaderBase[0x3F];
        if (command->cursor < (command->count - 1)) {
            overlay14MoveCommandCursor(1);
        }
    }
}
