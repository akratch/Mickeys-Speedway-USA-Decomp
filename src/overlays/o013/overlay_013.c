#include "overlays/overlay_013.h"

/*
 * Overlay 13: a pool of falling, fading sprites. One translation unit;
 * overlay13Initialize and overlay13DrawRecord are unexported and static.
 */

/* Pinned DKR v77/v80 have no O13 donor; JFG's ambiguous hit is at +0xB0C. */
static void overlay13Initialize(void) {
    u8 *record;
    u8 *entry;
    Overlay13Resource *resource;
    s32 recordIndex;
    s32 entryIndex;
    s32 width;
    s32 height;

    gOverlay13Resource0 = texLoadSprite(0x2C, 0);
    gOverlay13Resource1 = texLoadSprite(0x2D, 0);
    gOverlay13Resource2 = texLoadTexture(0x6A);

    record = (u8 *)gOverlay13Records;
    recordIndex = 0;
    do {
        record[6] = 0;
        entryIndex = 0;
        entry = record;
        do {
            entryIndex++;
            entry += 0x28;
            entry[0x18] = 0xFF;
            entry[0x19] = 0xFF;
            entry[0x1A] = 0xFF;
            entry[0x1B] = 0xFF;
            entry[0x22] = 0xFF;
            entry[0x23] = 0xFF;
            entry[0x24] = 0xFF;
            entry[0x25] = 0xFF;
            entry[0x2C] = 0xFF;
            entry[0x2D] = 0xFF;
            entry[0x2E] = 0xFF;
            entry[0x2F] = 0xFF;
            entry[0x0E] = 0xFF;
            entry[0x0F] = 0xFF;
            entry[0x10] = 0xFF;
            entry[0x11] = 0xFF;
        } while (entryIndex != 2);
        recordIndex++;
        record += 0x80;
    } while (recordIndex != 0x20);

    gOverlay13Enabled = 0;
    resource = gOverlay13Resource2;
    if (resource != 0) {
        width = (resource->width - 1) << 5;
        height = (resource->height - 1) << 5;
        *(s16 *)(gOverlay13Output + 0x08) = width;
        *(s16 *)(gOverlay13Output + 0x0C) = width;
        *(s16 *)(gOverlay13Output + 0x0E) = height;
        *(s16 *)(gOverlay13Output + 0x18) = width;
        *(s16 *)(gOverlay13Output + 0x1A) = height;
        *(s16 *)(gOverlay13Output + 0x1E) = height;
    }
    gOverlay13Active = 1;
}

/* DKR v77/v80 and JFG contain only generic null-guarded release patterns. */
void overlay13Release(void) {
    void *resource;

    resource = gOverlay13Resource0;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    resource = gOverlay13Resource1;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    resource = gOverlay13Resource2;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    gOverlay13Active = 0;
}

/* Pinned donors contain no hit in this ownership unit. */
Overlay13Record *overlay13CreateRecord(
    f32 value08, f32 value0C, f32 value10, f32 value14,
    f32 value18, f32 value1C, f32 value20, f32 value24) {
    Overlay13Record *record;
    s32 i;

    if (gOverlay13Active == 0) {
        overlay13Initialize();
    }

    record = gOverlay13Records;
    i = 0;
    if (gOverlay13Enabled >= 0x20) {
        return 0;
    }

    for (i = 0; i < 0x20; i++, record++) {
        if (record->state == 0) {
            break;
        }
    }

    if (i < 0x20) {
        record->field04 = 0;
        record->state = 1;
        record->timer = 0x80;
        record->scale = value08;
        record->x = value0C;
        record->z = value10;
        record->y = value14;
        record->targetZ = value18;
        record->velocityX = value1C;
        record->velocityZ = value20;
        record->velocityY = value24;
        record->phase = 0.0f;
        record->vertexIndex = 1;
        gOverlay13Enabled++;
    }

    return record;
}

