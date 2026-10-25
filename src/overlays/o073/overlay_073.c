#include "overlays/overlay_073.h"

/* DKR v77/v80 and JFG contain no exact donor for this initializer. */
/* Fresh configured full-TU and isolated canonical V0 objects are text-identical
 * at 90/100 raw and 92/100 runtime-relocation-normalized words, with exact
 * 100-word extent and frame 0x8. The first mismatch is +0x40; all eight
 * codegen residual words are one reciprocal a2/a3 allocation swap at
 * +0x40/+0x50/+0x80/+0x84/+0x94/+0x9C/+0xA8/+0xB0. All 119 flag-lattice
 * configurations were attempted: 53 compiled, thirteen O2/MIPS-II rows tied
 * with V0, and none was exact. A fidelity-clean procedure-0 trace shows the
 * loop/resource-index web selecting a2 while a3 has the same zero cost. The
 * diagnostic force to a3 clears every normalized site and cascades the paired
 * web back to a2, proving the cause but never a promotable result. Naturally
 * initializing the shared index before the preserve-state branch regressed to
 * 55/100 raw and normalized words and shifted the first LOCAL pair, so no
 * strict gain authorized a scoped form or batch. The 122-stock-build cap is
 * exhausted. Runtime table 2 resolves
 * D_80 through LOCAL base
 * +0xEB0/addend +0x80 (module +0xF30), while D_0/D_4/D_8 use LOCAL base
 * +0xF80 and addends 0/+4/+8. The 16 zero-immediate calls labelled with this
 * module-zero symbol in overlay_073.s are outgoing SYMBOL relocations, not
 * callers. ORT 1248 exports +0; resident relocation 90 at
 * func_8000AA38+0x42C is the sole inbound. Fresh relocation-surface comparison
 * reports target/candidate 8/8, offset/type 8/8, identity 8/8, and all eight
 * resolved. Owned +0x0..+0x190 / ROM 0x18CAAC0..0x18CAC50 has no target
 * padding; the next assembly owner starts at +0x190. Ordinary full-TU/link
 * equality remains fallback-only and no linked C proof exists. */
void func_overlay_073_F0000000_18CAAC0(Overlay73Object *object, Overlay73Header *header, s32 preserveState)
{
  s32 i;
  Overlay73State *state;
  Overlay73Vertex *vertex;
  s16 (*source)[3];
  state = object->state;
  if (preserveState == 0)
  {
    object->velocityY = 0.0f;
    state->flags = 0;
    state->target = 0;
    state->countdown = 0;
    state->mode = preserveState * 0;
    state->angle = 0x180;
    state->timer = 0.0f;
    vertex = state->vertices;
    source = D_80;
    i = 11;
    do
    {
      vertex->x = (*source)[0];
      vertex->y = (*source)[1];
      vertex->z = (*source)[2];
      vertex->r = 0xFF;
      vertex->g = 0xFF;
      vertex->b = 0xFF;
      vertex->a = 0xFF;
      vertex++;
      source++;
    }
    while (i--);
    state->vertexBank = 1;
  }
  i = header->resourceIndex;
  if (i >= object->config->resourceCount)
  {
    i = 0;
  }
  state->resource = object->resources[i];
  state->x = header->x;
  state->y = header->y;
  state->z = header->z;
  state->radius = header->scale;
  object->scale = ((f32) header->outputScale) * 0.01f;
  if (object->output != ((void *) 0))
  {
    object->output[0] = object->scale * 6.4f;
    object->output[1] = object->scale * 3.2f;
  }
}


/* Overlay 73 updater. Matched 2026-10-09 (lane s-1): the case 4 query keeps
 * one count, `if ((hitCount = query(...)) != 0)`, replaces it by mathRnd's
 * pick and decrements it in place before indexing. The count's two live
 * pieces (the mathRnd argument and the join) are what narrow the call result
 * twice; the narrowing of the dead piece after mathRnd is the ring draw that
 * leaves no instruction. The entry step is multiplied in place, so it is
 * narrowed once; the case 0 timer add goes through
 * the float `limit` local so the rate product is numbered before the timer
 * read (the float-rate spill at +0x34). `unused` holds the 0x98 frame. */
