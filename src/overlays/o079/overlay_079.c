#include "overlays/overlay_079.h"

static Overlay79Object *overlay79FindNearby(Overlay79Vector *point,
                                            f32 radiusSquared);

/*
 * Overlay 79's initialized image (ROM 0x18CE480..0x18CE4E0, module offsets
 * +0x14E0..+0x1540).  The shipped relocation table splits it at +0x1500: the
 * first eight words are one runtime-indexed table (LOCAL records patch a base
 * of +0x14E0 with a zero addend and a scaled `object->mode`), and the sixteen
 * words after it are the constant pool the module's code loads with LOCAL
 * records whose base is +0x1500 and whose addends run +0x4..+0x2C.  Tier A for
 * the split and for the addends; the byte values are unchanged from the
 * previously adopted image, so the module's data bytes are identical.
 *
 * Words 1 to 11 of the pool are this function's float literals in order of
 * first use; the compiled object's own pool is bound back onto them by the
 * object's rule in `mk/overlays.mk`.
 */
f32 gOverlay79ModeFactors[8] = {
    0.04f, 0.01f, 0.02f, 0.1f, 0.01f, 0.0f, 0.0f, 0.0f,
};

f32 gOverlay79Constants[16] = {
    0.05f,  -0.2f, 0.2f, 0.9f, -0.1f, 0.707f, 0.1f, 0.9f,
    0.46f,  0.9f,  -0.1f, -0.2f, 0.707f, 0.1f, 0.01f, 0.01f,
};

#define O79_ACTIVE_ACCELERATION   -0.2f
#define O79_INACTIVE_ACCELERATION 0.2f
#define O79_TURN_POWER            0.9f
#define O79_FORWARD_ACCELERATION  -0.1f
#define O79_TARGET_DOT            0.707f
#define O79_BRAKE_ACCELERATION    0.1f
#define O79_APPROACH_POWER        0.9f
#define O79_LAUNCH_HEIGHT         0.46f
#define O79_TURN_IN_POWER         0.9f
#define O79_GRAVITY_HALF          -0.1f
#define O79_GRAVITY               -0.2f

void func_overlay_079_F0000000_18CCFA0(Overlay79Object *object,
                                        Overlay79Config *config,
                                        s32 preserveState) {
    Overlay79MotionState *state;
    f32 range;
    s32 random;

    (void)preserveState;
    state = object->state;
    object->scale = config->scale * gOverlay79InitScaleReloc
                     * object->scaleSource->value;
    object->angle = config->angle;
    state->heightOffset = object->scale * 60.0f;
    state->travelRadius = config->range;
    state->radiusSquared = state->travelRadius * state->travelRadius;
    state->homeX = object->x;
    state->homeY = object->y;
    state->homeZ = object->z;
    state->effectTimer = 0xF0;
    random = mathRnd(-0x7FFF, 0x8000);
    range = mathRnd(0, (s32)state->travelRadius);
    state->targetX = state->homeX - (func_8002A8C0(random) * range);
    state->targetZ = state->homeZ - (func_8002A8BC(random) * range);
    func_8005AD64(object, 0, -1, 0.0f);
}

/*
 * Matched 2026-10-01 by rewriting the inherited shape rather than its
 * allocation.  The edits, in the order they were found:
 *
 *   - the eleven float constants are literals at each use, not reads of a
 *     data array: uopt hoists them as constant webs and the integration
 *     statements keep their loads across the stores (distance, speed, dy,
 *     velocityY in that order);
 *   - `forward`, `start` and `end` are `f32[3]` arrays, which is what
 *     `mathOneFloatRPY` takes elsewhere in the tree.  An array element in the
 *     defining expression stops uopt forwarding `dot` and the final `dy` into
 *     their single uses, so both are coloured;
 *   - `dx * forward[0] + dz * forward[2]` for both dot products, with `dx`
 *     written as one subtraction;
 *   - the race-state test reads bitfields and its 0/1 result is a cast
 *     conditional expression, which leaves the value in a compiler temporary
 *     rather than in the local's own home;
 *   - `state->mode` is read at each test with no local copy (one unused word
 *     keeps the frame);
 *   - `func_80008128` returns `s32`, as its other callers declare it, which
 *     keeps the following block's `object->mode` out of v0.
 */
