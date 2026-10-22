#include "PR/ultratypes.h"

typedef union Overlay101EventValue {
    s32 i;
    f32 f;
} Overlay101EventValue;

typedef struct Overlay101Event {
    f32 time;
    u16 type;
    u8 index;
    u8 key;
    s32 value8;
    s32 valueC;
    Overlay101EventValue value10;
    Overlay101EventValue value14;
    Overlay101EventValue value18;
} Overlay101Event;

extern Overlay101Event *D_220;
extern s32 D_224;
extern s32 D_228;
extern s32 D_218;
extern u8 D_0;

#define gOverlay101Events D_220
#define gOverlay101EventIndex D_224
#define gOverlay101EventClock D_228
#define gOverlay101PresentationDone D_218

extern void overlay101SchedulePair(s32, s32, s32, f32, s32);
extern void overlay101SchedulePair12(s32, s32, s32, f32, s32);
extern void overlay101ActivateSlot(s32);
extern void overlay101AdvanceSlot(s32);
extern void overlay101PromoteSlot(s32);
extern void overlay101ScheduleByte17(s32, s32, f32, s32);
extern void overlay101ScheduleByte16(s32, s32, f32, s32);
extern void overlay101ScheduleLinkedPair(s32, s32, s32, s32, f32, s32);
extern void overlay101ScheduleLinkedPair2(s32, s32, s32, s32, f32, s32);
extern void overlay101ScheduleLinkedFloat(s32, s32, f32, f32, s32);
extern void overlay101ScheduleLinkedScaled(s32, s32, f32, f32, s32);
extern void overlay101ScheduleLinkedByte(s32, s32, s32, f32, s32);
extern void overlay101ScheduleLinkedPair3(s32, s32, s32, s32, f32, s32);
extern void overlay101ScheduleLinkedColor(s32, s32, s32, s32, s32, s32,
                                           f32, s32);
extern void overlay101ScheduleFrames(s32, s32, f32, f32, f32, s32);
extern void overlay101ScheduleGlobalPair();
extern void overlay101Initialize(void);

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101DispatchEvents(s32 step) {
    Overlay101Event *events;
    Overlay101Event *event;
    s32 remaining;
    s32 delay;
    s32 finished;
    s32 packed;

    events = gOverlay101Events;
    if (events == 0) {
        return;
    }
    if ((s16)events[gOverlay101EventIndex].type == 0) {
        return;
    }
    gOverlay101EventClock += step;
    finished = 0;
    do {
        event = &gOverlay101Events[gOverlay101EventIndex];
        delay = (s32)(event->time * 60.0f);
        remaining = gOverlay101EventClock - delay;
        if (remaining >= 0) {
            switch ((u16)event->type) {
                case 0:
                    break;
                case 1:
                    overlay101SchedulePair(event->index, event->value8,
                        event->valueC, event->value18.f, remaining);
                    break;
                case 2:
                    overlay101SchedulePair12(event->index, event->value8,
                        event->valueC, event->value18.f, remaining);
                    break;
                case 3:
                    overlay101ActivateSlot(event->index);
                    break;
                case 4:
                    overlay101AdvanceSlot(event->index);
                    break;
                case 5:
                    overlay101PromoteSlot(event->index);
                    break;
                case 6:
                    overlay101ScheduleByte17(event->index, event->value8,
                        event->value18.f, remaining);
                    break;
                case 7:
                    overlay101ScheduleByte16(event->index, event->value8,
                        event->value18.f, remaining);
                    break;
                case 8:
                    overlay101ScheduleLinkedPair(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 9:
                    overlay101ScheduleLinkedPair2(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 10:
                    overlay101ScheduleLinkedFloat(event->index, event->key,
                        event->value10.f, event->value18.f, remaining);
                    break;
                case 11:
                    overlay101ScheduleLinkedScaled(event->index, event->key,
                        event->value10.f, event->value18.f, remaining);
                    break;
                case 12:
                    overlay101ScheduleLinkedByte(event->index, event->key,
                        event->value8, event->value18.f, remaining);
                    break;
                case 13:
                    overlay101ScheduleLinkedPair3(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 14:
                    packed = event->valueC;
                    overlay101ScheduleLinkedColor(event->index, event->key,
                        event->value8 >> 16, event->value8 & 0xFF,
                        packed >> 16, packed & 0xFF, event->value18.f, remaining);
                    break;
                case 15:
                    overlay101ScheduleFrames(event->index, event->key,
                        event->value10.f, event->value14.f, event->value18.f,
                        remaining);
                    break;
                case 16:
                    overlay101ScheduleGlobalPair(event->value8, event->valueC,
                        event->value18.i, remaining);
                    break;
                case 17:
                    overlay101Initialize();
                    gOverlay101PresentationDone = 1;
                    break;
                case 18:
                    D_0 = 1;
                    break;
            }
            gOverlay101EventIndex++;
        } else {
            finished = 1;
        }
    } while (finished == 0);
}
