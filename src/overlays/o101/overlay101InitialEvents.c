#include "overlays/overlay_101.h"

typedef char O101InitialIntegerWidth[(sizeof(s32) == 4) ? 1 : -1];
typedef char O101InitialFloatWidth[(sizeof(f32) == 4) ? 1 : -1];
typedef char O101InitialPayloadSize[(sizeof(Overlay101EventValue) == 4) ? 1 : -1];
typedef char O101InitialEventSize[(sizeof(Overlay101Event) == 28) ? 1 : -1];

enum {
    O101_INITIAL_END = 0,
    O101_INITIAL_PAIR = 1,
    O101_INITIAL_ACTIVATE = 3,
    O101_INITIAL_ADVANCE = 4,
    O101_INITIAL_PROMOTE = 5,
    O101_INITIAL_BYTE17 = 6,
    O101_INITIAL_BYTE16 = 7,
    O101_INITIAL_LINKED_PAIR = 8,
    O101_INITIAL_LINKED_PAIR2 = 9,
    O101_INITIAL_COLOR = 14,
    O101_INITIAL_FRAMES = 15,
    O101_INITIAL_GLOBAL_PAIR = 16,
    O101_INITIAL_FINISH = 17
};

/* Fields unused by an action are the actual zero-valued serialized slots,
 * not extra padding records. The dispatcher stops on the type-zero record. */
#define O101_EVENT(t, action, group, key, a, b, p0, p1, seconds) \
    {(t), (action), (group), (key), (a), (b), {(p0)}, {(p1)}, {(seconds)}}
#define O101_PAIR(t, group, x, y, seconds) \
    O101_EVENT(t, O101_INITIAL_PAIR, group, 0, x, y, 0.0f, 0.0f, seconds)
#define O101_ACTIVATE(t, group) \
    O101_EVENT(t, O101_INITIAL_ACTIVATE, group, 0, 0, 0, 0.0f, 0.0f, 0.0f)
#define O101_ADVANCE(t, group) \
    O101_EVENT(t, O101_INITIAL_ADVANCE, group, 0, 0, 0, 0.0f, 0.0f, 0.0f)
#define O101_PROMOTE(t, group) \
    O101_EVENT(t, O101_INITIAL_PROMOTE, group, 0, 0, 0, 0.0f, 0.0f, 0.0f)
#define O101_BYTE17(t, group, value, seconds) \
    O101_EVENT(t, O101_INITIAL_BYTE17, group, 0, value, 0, 0.0f, 0.0f, seconds)
#define O101_BYTE16(t, group, value, seconds) \
    O101_EVENT(t, O101_INITIAL_BYTE16, group, 0, value, 0, 0.0f, 0.0f, seconds)
#define O101_LINKED_PAIR(t, group, key, x, y, seconds) \
    O101_EVENT(t, O101_INITIAL_LINKED_PAIR, group, key, x, y, 0.0f, 0.0f, seconds)
#define O101_LINKED_PAIR2(t, group, key, x, y, seconds) \
    O101_EVENT(t, O101_INITIAL_LINKED_PAIR2, group, key, x, y, 0.0f, 0.0f, seconds)
#define O101_COLOR_PAIR(first, second) (((first) << 16) | (second))
#define O101_COLOR(t, group, key, red, green, blue, alpha, seconds) \
    O101_EVENT(t, O101_INITIAL_COLOR, group, key, \
               O101_COLOR_PAIR(red, green), O101_COLOR_PAIR(blue, alpha), \
               0.0f, 0.0f, seconds)
#define O101_FRAMES(t, group, key, fraction, rate, seconds) \
    O101_EVENT(t, O101_INITIAL_FRAMES, group, key, 0, 0, fraction, rate, seconds)
#define O101_GLOBAL_PAIR(t, x, y, seconds) \
    O101_EVENT(t, O101_INITIAL_GLOBAL_PAIR, 0, 0, x, y, 0.0f, 0.0f, seconds)
#define O101_FINISH(t) \
    O101_EVENT(t, O101_INITIAL_FINISH, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f)
#define O101_END() \
    O101_EVENT(0.0f, O101_INITIAL_END, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f)

/* Tier B: the initial builder selects data +0x340. Its 28-byte dispatcher
 * stride and type-zero stop bound 88 records before the next stream +0xCE0.
 * The table has module lifetime, contains no pointers, and does not alter the
 * separately owned mutable text buffers its existing callbacks consume. */