void func_overlay_079_F0000134_18CD0D4(Overlay79Object *object,
                                       s32 updateRate) {
    Overlay79MotionState *state;
    Overlay79Object *nearby;
    Overlay79Object *spawned;
    Overlay79SpawnDesc desc;
    f32 start[3];
    f32 end[3];
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 dot;
    f32 update;
    f32 range;
    f32 factor;
    f32 forward[3];
    s32 pad;
    s32 delta;
    s32 angle;
    s32 raceActive;

    state = object->state;
    update = updateRate;
    raceActive = (u32)((((D_800D3128.progress & 0x1C0) >> 6) >= 3 &&
                        D_800D3128.stage == 0xF &&
                        !D_800D3128.disabled)
                           ? 1
                           : 0);

    nearby = overlay79FindNearby((Overlay79Vector *)&object->x, 22500.0f);
    if (nearby != NULL) {
        state->active = 1;
    }

    if (state->active != 0) {
        if (state->collisionFlags & 2) {
            if (object->mode != 0) {
                func_8005AD64(object, 0, -1, 0.0f);
            }
        } else if (((state->collisionFlags & 2) == 0) &&
                   (object->mode != 3)) {
            func_8005AD64(object, 3, -1, 0.0f);
        }

        if (nearby != NULL) {
            if (state->speed > -25.0f) {
                state->acceleration = O79_ACTIVE_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
            }
            if (nearby != NULL) {
                dx = nearby->x - object->x;
                dz = nearby->z - object->z;
                if ((sqrtf((dx * dx) + (dz * dz)) < 70.0f) &&
                    (state->collisionFlags & 2)) {
                    object->velocityY = 5.0f;
                    if (state->effect != NULL) {
                        amSndStopXYZ(state->effect);
                    }
                    amSndPlayXYZ(mathRnd(0x21F, 0x226), object->x, object->y,
                                  object->z, 4, &state->effect);
                    state->effectTimer = mathRnd(0x78, 0xF0);
                }
            }
        } else if (state->speed < 0.0f) {
            state->acceleration = O79_INACTIVE_ACCELERATION;
        } else {
            state->active = 0;
            state->step = 0;
            state->mode = 2;
            state->acceleration = 0.0f;
            state->speed = 0.0f;
        }

        dx = object->x - state->homeX;
        dz = object->z - state->homeZ;
        if ((state->radiusSquared <= ((dx * dx) + (dz * dz))) ||
            (nearby == NULL)) {
            state->targetAngle = Arctanf(dx, dz);
        } else {
            state->targetAngle = Arctanf(nearby->x - object->x,
                                         nearby->z - object->z);
        }
        object->angle = dAngle(object->angle, state->targetAngle,
                               1.0f - Powerf(O79_TURN_POWER, updateRate));
        delta = (object->angle - state->targetAngle) & 0xFFFF;
        if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
            delta = -((state->targetAngle - object->angle) & 0xFFFF);
        }
        if ((delta >= -99) && (delta < 100)) {
            object->angle = state->targetAngle;
        }
        if (state->target != NULL) {
            dx = state->target->x - object->x;
            dy = state->target->y - object->y;
            dz = state->target->z - object->z;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 30.0f) {
                spawned = (Overlay79Object *)state->target->state;
                *(s32 *)spawned = 1;
            }
        }
        if (state->effectTimer != 0) {
            state->effectTimer -= updateRate;
            if (state->effectTimer < 0) {
                state->effectTimer = 0;
            }
        }
        if ((state->effect == NULL) && (state->effectTimer == 0)) {
            amSndPlayXYZ(mathRnd(0x227, 0x229), object->x, object->y,
                          object->z, 4, &state->effect);
            state->effectTimer = mathRnd(0x78, 0xF0);
        }
    } else {
        if (state->mode == 0) {
            if (state->speed > -1.0f) {
                state->acceleration = O79_FORWARD_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
            }
            forward[0] = 0.0f;
            forward[1] = 0.0f;
            forward[2] = -1.0f;
            mathOneFloatRPY(object, forward);
            dx = state->targetX - object->x;
            dz = state->targetZ - object->z;
            dot = (dx * forward[0]) + (dz * forward[2]);
            if ((dot < 0.0f) || (state->collisionFlags & 4)) {
                state->mode = 1;
            } else if (state->target != NULL) {
                dx = object->x - state->target->x;
                dz = object->z - state->target->z;
                distance = sqrtf((dx * dx) + (dz * dz));
                forward[0] = 0.0f;
                forward[1] = 0.0f;
                forward[2] = -1.0f;
                mathOneFloatRPY(object, forward);
                dx = state->target->x - object->x;
                dz = state->target->z - object->z;
                dot = (dx * forward[0]) + (dz * forward[2]);
                if ((distance < 50.0f) && (O79_TARGET_DOT < dot)) {
                    state->mode = 1;
                }
            }
            if (state->effectTimer != 0) {
                state->effectTimer -= updateRate;
                if (state->effectTimer < 0) {
                    state->effectTimer = 0;
                }
            }
            if ((state->effect == NULL) && (state->effectTimer == 0)) {
                amSndPlayXYZ(mathRnd(0x21B, 0x21E), object->x, object->y,
                              object->z, 4, &state->effect);
                state->effectTimer = mathRnd(0x78, 0xF0);
            }
        } else if (state->mode == 1) {
            if (state->speed < 0.0f) {
                state->acceleration = O79_BRAKE_ACCELERATION;
            } else {
                state->acceleration = 0.0f;
                state->speed = 0.0f;
                if ((state->target == NULL) && (raceActive != 0)) {
                    state->mode = 3;
                } else {
                    state->mode = 2;
                }
            }
        } else if (state->mode == 2) {
            if (state->step == 0) {
                func_8005AD64(object, 1, -1, 0.0f);
                state->step++;
            } else if ((state->step == 1) && (state->event != 0)) {
                if (state->effect == NULL) {
                    amSndPlayXYZ(mathRnd(0x218, 0x21A), object->x, object->y,
                                  object->z, 4, &state->effect);
                }
                func_8005AD64(object, 2, -1, 0.0f);
                state->step++;
            } else if ((state->step == 2) && (state->event != 0)) {
                func_8005AD64(object, 1, -1, 0.0f);
                state->step++;
            } else if ((state->step == 3) && (state->event != 0)) {
                func_8005AD64(object, 0, -1, 0.0f);
                state->step = 0;
                angle = mathRnd(-0x7FFF, 0x8000);
                range = mathRnd(0, (s32)state->travelRadius);
                state->targetX = state->homeX - (func_8002A8C0(angle) * range);
                state->targetZ = state->homeZ - (func_8002A8BC(angle) * range);
                dx = object->x - state->targetX;
                dz = object->z - state->targetZ;
                state->targetAngle = Arctanf(dx, dz);
                state->mode = 4;
            }
        } else if (state->mode == 3) {
            if (state->step == 0) {
                angle = mathRnd(-0x7FFF, 0x8000);
                range = mathRnd(0, (s32)state->travelRadius);
                state->targetX = state->homeX - (func_8002A8C0(angle) * range);
                state->targetZ = state->homeZ - (func_8002A8BC(angle) * range);
                dx = object->x - state->targetX;
                dz = object->z - state->targetZ;
                state->targetAngle = Arctanf(dx, dz);
                state->step = 1;
            } else if (state->step == 1) {
                object->angle = dAngle(object->angle, state->targetAngle,
                                       1.0f - Powerf(O79_APPROACH_POWER,
                                                     updateRate));
                delta = (object->angle - state->targetAngle) & 0xFFFF;
                if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
                    delta = -((state->targetAngle - object->angle) & 0xFFFF);
                }
                if ((delta >= -99) && (delta < 100)) {
                    object->angle = state->targetAngle;
                    state->step = 2;
                }
            } else if (state->step == 2) {
                func_8005AD64(object, 4, -1, 0.0f);
                if (state->effect == NULL) {
                    amSndPlayXYZ(mathRnd(0x270, 0x272), object->x, object->y,
                                  object->z, 4, &state->effect);
                }
                state->step = 3;
            } else if ((state->step == 3) &&
                       (state->previousHeight < O79_LAUNCH_HEIGHT) &&
                       (O79_LAUNCH_HEIGHT <= object->floorHeight)) {
                desc.objectId = 0x144;
                desc.kind = 0x14;
                desc.flags = 0;
                desc.x = (func_8002A8C0(object->angle) * 18.0f) + object->x;
                desc.y = object->y;
                desc.z = (func_8002A8BC(object->angle) * 18.0f) + object->z;
                desc.parent = object;
                desc.scale = object->scale;
                spawned = func_8000590C(&desc, 1);
                if (spawned != NULL) {
                    spawned->field3C = 0;
                    state->target = spawned;
                }
                amSndPlayXYZ(0x273, object->x, object->y, object->z, 4, NULL);
                state->step = 4;
            } else if ((state->step == 4) && (state->event != 0)) {
                func_8005AD64(object, 0, -1, 0.0f);
                state->step = 0;
                state->mode = 0;
            }
        } else if (state->mode == 4) {
            object->angle = dAngle(object->angle, state->targetAngle,
                                   1.0f - Powerf(O79_TURN_IN_POWER,
                                                 updateRate));
            delta = (object->angle - state->targetAngle) & 0xFFFF;
            if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
                delta = -((state->targetAngle - object->angle) & 0xFFFF);
            }
            if ((delta >= -99) && (delta < 100)) {
                object->angle = state->targetAngle;
                state->mode = 0;
            }
        }
    }

    distance = (state->speed * update) +
               (0.5f * state->acceleration * update * update);
    state->speed += state->acceleration * update;
    dy = (object->velocityY * update) +
         (O79_GRAVITY_HALF * update * update);
    object->velocityY += O79_GRAVITY * update;
    dx = func_8002A8C0(object->angle) * distance;
    dz = func_8002A8BC(object->angle) * distance;

    start[0] = object->x;
    start[1] = object->y + state->heightOffset;
    start[2] = object->z;
    end[0] = start[0] + dx;
    end[1] = start[1] + dy;
    end[2] = start[2] + dz;
    state->collisionFlags = 0;
    trackMakePolylist(1, start, end, &state->heightOffset, NULL, 1);
    if (func_80010900(start, end, state->heightOffset, object, func_overlay_079_F0000FA0_o079Reloc) != 0) {
        object->velocityY = 0.0f;
    }
    dx = end[0] - start[0];
    dy = end[1] - start[1];
    dz = end[2] - start[2];
    func_80008128(object, dx, dy, dz);
    object->velocityX = dx / update;
    object->velocityZ = dz / update;
    if (object->mode == 0) {
        factor = gOverlay79ModeFactors[object->mode] * -state->speed;
    } else {
        factor = gOverlay79ModeFactors[object->mode];
    }
    state->previousHeight = object->floorHeight;
    state->event = func_8005ABA8(object, factor, update);
    if (state->effect != NULL) {
        amSndSetXYZ(state->effect, object->x, object->y, object->z);
    }
    if (object->mode == 3) {
        object->flags |= 1;
        partUpdateTriggers(object, updateRate);
    }
}