/* Reconstructed from the listing 2026-09-16 (lane s1-a): the earlier candidate
 * tested the tick count after decrementing and returned a pointer; the ROM
 * tests the old count (`if (ticks--)`, `while (ticks--)` -- every
 * `or v0,s0,zero` is that test's dead copy, not a result snapshot), reloads
 * `state` on the count exit only, promotes `record->timer` in the fade loop,
 * flips `vertexIndex` through the field (a ring temp, no `index` local), and
 * fills the four vertices through a walking pointer that uopt folds to its
 * final value (the +48 in one block, the +30 in the next). The caller
 * discards the result, so the function is void.
 * s2-b (2026-09-17), matched from 70 at delta 0: no `state` local (its
 * symbol web was a phantom holding v0 in the break block; every test reads
 * `record->state`, whose reload on each loop exit is uopt's PRE of that
 * load); `s32 timer` so the once-masked value is tested without a second
 * `andi`, coupled with the fade loop leaving through a `goto` like the fall
 * loop; `s16 y`, whose dropped narrowing spends the two folded draws that
 * put the vertex temps on the ROM's ring positions; the vertex stores in
 * x, y, z order; and two dead definitions before the fall loop's guard
 * (`velocityX = record->z; velocityY = record->velocityY;`), which uopt
 * deletes after they have numbered the velocities' webs ahead of gravity's
 * and ordered the hoisted loads z-then-velocityY. See
 * docs/lastmile-forwarding-kill.md. */
void overlay13UpdateRecord(Overlay13Record *record, s32 ticks) {
    f32 radius;
    f32 gravity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    s16 y;
    s32 timer;
    s16 *result;

    SetLinkSlot(13, 50, 10);
    if (record->state == 1) {
        /* Dead, and deleted by uopt: these two definitions exist to number the
         * velocities' webs before gravity's and to order the hoisted loads. */
        velocityX = record->z;
        velocityY = record->velocityY;
        if (ticks--) {
            gravity = gOverlay13Gravity;
            velocityX = record->velocityX;
            do {
                velocityZ = record->velocityZ;
                record->x += velocityX;
                record->z += velocityZ;
                record->y += record->velocityY;
                record->velocityZ = velocityZ - gravity;
                if (record->z < record->targetZ) {
                    record->z = record->targetZ;
                    record->state = 2;
                    goto fall_done;
                }
            } while (ticks--);
        }
fall_done:;
    }

    if (record->state == 2) {
        if (ticks--) {
            timer = record->timer;
            do {
                record->timer -= 2;
                timer = record->timer;
                record->phase += 0.5f;
                if (timer == 0) {
                    record->state = 0;
                    gOverlay13ActiveCount--;
                    goto fade_done;
                }
            } while (ticks--);
        }
fade_done:
        if (record->state != 0) {
            record->vertexIndex = 1 - record->vertexIndex;
            result = (s16 *)&record->vertices[record->vertexIndex * 0x28];
            radius = record->scale * (24.0f - 0.1875f * (f32)record->timer);
            y = (s32)record->targetZ;

            result[0] = (s32)(record->x - radius);
            result[1] = y;
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[0] = (s32)(record->x + radius);
            result[1] = y;
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[0] = (s32)(record->x - radius);
            result[1] = y;
            result[2] = (s32)(record->y - radius);
            result += 5;
            result[0] = (s32)(record->x + radius);
            result[1] = y;
            result[2] = (s32)(record->y - radius);
        }
    }
}

/* DKR v77/v80 contain only generic fixed-pool loops, not this record walk. */
void overlay13ProcessActive(s32 arg) {
    s32 i;
    Overlay13Record *record;

    if (gOverlay13Enabled != 0) {
        record = gOverlay13Records;
        for (i = 0; i < 32; i++, record++) {
            if (record->state != 0) {
                overlay13UpdateRecord_o013Reloc(record, arg);
            }
        }
    }
}

/* Each command is one gbi-style block, `{ Gfx *_g = pkt; _g->w0 = ..; _g->w1 = ..; }`
 * (PROVENANCE: the shape of DKR's `fast3d_cmd`/`gDma1p` macros in
 * include/f3ddkr.h and gbi.h; the words are this ROM's own). The block-scoped
 * cursor is load-bearing: a shared `cmd` local is a phantom web that pushes the
 * render pointer off t1 and rotates the ring; the vertex word must be built
 * `(0x04 << 24 | p << 16) | 0x30` in gDma1p's order, and the colour bytes need
 * the `& 0xFF` of _SHIFTL so uopt evaluates r before g. Matched 2026-09-16
 * (lane s1-a): 97 to 39 on the block cursor and OR order, 39 to 0 on the masks. */
#define O13_GFX(w0v, w1v) { Overlay13Command *_g = (*commands)++; _g->w0 = (w0v); _g->w1 = (w1v); }

