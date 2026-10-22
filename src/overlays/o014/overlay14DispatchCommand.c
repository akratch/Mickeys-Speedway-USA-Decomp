#include "PR/ultratypes.h"

typedef struct Overlay14CommandHeader {
    u8 pad00[2];
    s16 activeIndex;
} Overlay14CommandHeader;

typedef struct Overlay14Command {
    u8 pad00[6];
    s16 checkValue;
    s16 encodedAction;
    s16 fallbackAction;
    s16 trailingAction;
    u8 pad0E[2];
} Overlay14Command;

extern Overlay14CommandHeader *gOverlay14CommandHeader;
extern s32 gOverlay14FlagC4;
extern s32 gOverlay14FlagC8;
extern s32 gOverlay14ActiveHandle;

extern s32 overlay14ReturnOneCallbackB(s16 value);
extern void overlay14CallUpdate(s32 value);
extern void overlay14ApplyValues(s32 value, s32 mode);
extern void overlay14ResetMode(void);

void overlay14DispatchCommand(void) {
    Overlay14Command *command;
    Overlay14CommandHeader *header;
    s32 encoded;
    s32 kind;
    s32 action;

    header = gOverlay14CommandHeader;
    if (header != NULL) {
        command = (Overlay14Command *)((u8 *)header +
            (header->activeIndex * 0x10) + 4);
        if (overlay14ReturnOneCallbackB(command->checkValue) != 0) {
            encoded = command->encodedAction;
            kind = encoded & 0xF000;
            switch (kind) {
                case 0:
                    overlay14CallUpdate(encoded);
                    break;
                case 0x1000:
                    overlay14ApplyValues(encoded & 0xFFF, 4);
                    break;
                case 0x2000:
                    overlay14ApplyValues(encoded & 0xFFF, 1);
                    break;
                case 0x3000:
                    gOverlay14ActiveHandle = encoded & 0xFFF;
                    gOverlay14FlagC8 = 2;
                    break;
                case 0x4000:
                    overlay14ResetMode();
                    break;
                default:
                    gOverlay14FlagC4 = 0;
                    break;
            }
            action = command->trailingAction;
            if (action != -1) {
                overlay14ApplyValues(action, 4);
            }
        } else {
            action = command->fallbackAction;
            if (action != -1) {
                overlay14ApplyValues(action, 4);
            }
        }
    }
}
