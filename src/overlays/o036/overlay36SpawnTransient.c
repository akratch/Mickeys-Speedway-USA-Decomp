#include "PR/ultratypes.h"

typedef struct Overlay36Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay36Position;

typedef struct Overlay36SpawnDescriptor {
    s16 type;
    u8 value2;
    u8 value3;
    s16 x;
    s16 y;
    s16 z;
} Overlay36SpawnDescriptor;

typedef struct Overlay36SpawnState {
    u8 pad00[4];
    s16 value4;
} Overlay36SpawnState;

typedef struct Overlay36Spawned {
    u8 pad00[0x3C];
    s32 value3C;
    u8 pad40[0x24];
    Overlay36SpawnState *state;
} Overlay36Spawned;

typedef struct Overlay36World {
    u8 pad000[0x192];
    u8 timer192;
    u8 pad193[7];
    u8 inactive19A;
    u8 timer19B;
    u8 pad19C[4];
    s16 *type1A0;
} Overlay36World;

extern s32 gOverlay36Mode;
extern Overlay36Spawned *func_8000590C(Overlay36SpawnDescriptor *desc,
                                              s32 one,
                                              Overlay36World *world);

Overlay36Spawned *overlay36SpawnTransient(
    Overlay36Position *position, Overlay36World *world) {
    Overlay36SpawnDescriptor desc;
    Overlay36Spawned *spawned;
    Overlay36SpawnState *state;
    volatile s32 framePad[1];

    desc.type = *world->type1A0;
    desc.value2 = 10;
    desc.value3 = 0;
    desc.x = (s16)(s32)position->x;
    desc.y = (s16)(s32)(position->y + 30.0f);
    desc.z = (s16)(s32)position->z;

    spawned = func_8000590C(&desc, 1, world);
    if (spawned != 0) {
        spawned->value3C = 0;
        state = spawned->state;
        if (gOverlay36Mode == 3) {
            state->value4 = 0xF0;
        } else {
            state->value4 = 0x5A;
        }
        if (*world->type1A0 == 0x1B) {
            world->timer192--;
            if (world->timer192 == 0) {
                world->inactive19A = 0xFF;
                world->type1A0 = 0;
            }
        } else {
            world->timer19B--;
            if (world->timer19B == 0) {
                world->inactive19A = 0xFF;
                world->type1A0 = 0;
            }
        }
    }
    return spawned;
}
