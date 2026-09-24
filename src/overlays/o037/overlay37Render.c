#include "PR/ultratypes.h"

typedef struct Overlay37Command {
    u32 w0;
    u32 w1;
} Overlay37Command;

typedef struct Overlay37Camera {
    s16 angle0;
    s16 angle2;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay37Camera;

typedef struct Overlay37Resource {
    s8 selector;
    u8 pad001[0x43B];
    u8 effectSource[0x0C];
    f32 x448;
    f32 y44C;
    f32 z450;
} Overlay37Resource;

typedef struct Overlay37Target {
    u8 pad00[0x64];
    Overlay37Resource *resource;
} Overlay37Target;

typedef struct Overlay37State {
    u8 pad00[0x10];
    Overlay37Target *target;
} Overlay37State;

typedef struct Overlay37Object {
    u8 pad00[4];
    s16 angle4;
    u8 pad06[0x5E];
    Overlay37State *state;
    void **resource;
} Overlay37Object;

typedef struct Overlay37Record {
    s32 active;
    f32 distance;
} Overlay37Record;

typedef struct Overlay37Position {
    f32 x;
    f32 y;
    f32 z;
} Overlay37Position;

typedef struct Overlay37Transform {
    s16 angle0;
    s16 angle2;
    s16 objectAngle;
    u8 pad06[2];
    f32 scale;
    Overlay37Position position;
} Overlay37Transform;

extern Overlay37Record gOverlay37Records[4];
extern u8 gOverlay37DisplayData[];
extern u8 gOverlay37DisplayData78[];

extern Overlay37Camera *overlay37CallProxy(void);
extern s32 func_80021964(void);
extern void func_8002A250(s32 mode, void *source, Overlay37State *state,
                          Overlay37Position *position);
extern void func_800244EC(Overlay37Command **commands, void *renderContext,
                          Overlay37Transform *transform, f32 scale,
                          f32 extra);
extern void func_800349A4(Overlay37Command **commands, void *resource,
                          s32 mode, s32 flags);
extern void func_8002460C(Overlay37Command **commands, const void *displayData);

#ifdef NON_MATCHING
void overlay37RenderEffect(Overlay37Command **commands, void *renderContext,
                           Overlay37Object *object) {
    /* 12 bytes at the frame top, then resource, camera, transform: homes 0x88, 0x84, 0x6C. */
    f32 pad[3];
    Overlay37Resource *resource;
    Overlay37Camera *camera;
    Overlay37Transform transform;
    Overlay37State *state;
    Overlay37Record *record;
    f32 padTail[1];
    f32 distanceDelta;
    Overlay37Command *command;
    s32 frame;
    s32 red;
    s32 green;
    s32 blue;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;

    camera = overlay37CallProxy();
    frame = func_80021964();
    state = object->state;
    if (state->target == NULL) {
        return;
    }
    resource = state->target->resource;
    if ((frame & 3) != resource->selector) {
        return;
    }

    record = &gOverlay37Records[resource->selector];
    record->active = 0;
    red = 0;
    if (record->distance > 2000.0f) {
        green = 0xFF;
        blue = 0;
    } else if ((record->distance > 1000.0f) &&
               (record->distance < 2000.0f)) {
        f32 blend;

        blend = (distanceDelta = record->distance - 1000.0f) / (f32)1000;
        red = 255.0f - blend * (f32)255;
        green = 255.0f - (16.0f - blend * (f32)16);
        blue = 13.0f - blend * (f32)13;
    } else {
        f32 blend;

        blend = record->distance / 1000.0f;
        red = 0xFF;
        green = blend * 239.0f;
        blue = blend * 13.0f;
    }

    func_8002A250(1, resource->effectSource, state, &transform.position);
    transform.position.x += resource->x448;
    transform.position.y += resource->y44C;
    transform.position.z += resource->z450;

    deltaX = camera->x - transform.position.x;
    deltaY = camera->y - transform.position.y;
    deltaZ = camera->z - transform.position.z;
    transform.angle0 = -camera->angle0;
    transform.angle2 = camera->angle2;
    transform.objectAngle = object->angle4;
    transform.scale = 0.5f;
    transform.position.x += deltaX * 0.5f;
    transform.position.y += deltaY * 0.5f;
    transform.position.z += deltaZ * 0.5f;

    func_800244EC(commands, renderContext, &transform, 1.0f, 0.0f);
    func_800349A4(commands, *object->resource, 0x10, 0);

    command = *commands;
    *commands = command + 1;
    command->w1 = (red << 24) | ((green & 0xFF) << 16) |
                  ((blue & 0xFF) << 8) | 0xFF;
    command->w0 = 0xFA000000;

    command = *commands;
    *commands = command + 1;
    command->w0 = (((((u32)gOverlay37DisplayData & 6) | 0x60) & 0xFF) << 16) |
                  0x04000000 | 0x80;
    command->w1 = (u32)gOverlay37DisplayData;

    command = *commands;
    *commands = command + 1;
    command->w1 = (u32)gOverlay37DisplayData78;
    command->w0 = 0x05310040;

    command = *commands;
    *commands = command + 1;
    command->w1 = 0;
    command->w0 = 0xE7000000;

    command = *commands;
    *commands = command + 1;
    command->w1 = 0xFFFFFFFF;
    command->w0 = 0xFA000000;

    func_8002460C(commands, gOverlay37DisplayData);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o037/overlay37Render/func_overlay_037_F000019C_18857BC.s")
#endif

/* PLATEAU-HANDOFF:overlay37RenderEffect:start
 * symbol: overlay37RenderEffect
 * score: 43/214 words
 * frame: 0x98
 * relocations: 12
 * first-mismatch: +0x54
 * summary: hypothesis=command-pointer load before the green reload and transform at +0x6C; spellings=declaration order kept 61 to 43 at delta 0, w0 before w1 stayed 43, if(1) around the prim-color store scored 62; stall=the green reload is still emitted before the command-pointer load
 * PLATEAU-HANDOFF:overlay37RenderEffect:end
 */