/* DKR v77/v80 and JFG exact-object scans are negative for this radius query. */
static Overlay79Object *overlay79FindNearby(Overlay79Vector *point, f32 radiusSquared) {
    Overlay79Object **entries;
    Overlay79Object *entry;
    Overlay79Object *result;
    f32 dx;
    f32 dz;
    /* Unused volatile slot kept: it sizes the frame; removing it moves 11 words. */
    volatile s32 scratch;
    s32 count;

    entries = func_80005750(&count);
    result = NULL;
    while (count != 0) {
        entry = *entries++;
        dx = entry->x - point->x;
        dz = entry->z - point->z;
        if ((dx * dx) + (dz * dz) < radiusSquared) {
            result = entry;
        }
        count--;
    }
    return result;
}

/*
 * PROVENANCE: copied from the shape of the matched resident sibling
 * func_800115E4 (src/main/track.c), the same collision response with one
 * branch fewer; no external function body is adapted. The plane offset is
 * computed inside the slide branch, the three literals are written at their
 * uses (the module's constant pool, two distinct 0.01f words), and the
 * declaration order places d, delta, u, v and w on the target's stack homes.
 */
void func_overlay_079_F0000FA0_18CDF40(
    void *unused, Overlay79Vector *pos, Overlay79Vector *vel,
    f32 radius, Overlay79Plane *plane, Overlay79Object *object) {
    f32 nx;
    f32 ny;
    f32 nz;
    f32 dx;
    f32 d;
    f32 dy;
    f32 dz;
    f32 len;
    f32 delta;
    f32 u;
    f32 v;
    f32 w;
    f32 value;
    f32 angle;
    Overlay79MotionState *state;

    nx = plane->normal.x;
    ny = plane->normal.y;
    nz = plane->normal.z;
    state = object->state;
    d = plane->constant;
    if ((0.707f <= ny) || (plane->flags & 0x10000000)) {
        u = vel->z * ny;
        v = -(vel->z * nx) + (nz * vel->x);
        w = -(vel->x * ny);
        dx = (v * nz) - (w * ny);
        dy = (w * nx) - (u * nz);
        dz = (u * ny) - (v * nx);
        len = (dx * dx) + (dy * dy) + (dz * dz);
        if (0.1f < len) {
            len = sqrtf(len);
            dx /= len;
            dy /= len;
            dz /= len;
            len = radius - plane->distance;
            pos->x = plane->origin.x + (len * dx);
            pos->y = plane->origin.y + (len * dy);
            pos->z = plane->origin.z + (len * dz);
        } else {
            pos->y = (-((pos->z * nz) + (nx * pos->x) + d) / ny) + 0.01f;
        }
        state->collisionFlags |= 2;
    } else {
        value = (pos->z * nz) + ((nx * pos->x) + (ny * pos->y)) + d;
        len = 0.01f - value;
        delta = len;
        u = pos->x + (len * nx);
        v = pos->y + (len * ny);
        w = pos->z + (len * nz);
        dx = pos->x - u;
        dy = pos->y - v;
        dz = pos->z - w;
        len = sqrtf((dx * dx) + (dz * dz));
        angle = func_8002A8BC((s16) Arctanf(dy, len));
        if (angle != 0.0f) {
            value = delta / angle;
            len = sqrtf((nx * nx) + (nz * nz));
            pos->x += value * (nx / len);
            pos->z += value * (nz / len);
        } else {
            pos->x = u;
            pos->y = v;
            pos->z = w;
        }
        state->collisionFlags |= 4;
    }
}

