#include "overlays/overlay_071.h"

/* Mickey-local reconstruction; no external donor body was used. */
void func_overlay_071_F0000000_18C9B20(O71Object *object, O71Input *input,
                                       s32 preserveState) {
    O71State *data;
    O71Vertex *color;
    s32 group;
    s32 index;
    f32 scale;

    object->value00 = input->value22;
    data = object->state;
    object->value02 = input->value20;
    object->value04 = input->value1F << 6;
    object->value08 = gOverlay71Scale0;

    data->centerX = (f32)input->value0E * 10.0f;
    data->centerY = (f32)input->value10 * 10.0f;
    data->centerZ = (f32)input->value12 * 10.0f;
    data->extentX = (f32)input->value0A * 10.0f;
    data->extentY = (f32)input->value0C * 10.0f;
    data->radius = (f32)input->value1A * 10.0f;
    scale = gOverlay71Scale1;
    data->scaleX = (f32)input->value1C * scale;
    data->scaleY = (f32)input->value1D * scale;
    data->lowerLimit = (f32)input->value14 * scale;
    data->upperLimit = (f32)input->value15 * scale;
    data->red = input->value16;
    data->green = input->value17;
    data->blue = input->value18;
    data->alpha = input->value19;
    data->flags = input->value1E;

    for (group = 0; group < 2; group++) {
        index = 0;
        color = data->vertices[group];
        do {
            if (index < 4) {
                color->red = 0xFF;
                color->green = 0xFF;
                color->blue = 0xFF;
                color->alpha = input->value19;
            } else {
                color->red = input->value16;
                color->green = input->value17;
                color->blue = input->value18;
                color->alpha = 0;
            }
            color++;
            index++;
        } while (index != 8);
    }
    if (preserveState == 0) {
        data->bank = 0;
        data->status = 0;
    }
    func_overlay_071_F0000278_18C9D98(object);
}

/* Declaration order preserves the retail automatic stack homes. */
void func_overlay_071_F0000278_18C9D98(O71Object *object) {
    s32 i;
    f32 halfX;
    f32 halfY;
    f32 factor;
    f32 distanceSquared;
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 count;
    O71Vec3 *point;
    O71Vec3 points[8];
    O71State *state;
    O71Vertex *vertex;

    SetLinkSlot(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    state->bank ^= 1;
    state->status = 0;

    halfX = state->extentX * state->scaleX * 0.5f;
    halfY = state->extentY * state->scaleY * 0.5f;
    if ((halfX != 0.0f) && (halfY != 0.0f)) {
        D_80079274 = 1;
        points[0].x = -halfX;
        points[0].y = halfY;
        points[0].z = 0.0f;
        points[1].x = halfX;
        points[1].y = halfY;
        points[1].z = 0.0f;
        points[2].x = halfX;
        points[2].y = -halfY;
        points[2].z = 0.0f;
        points[3].x = -halfX;
        points[3].y = -halfY;
        points[3].z = 0.0f;

        state->status |= state->flags & 1;
        if (state->flags & 2) {
            factor = state->scaleX * state->scaleY;
            if (state->lowerLimit <= factor) {
                factor = (factor - state->lowerLimit) /
                         (state->upperLimit - state->lowerLimit);
                if (factor > 1.0f) {
                    factor = 1.0f;
                }
                radius = state->radius * factor;
                i = 0;
                point = points;
                while (i < 4) {
                    dx = point->x - state->centerX;
                    dy = point->y - state->centerY;
                    dz = point->z - state->centerZ;
                    distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);
                    factor = distanceSquared;
                    if (distanceSquared != 0.0f) {
                        factor = radius / sqrtf(distanceSquared);
                    }
                    point[4].x = point->x + (dx * factor);
                    point[4].y = point->y + (dy * factor);
                    point[4].z = point->z + (dz * factor);
                    point++;
                    i++;
                }
                state->status |= state->flags & 6;
            }
        }

        count = 4;
        if (state->status & 2) {
            count = 8;
        }
        vertex = state->vertices[state->bank];
        i = 0;
        point = points;
        while (i < count) {
            vertex->x = point->x;
            vertex->y = point->y;
            vertex->z = point->z;
            if (object->active != 0) {
                if (i >= 4) {
                    vertex->red = state->red;
                    vertex->green = state->green;
                    vertex->blue = state->blue;
                } else {
                    vertex->alpha = state->alpha;
                }
            }
            vertex++;
            point++;
            i++;
        }
    }
    object->active = 0;
}