Overlay101Event o101InitialEvents[88] = {
    O101_GLOBAL_PAIR(0.0f, 0, 0, 0.0f),
    O101_BYTE16(0.0f, 0, 0, 0.0f),
    O101_BYTE17(0.0f, 2, 0, 0.0f),
    O101_BYTE17(0.0f, 4, 0, 0.0f),
    O101_BYTE17(0.0f, 3, 0, 0.0f),
    O101_BYTE17(0.0f, 11, 0, 0.0f),
    O101_LINKED_PAIR2(0.0f, 0, 1, 53, 264, 0.0f),
    O101_LINKED_PAIR2(0.0f, 0, 0, 192, 264, 0.0f),
    O101_BYTE16(0.0f, 0, 255, 2.0f),
    O101_BYTE17(1.0f, 2, 255, 1.0f),
    O101_BYTE17(1.2f, 4, 255, 1.0f),
    O101_BYTE17(1.4f, 3, 255, 1.0f),
    O101_BYTE17(1.6f, 11, 255, 1.0f),
    O101_LINKED_PAIR2(1.6f, 0, 1, 53, 224, 1.0f),
    O101_LINKED_PAIR2(1.6f, 0, 0, 192, 224, 1.0f),
    O101_GLOBAL_PAIR(2.0f, 40, 40, 0.5f),
    O101_PROMOTE(2.5f, 2),
    O101_ACTIVATE(2.5f, 2),
    O101_LINKED_PAIR(3.0f, 2, 0, 4, 14, 6.0f),
    O101_GLOBAL_PAIR(10.0f, 160, 120, 0.75f),
    O101_PROMOTE(11.0f, 3),
    O101_ACTIVATE(11.0f, 3),
    O101_BYTE16(11.0f, 2, 64, 0.5f),
    O101_GLOBAL_PAIR(11.5f, 0, 0, 0.5f),
    O101_GLOBAL_PAIR(16.0f, 40, 80, 0.75f),
    O101_ADVANCE(16.5f, 3),
    O101_ADVANCE(16.5f, 2),
    O101_PROMOTE(17.0f, 4),
    O101_ACTIVATE(17.0f, 4),
    O101_LINKED_PAIR(17.5f, 4, 0, 6, -150, 5.47f),
    O101_ACTIVATE(24.0f, 5),
    O101_ADVANCE(24.5f, 4),
    O101_GLOBAL_PAIR(24.5f, 44, -128, 0.5f),
    O101_PROMOTE(25.0f, 5),
    O101_GLOBAL_PAIR(25.0f, 44, 58, 1.0f),
    O101_PAIR(25.0f, 5, 40, 54, 1.0f),
    O101_GLOBAL_PAIR(26.0f, 0, 0, 0.5f),
    O101_BYTE17(30.0f, 5, 0, 3.0f),
    O101_PROMOTE(31.0f, 6),
    O101_ACTIVATE(31.0f, 6),
    O101_PROMOTE(32.0f, 8),
    O101_ACTIVATE(32.0f, 8),
    O101_PROMOTE(33.0f, 7),
    O101_ACTIVATE(33.0f, 7),
    O101_PROMOTE(34.0f, 9),
    O101_ACTIVATE(34.0f, 9),
    O101_ADVANCE(35.0f, 5),
    O101_ACTIVATE(35.0f, 10),
    O101_GLOBAL_PAIR(35.5f, 80, -158, 0.5f),
    O101_BYTE16(36.0f, 6, 64, 1.0f),
    O101_BYTE16(36.0f, 7, 64, 1.0f),
    O101_BYTE16(36.0f, 8, 64, 1.0f),
    O101_BYTE16(36.0f, 9, 64, 1.0f),
    O101_PROMOTE(36.0f, 10),
    O101_GLOBAL_PAIR(36.0f, 80, 43, 1.0f),
    O101_PAIR(36.0f, 10, 76, 39, 1.0f),
    O101_GLOBAL_PAIR(37.0f, 0, 0, 0.5f),
    O101_BYTE16(40.0f, 10, 64, 2.0f),
    O101_GLOBAL_PAIR(40.0f, 40, 160, 0.75f),
    O101_PROMOTE(41.0f, 1),
    O101_PROMOTE(41.0f, 11),
    O101_LINKED_PAIR2(41.0f, 1, 0, 242, 120, 1.0f),
    O101_ACTIVATE(41.0f, 11),
    O101_FRAMES(41.0f, 11, 3, 0.0f, 1.0f, 0.05f),
    O101_FRAMES(41.0f, 11, 2, 0.0f, 2.55f, 0.05f),
    O101_FRAMES(41.0f, 11, 1, 0.0f, 4.15f, 0.05f),
    O101_FRAMES(41.0f, 11, 0, 0.0f, 5.0f, 0.05f),
    O101_GLOBAL_PAIR(42.0f, 0, 0, 0.5f),
    O101_COLOR(42.0f, 11, 0, 255, 128, 0, 255, 1.0f),
    O101_COLOR(42.0f, 11, 1, 255, 240, 0, 255, 1.0f),
    O101_COLOR(42.0f, 11, 2, 255, 248, 0, 255, 1.0f),
    O101_COLOR(42.0f, 11, 3, 255, 255, 0, 255, 1.0f),
    O101_LINKED_PAIR2(49.5f, 1, 0, 242, 334, 0.5f),
    O101_ADVANCE(50.0f, 6),
    O101_ADVANCE(50.0f, 7),
    O101_ADVANCE(50.0f, 8),
    O101_ADVANCE(50.0f, 9),
    O101_ADVANCE(50.0f, 10),
    O101_ADVANCE(50.0f, 11),
    O101_BYTE16(51.0f, 0, 0, 2.0f),
    O101_LINKED_PAIR2(51.0f, 0, 1, 53, 264, 1.0f),
    O101_LINKED_PAIR2(51.0f, 0, 0, 192, 264, 1.0f),
    O101_BYTE17(51.0f, 11, 0, 1.0f),
    O101_BYTE17(51.2f, 3, 0, 1.0f),
    O101_BYTE17(51.4f, 4, 0, 1.0f),
    O101_BYTE17(51.6f, 2, 0, 1.0f),
    O101_FINISH(52.5f),
    O101_END(),
};

typedef char O101InitialStreamSize[(sizeof(o101InitialEvents) == 0x9A0) ? 1 : -1];