void func_overlay_073_F0000190_18CAC50(Overlay73Object *object, s32 updateRate) {
    s32 horizontal;
    s32 vertical;
    s32 phase;
    f32 height;
    s16 absStep;
    s16 delta;
    f32 dx;
    f32 dy;
    f32 dz;
    s16 targetAngle;
    f32 limit;
    s16 hitCount;
    s32 unused;
    Overlay73TargetData *data;
    Overlay73Target *target;
    Overlay73Vertex *vertex;
    Overlay73State *state;
    struct HitCopyState *hits[8];

    state = object->state;

    absStep = state->angle;
    absStep *= updateRate;
    if (absStep < 0) {
        absStep = -absStep;
    }

    switch (state->mode) {
    case 0: {
        state->timer = 0;
        if ((s16)func_8005776C(state->x, 0.0f, state->z, state->radius,
                          1, hits) != 0) {
            state->target = hits[0];
            state->countdown = 0xF0;
            state->mode = 3;
        }
        limit = (f32)updateRate * 0.004f;
        object->timer += limit;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 1:
    case 2: {

        if (state->angle < 0x480) {
            state->angle += updateRate * 0x10;
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        targetAngle = Arctanf(dx, dz);
        delta = targetAngle - object->angle;
        if (delta < 0) {
            if (-absStep < delta) {
                object->angle += delta;
            } else {
                object->angle -= absStep;
            }
        } else if (delta > 0) {
            if (delta < absStep) {
                object->angle += delta;
            } else {
                object->angle += absStep;
            }
        }

        height = state->y - object->y;
        if (height < 0) {
            object->velocityY -= 0.1f * (f32)updateRate;
            if (object->velocityY < height) {
                object->velocityY = height;
            }
        } else if (height > 0) {
            object->velocityY += 0.1f * (f32)updateRate;
            if (height < object->velocityY) {
                object->velocityY = height;
            }
        } else {
            object->velocityY = 0;
        }

        if (state->mode == 1 &&
            (s16)func_8005776C(state->x, 0, state->z, state->radius,
                          1, hits) != 0) {
            state->target = hits[0];
            state->countdown = 0x78;
            state->mode = 3;
        }

        if (state->mode != 3) {
            dy = object->y - state->y;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 4.0f) {
                func_80008118();
                func_80008128(object, -dx, -dy, -dz);
                object->velocityY = 0;
                state->mode = 0;
                state->timer = 0.0f;
            }
        }

        object->timer += 0.064f * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 3: {

        if (state->angle < 0x480) {
            state->angle += updateRate * 0x10;
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        data = ((Overlay73Target *)state->target)->data;
        dx = data->x - state->x;
        dz = data->z - state->z;
        limit = state->radius * state->radius * 4.0f;
        if (limit < ((dx * dx) + (dz * dz))) {
            state->target = NULL;
            if ((s16)func_8005776C(state->x, 0.0f, state->z, state->radius,
                              1, hits) != 0) {
                state->target = hits[0];
                state->countdown = 0xF0;
            }
        }

        target = (Overlay73Target *)state->target;
        if (target != NULL) {
            data = target->data;
            dx = object->x - data->x;
            dz = object->z - data->z;
            targetAngle = Arctanf(dx, dz);
            delta = targetAngle - object->angle;
            if (delta < 0) {
                if (-absStep < delta) {
                    object->angle += delta;
                } else {
                    object->angle -= absStep;
                }
            } else if (delta > 0) {
                if (delta < absStep) {
                    object->angle += delta;
                } else {
                    object->angle += absStep;
                }
            }

            height = ((Overlay73Target *)state->target)->data->y - object->y;
            if (height < 0) {
                object->velocityY -= 0.1f * (f32)updateRate;
                if (object->velocityY < height) {
                    object->velocityY = height;
                }
            } else if (height > 0) {
                object->velocityY += 0.1f * (f32)updateRate;
                if (height < object->velocityY) {
                    object->velocityY = height;
                }
            }

            if (((dx * dx) + (dz * dz)) < 256.0f) {
                state->mode = 4;
            }
        } else {
            state->mode = 1;
        }

        object->timer += 0.064f * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 4: {

        if (state->angle >= 0x181) {
            state->angle -= updateRate * 0x10;
            if (state->angle < 0x180) {
                state->angle = 0x180;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        limit = state->radius * state->radius * 4.0f;
        if (limit < ((dx * dx) + (dz * dz))) {
            state->mode = 2;
        } else {

            if (updateRate < state->countdown) {
                state->countdown -= updateRate;
            } else {
                state->countdown = 0;
                if ((hitCount = func_8005776C(object->x, 0.0f, object->z, 150.0f, 1, hits)) != 0) {
                    if (hitCount >= 2) {
                        hitCount = mathRnd(1, hitCount);
                    }
                    hitCount--;
                    state->target = hits[hitCount];
                    state->countdown = 0xF0;
                } else {
                    state->target = NULL;
                    state->mode = 1;
                }
            }

            if (state->mode == 4) {
                data = ((Overlay73Target *)state->target)->data;
                dx = object->x - data->x;
                dz = object->z - data->z;
                if (22500.0f < ((dx * dx) + (dz * dz))) {
                    state->mode = 3;
                } else {
                    if (func_800299E8(0, 0x3FF) >= 0x3EC) {
                        state->angle = -state->angle;
                    }
                    object->angle += updateRate * state->angle;
                    dy = ((Overlay73Target *)state->target)->data->y;
                    if ((dy + 40.0f) < object->y) {
                        state->flags &= ~1;
                    } else if (object->y < dy) {
                        state->flags |= 1;
                    }
                    if (state->flags & 1) {
                        object->velocityY += 0.1f * (f32)updateRate;
                    } else {
                        object->velocityY -= 0.1f * (f32)updateRate;
                    }
                    object->angle += state->angle;
                }
            }

        }

        object->timer += 0.064f * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    default:
        goto common;
    }

common:
    if (state->mode != 0) {
        if (state->timer < 3.0f) {
            state->timer += 0.25f * (f32)updateRate;
            if (state->timer > 3.0f) {
                state->timer = 3.0f;
            }
        }
    }

    if (state->timer != 0.0f) {

        if (state->mode != 4) {
            limit = 1.2f;
        } else {
            limit = 1.6f;
        }
        if (object->velocityY < -limit) {
            object->velocityY = -limit;
        }
        if (limit < object->velocityY) {
            object->velocityY = limit;
        }
        object->velocityX = func_8002A8C0(object->angle) * -state->timer;
        object->velocityZ = func_8002A8BC(object->angle) * -state->timer;
        func_80008128(object, object->velocityX, object->velocityY,
                      object->velocityZ);
    }

    state->vertexBank = 1 - state->vertexBank;
    vertex = &state->vertices[state->vertexBank * 6];
    phase = (s32)(object->timer * 32768.0f);
    horizontal = (s32)(func_8002A8C0(phase) * 64.0f);
    vertical = (s32)(func_8002A8BC(phase) * 64.0f);
    if (horizontal < 0) {
        horizontal = -horizontal;
    }
    if (vertical < 0) {
        vertical = -vertical;
    }
    vertex[0].x = -horizontal;
    vertex[0].y = vertical;
    vertex[1].x = -horizontal;
    vertex[1].y = vertical;
    vertex[4].x = horizontal;
    vertex[4].y = vertical;
    vertex[5].x = horizontal;
    vertex[5].y = vertical;
    if (object->output != NULL) {
        *object->output = (f32)horizontal * 0.1f * object->scale;
    }
}

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDPSetPrimColor and gDma1p, include/PR/mbi.h
 * _SHIFTL, include/f3ddkr.h gSPVertexJFG and gSPPolygon,
 * include/PR/os_convert.h OS_PHYSICAL_TO_K0), a permitted source under
 * docs/CLEANROOM.md, by way of the matched overlay 71 renderer
 * func_overlay_071_F0000870_18CA390, whose shape this function copies.  No
 * function body was imported.
 *
 * Matched 2026-10-02 as a sibling copy of that overlay 71 renderer: every
 * command is its packet macro, and the vertex bank is a subscript into an
 * array of ten-byte vertices, `vertices[vertexBank * 6]`, so the address is
 * the index times six times the element size -- the shipped 3*2 then 5*2
 * multiply chain, which a single `* 60` byte offset folds into 15*4.  The
 * unused `pad` between `vertices` and `state` lands the 0x30 frame and the
 * vertices spill at +0x2C.
 */
#define O73_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O73_RGBA(r, g, b, a) (O73_SHIFTL(r, 24, 8) | O73_SHIFTL(g, 16, 8) | O73_SHIFTL(b, 8, 8) | O73_SHIFTL(a, 0, 8))
#define O73_SET_PRIM_COLOR(pkt, m, l, r, g, b, a) { Overlay73Command *_g = (Overlay73Command *)(pkt); _g->w0 = (O73_SHIFTL(0xFA, 24, 8) | O73_SHIFTL(m, 8, 8) | O73_SHIFTL(l, 0, 8)); _g->w1 = O73_RGBA(r, g, b, a); }
#define O73_DMA1P(pkt, c, s, l, p) { Overlay73Command *_g = (Overlay73Command *)(pkt); _g->w0 = (O73_SHIFTL((c), 24, 8) | O73_SHIFTL((p), 16, 8) | O73_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O73_VERTEX(pkt, v, n, v0) O73_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O73_POLYGON(dl, ptr, numTris, texEnabled) { Overlay73Command *_g = (Overlay73Command *)(dl); _g->w0 = O73_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O73_SHIFTL(0x05, 24, 8) | O73_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O73_PHYSICAL_TO_K0(x) (void *)(((u32)(x) + 0x80000000))

void func_overlay_073_F0000D70_18CB830(Overlay73Command **commands,
                                       void *context,
                                       Overlay73Object *object) {
    Overlay73Vertex *vertices;
    s32 pad;
    Overlay73State *state;

    state = object->state;
    if (state->resource != NULL) {
        vertices = &state->vertices[state->vertexBank * 6];
        camPushModelMtx(commands, context, object, 1.0f, 0.0f);
        O73_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, object->alpha);
        texDPTextureX(commands, state->resource, 0xE, 0);
        O73_VERTEX((*commands)++, O73_PHYSICAL_TO_K0(vertices), 6, 0);
        O73_POLYGON((*commands)++, D_80000000, 8, 1);
        O73_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, object->alpha);
        camPopModelMtx(commands);
    }
}