/* DKR v77/v80 and JFG contain no exact donor for this coordinate updater. */
void overlay71UpdateCoordinates(s32 amount) {
    Overlay71SourceCoordinate *coordinate;
    Overlay71CoordinateRecord *record;
    register s32 previous;
    s16 remaining;
    s16 group;

    gOverlay71Offset = (gOverlay71Offset + (amount * 2)) & 0x3FF;
    group = 0; do {
        if (group == 0) {
            record = gOverlay71PrimaryRecords;
            coordinate = gOverlay71PrimaryCoordinates;
        } else {
            record = gOverlay71SecondaryRecords;
            coordinate = gOverlay71SecondaryCoordinates;
        }
        previous = 7;
        remaining = previous;
        do {
            record->x = coordinate->x + gOverlay71Offset;
            record->y = coordinate->y + gOverlay71Offset;
            record->z = coordinate->z + gOverlay71Offset;
            coordinate++;
            record++;
            previous = remaining;
            remaining--;
        } while (previous);
        group++;
    } while (group < 3);
}

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDPNoParam, gDPSetColor, gDPSetPrimColor
 * and gDma1p, include/PR/mbi.h _SHIFTL, include/f3ddkr.h gSPVertexJFG and
 * gSPPolygon, include/PR/os_convert.h OS_PHYSICAL_TO_K0), a permitted source
 * under docs/CLEANROOM.md.  No function body was imported; DKR v77/v80 and
 * JFG contain no exact donor for this renderer.
 *
 * Matched 2026-10-01 by writing every display-list command as its macro.
 * Each expansion declares its own block-scoped `_g`, so the twelve command
 * sites are twelve separate symbols instead of the two shared pointer locals
 * (`command`, `head`) the earlier candidate carried, and the `u16 flags`
 * carrier, the `if (1)` region and the hand-folded store lines are gone: a
 * macro is one physical line already.  With those webs in place all four
 * `state->status` loads take the shipped register with no further lever.
 */
#define O71_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O71_NO_PARAM(pkt, cmd) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = O71_SHIFTL(cmd, 24, 8); _g->w1 = 0; }

#define O71_SET_COLOR(pkt, c, d) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = O71_SHIFTL(c, 24, 8); _g->w1 = (u32)(d); }

#define O71_RGBA(r, g, b, a) (O71_SHIFTL(r, 24, 8) | O71_SHIFTL(g, 16, 8) | O71_SHIFTL(b, 8, 8) | O71_SHIFTL(a, 0, 8))

#define O71_PIPE_SYNC(pkt) O71_NO_PARAM(pkt, 0xE7)

#define O71_SET_ENV_COLOR(pkt, r, g, b, a) O71_SET_COLOR(pkt, 0xFB, O71_RGBA(r, g, b, a))

#define O71_SET_PRIM_COLOR(pkt, m, l, r, g, b, a) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = (O71_SHIFTL(0xFA, 24, 8) | O71_SHIFTL(m, 8, 8) | O71_SHIFTL(l, 0, 8)); _g->w1 = O71_RGBA(r, g, b, a); }

#define O71_DMA1P(pkt, c, s, l, p) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = (O71_SHIFTL((c), 24, 8) | O71_SHIFTL((p), 16, 8) | O71_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }

#define O71_VERTEX(pkt, v, n, v0) O71_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))

#define O71_POLYGON(dl, ptr, numTris, texEnabled) { Overlay71Command *_g = (Overlay71Command *)(dl); _g->w0 = O71_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O71_SHIFTL(0x05, 24, 8) | O71_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }

#define O71_PHYSICAL_TO_K0(x) (void *)(((u32)(x) + 0x80000000))

#define O71_VERTEX_BANK(state) O71_PHYSICAL_TO_K0((u32)(state) + (state)->bank * 0x50)

void func_overlay_071_F0000870_18CA390(Overlay71Command **commands,
                                       void *context,
                                       O71Object *object) {
    O71State *state;

    SetLinkSlot(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    if (state->flags != 0) {
        camPushModelMtx(commands, context, object, 1.0f, 0.0f);
        O71_PIPE_SYNC((*commands)++);
        O71_SET_ENV_COLOR((*commands)++, 255, 255, 255, 255);
        if (state->status & 1) {
            texDPTextureX(commands, 0, 0x17, 0);
            O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 0xD0);
            O71_VERTEX((*commands)++, O71_VERTEX_BANK(state), 4, 0);
            O71_POLYGON((*commands)++, D_80000008, 2, 0);
            O71_PIPE_SYNC((*commands)++);
        }
        if (state->status & 6) {
            texDPTextureX(commands, *object->resourceIndex, 0x17, 0);
            O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 255);
            O71_VERTEX((*commands)++, O71_VERTEX_BANK(state), 8, 0);
            if (state->status & 4) {
                O71_POLYGON((*commands)++, D_800000D8, 8, 1);
            }
            if (state->status & 2) {
                O71_POLYGON((*commands)++, D_80000028, 8, 1);
            }
            O71_PIPE_SYNC((*commands)++);
        }
        O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 255);
        camPopModelMtx(commands);
    }
}
