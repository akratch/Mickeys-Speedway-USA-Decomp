#include "PR/ultratypes.h"

typedef struct Overlay47SpawnPacket {
    s16 selector;
    u8 type;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 byte0A;
    u8 byte0B;
    s8 state;
} Overlay47SpawnPacket;

typedef struct Overlay47Spawned {
    s16 state;
    u8 pad02[0x22];
    s32 field24;
    u8 pad28[0x14];
    s32 field3C;
} Overlay47Spawned;

typedef struct Overlay47Object {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[0x14];
    s16 state;
    u8 pad22[2];
    Overlay47Spawned *spawned;
    s16 selectorIndex;
} Overlay47Object;

extern s16 gOverlay47InitializedData[];
extern Overlay47Spawned *func_8000590C(Overlay47SpawnPacket *packet, s32 mode);
extern void func_80005768(Overlay47Spawned *spawned);
extern void func_8005AD64(Overlay47Spawned *spawned, s32 mode, s32 index,
                          f32 value);

void func_overlay_047_F0002D10_1893B28(Overlay47Object *object) {
    Overlay47SpawnPacket packet;

    packet.selector = gOverlay47InitializedData[0x288 + object->selectorIndex];
    packet.type = 0xE;
    packet.x = (s16)object->x;
    packet.y = (s16)object->y;
    packet.z = (s16)object->z;
    packet.state = (s8)object->state;
    packet.byte0B = 0x4B;
    packet.byte0A = 0;
    object->spawned = func_8000590C(&packet, 1);
    if (object->spawned != NULL) {
        func_80005768(object->spawned);
        object->spawned->field3C = 0;
        func_8005AD64(object->spawned, 1, -1, 0.0f);
        object->spawned->state = object->state;
    }
}
