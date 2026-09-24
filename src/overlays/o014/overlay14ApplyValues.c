#include "PR/ultratypes.h"

typedef struct Overlay14QueuedCommand {
    s16 value;
    s16 mode;
    s32 result;
} Overlay14QueuedCommand;

typedef struct O14PendingValueD0Ref {
    u8 pad00[0xD0];
    s32 value;
} O14PendingValueD0Ref;

typedef struct O14PendingModeD4Ref {
    u8 pad00[0xD4];
    s32 value;
} O14PendingModeD4Ref;

typedef struct O14TransitionD8Ref {
    u8 pad00[0xD8];
    s32 value;
} O14TransitionD8Ref;

typedef struct O14CursorDCRef {
    u8 pad00[0xDC];
    s32 value;
} O14CursorDCRef;

typedef struct O14PointerE0Ref {
    u8 pad00[0xE0];
    s32 value;
} O14PointerE0Ref;

typedef struct O14ModeE4Ref {
    u8 pad00[0xE4];
    s32 value;
} O14ModeE4Ref;

typedef struct O14ResultFCRef {
    u8 pad00[0xFC];
    s32 value;
} O14ResultFCRef;

typedef struct O14Value108Ref {
    u8 pad00[0x108];
    s32 value;
} O14Value108Ref;

extern s32 gOverlay14StateC8;
extern O14PendingValueD0Ref gOverlay14PendingValueD0Base;
extern O14PendingModeD4Ref gOverlay14PendingModeD4Base;
extern O14TransitionD8Ref gOverlay14TransitionD8Base;
extern O14CursorDCRef gOverlay14CursorDCBase;
extern O14PointerE0Ref gOverlay14PointerE0Base;
extern O14ModeE4Ref gOverlay14ModeE4;
extern s32 gOverlay14CommandCountEC;
extern s32 gOverlay14ResultF8;
extern O14ResultFCRef gOverlay14ResultFCBase;
extern O14Value108Ref gOverlay14Value108;
extern Overlay14QueuedCommand gOverlay14QueuedCommands128[];

extern s32 overlay14CreateValue(s32 value, s32 mode);
extern s32 overlay14MoveCommandCursor(s32 step);

s32 overlay14ApplyValues(s32 value, s32 mode) {
    s32 result;
    s32 index;
    Overlay14QueuedCommand *command;

    result = 0;
    if (gOverlay14StateC8 != 0) {
        gOverlay14PendingValueD0Base.value = value;
        gOverlay14PendingModeD4Base.value = mode;
        return 0;
    }

    gOverlay14PendingValueD0Base.value = -1;
    gOverlay14PendingModeD4Base.value = -1;
    if (gOverlay14CommandCountEC < 0x10) {
        if (mode == 2) {
            gOverlay14Value108.value = value;
            gOverlay14StateC8 = 2;
        } else if (mode == 1) {
            gOverlay14ResultFCBase.value = overlay14CreateValue(value, 1);
            if (overlay14MoveCommandCursor(0) != 0) {
                gOverlay14ModeE4.value = 1;
                result = gOverlay14ResultFCBase.value;
            }
        } else {
            gOverlay14ResultF8 = overlay14CreateValue(value, 4);
            gOverlay14TransitionD8Base.value = 0;
            gOverlay14CursorDCBase.value = 0;
            gOverlay14PointerE0Base.value = 0;
            gOverlay14ModeE4.value = mode;
            result = gOverlay14ResultF8;
        }

        if (result != 0) {
            index = gOverlay14CommandCountEC;
            command = &gOverlay14QueuedCommands128[index];
            command->value = value;
            command->mode = mode;
            command->result = result;
            gOverlay14CommandCountEC = index + 1;
        }
    }
    return result;
}
