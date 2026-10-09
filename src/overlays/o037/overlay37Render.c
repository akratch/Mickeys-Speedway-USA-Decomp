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
extern void texDPTextureX(Overlay37Command **commands, void *resource,
                          s32 mode, s32 flags);
extern void func_8002460C(Overlay37Command **commands);

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDPNoParam, gDPSetPrimColor and gDma1p,
 * include/PR/mbi.h _SHIFTL, include/f3ddkr.h gSPVertexJFG and gSPPolygon), a
 * permitted source under docs/CLEANROOM.md.  No function body was imported.
 *
 * Matched 2026-10-01 by rewriting three inherited parts of the candidate.
 *   - Every display-list command is its packet macro taking (*commands)++,
 *     so each site has its own block-scoped pointer instead of one shared
 *     `command` local.
 *   - The closing call takes the command cursor only.  The resident callee
 *     pops a model matrix and has one parameter; the second argument the
 *     candidate passed was the vertex address still sitting in a1 from the
 *     vertex command.  Passing it made uopt either rebuild the address (two
 *     words long) or carry it in the wrong register.
 *   - The locals are declared in frame order: frame, state and record above
 *     resource, camera and the transform, then the deltas, the two blend
 *     scalars and one unused word above the three colour components.  The
 *     macro pointers take the five cells below them, which is what the two
 *     pad arrays were standing in for.
 * The integer literals in the middle blend arm keep its constants apart from
 * the float literals of the same value (the pool is keyed on spelling).
 */
#define O37_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O37_NO_PARAM(pkt, cmd) { Overlay37Command *_g = (Overlay37Command *)(pkt); _g->w0 = O37_SHIFTL(cmd, 24, 8); _g->w1 = 0; }
#define O37_RGBA(r, g, b, a) (O37_SHIFTL(r, 24, 8) | O37_SHIFTL(g, 16, 8) | O37_SHIFTL(b, 8, 8) | O37_SHIFTL(a, 0, 8))
#define O37_PIPE_SYNC(pkt) O37_NO_PARAM(pkt, 0xE7)
#define O37_SET_PRIM_COLOR(pkt, m, l, r, g, b, a) { Overlay37Command *_g = (Overlay37Command *)(pkt); _g->w0 = (O37_SHIFTL(0xFA, 24, 8) | O37_SHIFTL(m, 8, 8) | O37_SHIFTL(l, 0, 8)); _g->w1 = O37_RGBA(r, g, b, a); }
#define O37_DMA1P(pkt, c, s, l, p) { Overlay37Command *_g = (Overlay37Command *)(pkt); _g->w0 = (O37_SHIFTL((c), 24, 8) | O37_SHIFTL((p), 16, 8) | O37_SHIFTL((l), 0, 16)); _g->w1 = (u32)(s); }
#define O37_VERTEX(pkt, v, n, v0) O37_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O37_POLYGON(dl, ptr, numTris, texEnabled) { Overlay37Command *_g = (Overlay37Command *)(dl); _g->w0 = O37_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O37_SHIFTL(0x05, 24, 8) | O37_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }

void overlay37RenderEffect(Overlay37Command **commands, void *renderContext,
                           Overlay37Object *object) {
    s32 frame;
    Overlay37State *state;
    Overlay37Record *record;
    Overlay37Resource *resource;
    Overlay37Camera *camera;
    Overlay37Transform transform;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distanceDelta;
    f32 blend;
    s32 pad;
    s32 red;
    s32 green;
    s32 blue;

    camera = overlay37CallProxy();
    frame = func_80021964();
    state = object->state;
    if (state->target == NULL) {
        return;
    }
    resource = state->target->resource;
    if (resource->selector != (frame & 3)) {
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
        distanceDelta = record->distance - 1000.0f;
        blend = distanceDelta / 1000;
        red = 255.0f - blend * 255;
        green = 255.0f - (16.0f - blend * 16);
        blue = 13.0f - blend * 13;
    } else {
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
    texDPTextureX(commands, *object->resource, 0x10, 0);

    O37_SET_PRIM_COLOR((*commands)++, 0, 0, red, green, blue, 255);
    O37_VERTEX((*commands)++, gOverlay37DisplayData, 12, 0);
    O37_POLYGON((*commands)++, gOverlay37DisplayData78, 4, 1);
    O37_PIPE_SYNC((*commands)++);
    O37_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 255);

    func_8002460C(commands);
}