void overlay79SetLink(Overlay79TimerObject *object, Overlay79LinkConfig *entry) {
    s32 *state = object->timers;
    state[1] = entry->link;
}

/* Bounded reproof (2026-08-29): configured -O2 -mips2 -32 C remains exactly
 * 123 instructions / 492 bytes with frame 0x48, 111/123 raw and runtime-
 * normalized words, 12 register-field differences, and first mismatch +0xC8.
 * Its executable-text SHA-256 is d8b80e23d14ea9c2. All 119 flag identities
 * were attempted; seven O2/MIPS-II rows tie V0 and none is exact. A fidelity-
 * clean proc-0 globalcolor trace identifies separate v1/v0 linked-state webs.
 * A distinct linked-state local and branch-local linked-state scopes both
 * regress to frame 0x50 and 30 differences; a distinct next-node carrier
 * regresses to frame 0x50 and 43 differences. No strict gain authorized a
 * combination or generic batch. Earlier source-faithful lifetime/access forms
 * likewise remained at 12 or regressed.
 *
 * A follow-on trace-led pass spent 20 stock configured builds (two V0 proofs
 * and 18 new representation, ABI, expression, scoped-constant, and declaration-
 * order forms) plus one fidelity-clean instrumented ugen capture. The trace
 * attributes the tail to a one-slot temp-FIFO phase: the candidate allocates
 * t1/t2 for the flags update and t3 for the alternate emit, while the target
 * requires t2/t3 and t4. The linked-state loads remain an independent v1/v0
 * global-color outcome. Every new form was byte-flat at 12 differences or
 * regressed to 14-42; the scoped emit-id local was exact-sized but regressed
 * to 33. No JFG function supplies a body, ABI, or allocator mechanism here.
 *
 * A resumed bounded pass (2026-08-30) proved the missing temp-FIFO mechanism.
 * Preserving the u8 width explicitly as `(value & 0xFF) | 4` consumes one
 * backend temp while the redundant mask emits no instruction. The retained C
 * remains exactly 123 words / 492 bytes with frame 0x48 and first mismatch
 * +0xC8, improves to 119/123 raw and runtime-normalized words, and reproduces
 * the complete integer temp lane. An opaque `void *` state slot and a zero-
 * offset typed state-slot wrapper were independently byte-flat on top of that
 * gain. The sole executable residual is now the four-site v1/v0 linked-state
 * pool web; no source-faithful coalescing form was found within the narrow
 * follow-up cap.
 *
 * The owned +0x1290..+0x147C range has no target padding. IDO's four trailing
 * alignment bytes are outside the function and production trims only them.
 * The C object emits all 15 shipped runtime tuples. Fresh ordinary preflight
 * proves every offset/type but resolves only one candidate identity and fails
 * closed on the other 14. A prior identity-canonical diagnostic comparison
 * proves all 15 identities/addends; it is evidence, not promotion machinery.
 * The assembled target has 13 static records because the runtime table adds the
 * already-zero flags pair. The committed filter/rebind metadata remains
 * diagnostic and is not production POSTPROCESS, so linked C proof is absent.
 * ORT 1297 and resident relocation 139 establish func_8000AEEC+0x43C as the
 * sole inbound; no cross-overlay inbound exists. Pinned DKR v77/v80 and JFG
 * scans are negative. Preserve the fallback and resume only with a newly
 * proved natural v0/v1 linked-state coalescing form; do not repeat the flag
 * lattice, temp-FIFO lever, state-slot type forms, or a generic batch.
 *
 * Naturalness pass (lane c-1, 2026-10-08): the empty `if (object->position.y)`
 * and the `if (1)` that opened the else arm were byte-inert and are gone. The
 * empty `if (1)` between the `field3C` store and the emit call is load-bearing:
 * without it the linked-state load after the call takes v1 instead of v0 at
 * both sites (4 words), and storing through `node->next->state` directly
 * instead of reusing `spawned` costs 26 words. */