static void overlay13DrawRecord(
    Overlay13Record *record, Overlay13Command **commands, void *mtx, void *vertices) {
    f32 savedScale;
    Overlay13RenderState *render;

    if (((record->state == 1) && (D_20 != 0)) || record->state == 2) {
        render = levelGetLevel();
        savedScale = record->scale;
        if (record->state != 2) {
            sprSetIA2ColOverride(0xFF, 0xFF, 0xFF, render->r, render->g, render->b);
            record->scale = savedScale;
            camDoSprite(commands, mtx, vertices, record, D_20, 6, 0xFF);
            sprClearIA2ColOverride();
        } else {
            texDPTextureX(commands, D_28, 0xE, 0);
            if (record->timer < 0x20) {
                O13_GFX(0xFA000000, ((record->timer * 8) & 0xFF) | 0xFFFFFF00);
            }

            O13_GFX(0x04000000 |
                        ((((((u32)record + record->vertexIndex * 0x28 +
                             0x80000030U) & 6) | 0x20) & 0xFF) << 16) | 0x30,
                    (u32)record + record->vertexIndex * 0x28 + 0x80000030U);
            O13_GFX(0x05110020, (u32)&D_80000000);

            if (D_24 != 0 && record->phase < (f32)(u32)*D_24) {
                record->scale = savedScale * D_4;
                O13_GFX(0xFA000000, ((render->r & 0xFF) << 24) |
                                        ((render->g & 0xFF) << 16) |
                                        ((render->b & 0xFF) << 8) | 0xA0);
                O13_GFX(0xFB000000, 0xFFFFFF00);
                camDoSprite(commands, mtx, vertices, record, D_24, 0xE, 0);
            }
        }

        record->scale = savedScale;
        O13_GFX(0xE7000000, 0);
        O13_GFX(0xFB000000, 0xFFFFFFFF);
        O13_GFX(0xFA000000, 0xFFFFFFFF);
    }
}

/* Frame 0x168 is the two pointer pads plus indices[26] and distances[35].
 * temporaryDistance = 0 before the branch takes the first FP colour. The
 * collection walk reuses i, so that web is coloured before record. if (1)
 * is the region boundary that rematerializes the record base. The view is
 * the resident camGetPtr (shipped SYMBOL record, resident +0x241DC). */
void overlay13DrawActive(void *commands, void *mtx, void *vertices) {
    s32 i;
    void *pad0;
    void *pad1;
    s32 count;
    s32 done;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 temporaryDistance;
    s32 temporaryIndex;
    s32 indices[26];
    f32 distances[35];
    Overlay13View *view;
    Overlay13Record *record;

    temporaryDistance = 0.0f;
    if (gOverlay13Active != 0 && gOverlay13Enabled != 0) {
        view = camGetPtr();
        if (1) {
            record = gOverlay13Records;
            i = 0;
            count = 0;
            do {
                if (record->state != 0) {
                    dx = record->x - view->x;
                    dz = record->z - view->z;
                    dy = record->y - view->y;
                    indices[count] = i;
                    distances[count] = dx * dx + dz * dz + dy * dy;
                    count++;
                }
                record++;
                if (record->state != 0) {
                    dx = record->x - view->x;
                    dz = record->z - view->z;
                    dy = record->y - view->y;
                    indices[count] = i + 1;
                    distances[count] = dx * dx + dz * dz + dy * dy;
                    count++;
                }
                record++;
                i += 2;
            } while (i != 32);
        }
        if (count >= 2) {
            do {
                done = 1;
                for (i = 0; i < count - 1; i++) {
                    if (distances[i + 1] < distances[i]) {
                        temporaryDistance = distances[i];
                        distances[i] = distances[i + 1];
                        distances[i + 1] = temporaryDistance;
                        temporaryIndex = indices[i];
                        indices[i] = indices[i + 1];
                        indices[i + 1] = temporaryIndex;
                        done = 0;
                    }
                }
            } while (done == 0);
        }
        for (i = 0; i < count; i++) {
            overlay13DrawRecord(&gOverlay13Records[indices[i]], commands, mtx, vertices);
        }
    }
}

/* DKR v77/v80 and JFG checks found no exact donor for this no-argument wrapper. */
void overlay13Call(void) {
    overlay13Release_o013Reloc();
}
