#include "overlays/overlay_037.h"

void overlay37Init(Overlay37Object *object, Overlay37Init *init) {
    Overlay37State *state;

    state = object->state;
    state->x = init->x;
    state->y = init->y;
    state->z = init->z;
    state->target = init->target;
    if (init->target != 0) {
        object->x = init->target->x;
        object->y = init->target->y;
        object->z = init->target->z;
        object->angle = init->target->angle;
    }
}

void overlay37Update(Overlay37Object *object, s32 ticks) {
    Overlay37State *state;
    Overlay37Object *target;
    Overlay37Resource *resource;
    Overlay37Object *callObject;
    f32 period;
    f32 sine;
    s32 savedTicks;

    savedTicks = ticks;
    callObject = object;
    state = object->state;
    target = state->target;
    if (target != 0) {
        resource = (Overlay37Resource *)target->state;
        period = gOverlay37Period;
        object->x = ((Overlay37Resource *)target->state)->x448;
        object->y = resource->y44C;
        object->z = resource->z450;
        object->angle = target->angle;
        state->phase += ((f32)savedTicks / 60.0f) * 10.0f;
        if (period <= state->phase) {
            state->phase -= period;
        }
        sine = func_8002A8C0(
            (s32)((state->phase / period) * 65536.0f));
        callObject->wave = (s32)(4096.0f * sine);
        if (resource->flags1A8 & 1) {
            func_80006EA0(callObject);
        }
    }
}

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

    camera = camGetPtr();
    frame = func_80021964();
    state = object->state;
    if (state->target == NULL) {
        return;
    }
    resource = (Overlay37Resource *)state->target->state;
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
    transform.objectAngle = object->wave;
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

void overlay37RecordMinimum(s32 index, f32 value) {
    Overlay37Record *record;

    record = &gOverlay37Records[index];
    if (record->active == 0) {
        record->active = 1;
        record->distance = value;
        return;
    }
    if (value < record->distance) {
        record->distance = value;
    }
}

s32 overlay37RecordActive(s32 index) {
    return gOverlay37Records[index].active;
}