void func_overlay_079_F0001290_18CE230(Overlay79SpawnerObject *object, s32 unused1)
{
  Overlay79Node *node;
  Overlay79SpawnDesc desc;
  Overlay79Spawned *spawned;
  node = object->node;
  if (node->active != 0)
  {
    desc.objectId = 0x14B;
    desc.kind = 0xC;
    desc.flags = 0;
    desc.x = object->position.x;
    desc.y = object->position.y;
    desc.z = object->position.z;
    desc.angle = mathRnd(-0x7FFF, 0x8000);
    spawned = func_8000590C(&desc, 1);
    if (spawned != 0)
    {
      spawned->field3C = 0;
      if (1)
      {
      }
      amSndPlayXYZ(0x277, object->position.x, object->position.y, object->position.z, 4, 0);
    }
    spawned = node->next->state;
    spawned->field40 = 0;
    func_80006EA0(object);
  }
  else
  {
    if (overlay79FindNearby(&object->position, 900.0f) != 0)
    {
      desc.objectId = 0x14B;
      desc.kind = 0xC;
      desc.flags = 0;
      desc.x = object->position.x;
      desc.y = object->position.y;
      desc.z = object->position.z;
      desc.angle = mathRnd(-0x7FFF, 0x8000);
      spawned = func_8000590C(&desc, 1);
      if (spawned != 0)
      {
        spawned->field3C = 0;
        if ((++gOverlay79CounterReloc) == 0x14)
        {
          amSndPlay(0x27C, 0);
          ((u8 *)&D_800D3128)[1] = (((u8 *)&D_800D3128)[1] & 0xFF) | 4;
          func_800291B4();
        }
        else
        {
          amSndPlayXYZ(0x277, object->position.x, object->position.y, object->position.z, 4, 0);
        }
      }
      spawned = node->next->state;
      spawned->field40 = 0;
      func_80006EA0(object);
    }
  }
}

void overlay79InitState(Overlay79TimerObject *object, Overlay79Config *entry) {
    s32 *state = object->timers;
    state[1] = 0xFF;
    state[0] = 0x78;
    object->angle = entry->angle;
}

/* DKR v77/v80 and JFG checks found only broad timer/update semantics. */
void overlay79UpdateTimers(Overlay79TimerObject *object, s32 step) {
    s32 *timers = object->timers;

    timers[0] -= step;
    if (timers[0] <= 0) {
        timers[1] -= step;
        if (timers[1] <= 0) {
            timers[1] = 0;
            func_80006EA0(object);
        }
    }
}
