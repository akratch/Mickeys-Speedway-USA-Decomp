#include "overlays/overlay_046.h"

/* Module-private helpers (no ORT row): static, so their calls are JUMP
 * records. overlay46InitializeParticles also has no ORT row, but nothing in
 * the module calls it by name (the module's retained data holds a pointer to
 * it), so it keeps external linkage: a static IDO would discard. */
static s32 func_overlay_046_F0000874_188EC6C(s32 updateRate);
static void overlay46InitializeBuffers(void);
static void func_overlay_046_F0001228_188F620(s32 updateRate);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay46InitializeState(void) {
    Overlay46Descriptor descriptor;
    Overlay46Resource *resource;

    loadFrontEndList(gOverlay46Ids);
    gOverlay46Value2C = gOverlay46Value1E4;
    gOverlay46Resource4C = func_80036DD0(2);
    gOverlay46Resource50 = func_80036DD0(0);
    gOverlay46Group54 = overlay44CreateAnimationState(0, 0x7C, 0x41, 0, 0);
    gOverlay46Mode58 = 1;
    gOverlay46Timer5C = 100;
    gOverlay46Value0 = 0xFF;
    gOverlay46Value4 = 0;
    gOverlay46Value8 = 0;
    func_80037414(1, 2.5f, 0.0f, 0, 0, 0, 0);

    descriptor.type = 0x101;
    descriptor.value4 = 0;
    descriptor.value6 = 0;
    descriptor.value8 = 0;
    descriptor.red = 0;
    descriptor.green = 0x40;
    descriptor.blue = 0;
    descriptor.alpha = 0;
    if ((resource = gOverlay46Resource184 =
             func_8000590C(&descriptor, 0)) != NULL) {
        resource->value3C = 0;
    }
    amTunePlay(1);
    func_800015F8();
}

/* Matched 2026-10-01. Four natural spellings together, none of which the
 * colour and force sweeps recorded on the earlier shape could reach:
 * - the name compare is a plain while loop over the two strings with no
 *   current/expected carriers, so the compared byte is a ring temporary;
 * - the first tail command is the same primitive-colour macro as the second,
 *   with no reuse of the match flag as an alpha carrier, which releases the
 *   seventh colour and starts the ring at its first register;
 * - each timer countdown stores the global and reads it back for the test,
 *   which spends the ring draw the copy-into-a-local form skipped;
 * - the status word is a bitfield, whose test spends one more draw and whose
 *   set is the shipped byte update. */
void func_overlay_046_F0000120_188E518(s32 amount) {
    s32 matched;
    s32 value;
    u8 *left;
    u8 *right;
    Overlay46Counter *counter;

    switch (gOverlay46Mode58) {
    case 1:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                gOverlay46Value0 = 0;
                gOverlay46Mode58 = 2;
                gOverlay46Timer5C = 100;
                gOverlay46Value4 = 255;
                func_80037414(1, 2.5f, 0.0f, 0, 0, 0, 0);
            }
        } else if (func_80037664() != 1) {
            gOverlay46Timer5C -= amount;
            if (gOverlay46Timer5C <= 0) {
                gOverlay46Timer5C = 0;
                overlay99InitializeEntries(2, D_60, 2.5f, -1.0f);
                func_80000F94(mathRnd(0x203, 0x204) & 0xFFFF, 0);
            }
        }
        break;
    case 2:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                func_80037658();
                gOverlay46Value4 = 0;
                gOverlay46Mode58 = 3;
                gOverlay46Timer5C = 0;
            }
        } else if (func_80037664() != 1) {
            gOverlay46Timer5C -= amount;
            if (gOverlay46Timer5C <= 0) {
                gOverlay46Timer5C = 0;
                overlay99InitializeEntries(2, D_C0, 2.5f, -1.0f);
                func_80000F94(mathRnd(0x203, 0x204) & 0xFFFF, 0);
            }
        }
        break;
    case 3:
        overlay44UpdateFrameCache(gOverlay46Group54Update, amount);
        counter = gOverlay46Group54Update;
        if (counter->value < 0x200) {
            gOverlay46Value8 = counter->value >> 1;
        } else {
            gOverlay46Value8 = 255;
        }
        if (counter->value >= 0x1E01) {
            if (gOverlay46Timer5C == 0) {
                overlay99InitializeEntries(2, D_120, 2.5f, -1.0f);
                func_80000F94(mathRnd(0x203, 0x204) & 0xFFFF, 0);
                gOverlay46Timer5C = 1;
            } else if (func_80037664() != 1) {
                gOverlay46Value8 = 0;
                gOverlay46Mode58 = 4;
                func_overlay_046_F000069C_188EA94();
            }
        }
        break;
    case 4:
        if (func_overlay_046_F0000874_188EC6C(amount) == 0) {
            gOverlay46Mode58 = 5;
        }
        break;
    case 5:
        matched = 0;
        if (!D_800D3128.nameMatched) {
            right = func_80058240();
            if (right != 0) {
                left = gOverlay46ExpectedName180;
                right += 0x34;
                matched = 1;
                while (*left != 0) {
                    if (*left++ != *right++) {
                        matched = 0;
                        break;
                    }
                }
            }
        }
        if (matched != 0) {
            D_800D3128.nameMatched = 1;
            func_800291B4();
            D_8007BF44 = 0;
            func_8003A680(10);
            D_8007BF48 = -1;
            func_80001608();
            func_80028374(0x12, 0, 0, 0xF, 1, 0);
            func_80028528(1);
        } else {
            func_80028D30(0);
        }
        gOverlay46Mode58 = 6;
        break;
    }

    if (gOverlay46Value4 != 0) {
        O46_PRIM(gDisplayListHead++, gOverlay46Value4, gOverlay46Value4,
                 gOverlay46Value4, 0xFF);
        func_80036F08(&gDisplayListHead, gOverlay46Resource50, 0);
    }

    if (gOverlay46Value0 != 0) {
        O46_PRIM(gDisplayListHeadB++, gOverlay46Value0, gOverlay46Value0,
                 gOverlay46Value0, 0xFF);
        func_80036F08(&gDisplayListHeadB, gOverlay46Resource4C, 0);
    }

    value = gOverlay46Value8;
    if (value != 0) {
        gOverlay46Group54Render->alpha = value;
        func_overlay_044_F0000580_188BDE0(gOverlay46Group54Render,
                                          gOverlay46MotionTarget, 1.0f);
        fontUseFont(2);
        fontBackground(0, 0, 0, 0);
        func_8004B0B8(255, 255, 255, 255, gOverlay46Value8);
    }
}

void overlay46ReleaseState(void) {
    Overlay46Resource_ReleaseState *resource;

    freeFrontEndList(gOverlay46Data.resourceIds);
    overlay44ReleaseHandles(gOverlay46Data.group54);

    if (gOverlay46Data.resource50 != NULL) {
        resource = gOverlay46Data.resource50;
        mmFree(resource);
    }
    if (gOverlay46Data.resource4C != NULL) {
        resource = gOverlay46Data.resource4C;
        mmFree(resource);
    }
    if (gOverlay46Data.resource184 != NULL) {
        func_80006EA0(gOverlay46Data.resource184);
    }

    frontSetScreenMode(frontRecallScreenMode());
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
/*
 * IDO 5.3 uses the loop opener's source-line boundary when scheduling the
 * independent resource-table address and loop count initializations. Keeping
 * the count assignment with the do opener reproduces the retail order.
 */
void overlay46InitializeParticles(void) {
    Overlay46Particle *particle;
    Overlay46ParticleConfig *config;
    void **resources;
    f32 offsetY;
    f32 baseY;
    f32 spacing;
    s32 magnitude;
    s32 index;

    particle = gOverlay46Particles20;
    config = gOverlay46Configs1C8;
    spacing = 0.025f;
    magnitude = -0x4000;
    resources = D_800D31C8;
    index = 0x12; do {
        particle->value00 = 0;
        particle->value02 = 0;
        particle->value04 = 0;
        particle->angle06 = mathRnd(0x100, 0x200);
        particle->scale08 = 1.0f;
        particle->baseX0C = config->x;
        particle->baseY10 = config->y;
        particle->value14 = 0.0f;
        particle->angle18 = mathRnd(0, 0x8000);
        particle->angle1A = mathRnd(0x600, 0xA00);
        particle->positionX1C =
            func_8002A8C0(magnitude) * 400.0f +
            particle->baseX0C;
        offsetY = func_8002A8BC(magnitude) * 400.0f;
        baseY = particle->baseY10;
        particle->value28 = 0.0f;
        particle->positionY20 = offsetY + baseY;
        particle->value24 = (f32)index * spacing;
        particle->resource38 = resources[config->resourceIndex];
        particle->value34 = 0;
        particle->targetY30 = baseY;
        particle->targetX2C = particle->baseX0C;
        particle->variant36 = config->variant;
        if (func_800299E8(0, 1) != 0) {
            particle->angle06 = -particle->angle06;
        }
        magnitude += 0x1000;
        particle++;
        config++;
    } while (index--);

    gOverlay46StateC = 1;
    gOverlay46Value10 = 0;
    gOverlay46Value14 = 0;
    gOverlay46Value18 = 0;
    gOverlay46Timer5C = 0x3C;
}

/* Pinned DKR v77/v80 and JFG skeleton scans found no close donor.
 *
 * Matched 2026-10-01 by writing what the relocation records name instead of
 * the inherited placeholders:
 * - the three per-state step factors and the two captions are literals in
 *   this TU's own pool (LOCAL records against the module rodata), not
 *   globals, so each step is a hoisted invariant and the state-2 step shares
 *   the one compiler temporary with the draw loop's spilled walker;
 * - every "render data" argument is the address of one of three resident
 *   cursors (display list, matrix, vertex), not a loaded value;
 * - the primitive-colour command is the one-line packet macro, whose
 *   block-scoped pointer is the home below the table. */
static s32 func_overlay_046_F0000874_188EC6C(s32 updateRate) {
    s32 finished;
    s32 count;
    s32 result;
    Overlay46Particle_F0000874_188 *particle;
    s32 pad; /* fifth scalar home above the table (L99) */
    Overlay46Particle_F0000874_188 *particlesByVariant[19];

    func_80037658();
    result = 1;
    texDPInit(&D_800D3140);
    camStandardOrtho(&D_800D3140, &D_800D3144);

    count = 0x12;
    do {
        particlesByVariant[count] = NULL;
    } while (count--);

    switch (D_C) {
    case 1: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 += updateRate * 0.01f;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }
            particle->baseX0C =
                particle->positionX1C
                + ((particle->targetX2C - particle->positionX1C)
                   * particle->progress24);
            particle->baseY10 =
                particle->positionY20
                + ((particle->targetY30 - particle->positionY20)
                   * particle->progress24);
            particle->value04 = (s16)(2.0f * (particle->progress24 * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            D_C = 2;
            particle = D_20;
            count = 0x12;
            do {
                particle->progress24 = 0.0f;
                particle++;
            } while (count--);
        }
        break;
    }

    case 2: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 += updateRate * 0.05f;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }

            particle->angle18 =
                particle->angle18 + (particle->angle1A * updateRate);
            if (particle->angle18 >= 0x8001) {
                particle->angle18 = particle->angle18 - 0x8000;
                particle->angle1A = mathRnd(0x600, 0xA00);
            }

            particle->angle34 += particle->angle06 * updateRate;
            if (particle->angle34 < -0x1000) {
                particle->angle34 = -0x1000;
                particle->angle06 = mathRnd(0x100, 0x200);
            } else if (particle->value04 >= 0x1001) {
                particle->angle34 = 0x1000;
                particle->angle06 =
                    -mathRnd(0x100, 0x200);
            }

            particle->baseX0C = particle->targetX2C;
            particle->baseY10 =
                (func_8002A8C0((u16)particle->angle18) *
                 particle->progress24 * 5.0f) + particle->targetY30;
            particle->value04 =
                (s16)((f32)particle->angle34 * particle->progress24);
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            if (D_10 < 0xFF) {
                D_10 += updateRate * 4;
                if (D_10 >= 0x100) {
                    D_10 = 0xFF;
                }
            } else if (D_14 < 0xFF) {
                D_14 += updateRate * 4;
                if (D_14 >= 0x100) {
                    D_14 = 0xFF;
                }
            } else if (D_18 < 0xFE) {
                D_18 += updateRate * 4;
                if (D_18 >= 0xFF) {
                    D_18 = 0xFE;
                }
            } else {
                D_5C -= updateRate;
                if (D_5C <= 0) {
                    D_C = 4;
                }
            }
        }
        break;
    }

    case 4: {
        particle = D_20;
        finished = 1;
        count = 0x12;
        do {
            particle->progress24 -= updateRate * 0.025f;
            if (particle->progress24 <= 0.0f) {
                particle->progress24 = 0.0f;
            } else {
                finished = 0;
            }
            particle->baseX0C =
                particle->positionX1C
                + ((particle->targetX2C - particle->positionX1C)
                   * particle->progress24);
            particle->baseY10 =
                particle->positionY20
                + ((particle->targetY30 - particle->positionY20)
                   * particle->progress24);
            particle->value04 =
                particle->angle34
                + (s32)(2.0f * (particle->progress24 * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        D_10 -= updateRate * 8;
        if (D_10 < 0) {
            D_10 = 0;
        }
        D_14 -= updateRate * 8;
        if (D_14 < 0) {
            D_14 = 0;
        }
        D_18 -= updateRate * 8;
        if (D_18 < 0) {
            D_18 = 0;
        }
        if (finished != 0) {
            result = 0;
        }
        break;
    }
    }

    fontUseFont(2);
    fontBackground(0, 0, 0, 0);
    if (D_10 != 0) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, D_10);
        fontPrintXY(&D_800D3140, 0xA0, 0xAC, "< 2000 Disney",
                                0xC);
    }
    if (D_14 != 0) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, D_14);
        fontPrintXY(&D_800D3140, 0xA0, 0xB6, "Licensed to Nintendo",
                                0xC);
    }
    if (D_18 != 0) {
        func_8002F618(&D_800D3140, D_2C, 0xA0, 0xCC,
                                  0xFF, 0xFF, 0xFF, D_18);
    }

    texDPInit(&D_800D3140);
    O46_PRIM(D_800D3140++, 255, 255, 255, 255);

    count = 0;
    do {
        particle = particlesByVariant[count++];
        if (particle != NULL) {
            camDo2DSprite(
                &D_800D3140, &D_800D3144,
                &D_800D3148, particle, particle->resource38,
                0x8001, 0xFF);
        }
    } while (count != 19);

    return result;
}

/* DKR v77/v80 and JFG checks found no exact donor for this state initializer. */
void overlay46InitState(void) {
    gOverlay46Enabled = 1;
    gOverlay46Limit = 0x5A;
    loadFrontEndList(gOverlay46StateA);
    setupFrontEndList(gOverlay46StateB);
    overlay46InitializeBuffers();
    gOverlay46Selection = 0;
}

/* Pinned DKR v77/v80 and JFG scans found no donor inside this owner. */
void overlay46UpdateTransition(s32 updateRate) {
    s32 finished;

    finished = 0;
    camSetView(gOverlay46DisplayState, 0);
    func_80022B94(gOverlay46DisplayState, &gOverlay46DisplayOutput);

    switch (gOverlay46Mode58) {
        case 1:
            if (gOverlay46Timer5C > 0) {
                gOverlay46Timer5C -= updateRate;
            }
            if (overlay41IsUnitScale(0) != 0) {
                gOverlay46Mode58 = 6;
                gOverlay46Timer5C = 270;
            }
            break;
        case 6:
            gOverlay46Timer5C -= updateRate;
            if (gOverlay46Timer5C <= 0) {
                gOverlay46Mode58 = 0;
                func_80028D30(0);
            }
            finished = 1;
            break;
        case 0:
            finished = 1;
            break;
    }

    if (finished != 0) {
        gOverlay46Selection += updateRate * 2;
        if (gOverlay46Selection >= 256) {
            gOverlay46Selection = 255;
        }
        gOverlay46FadeOutput = gOverlay46Selection;
        frontDrawObj(5);
        gOverlay46FadeOutput = 255;
    }

    func_overlay_046_F0001228_188F620(updateRate);
}

void overlay46Submit(void) {
    freeFrontEndList(gOverlay46SubmitBlock);
}

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
static void overlay46InitializeBuffers(void) {
    Overlay46ColorEntry *color;
    Overlay46BufferSlot *slot;
    s32 i;

    gOverlay46State = &gOverlay46StateStorage;
    gOverlay46State->value10 = 0;
    gOverlay46State->w = gOverlay46InitialW;
    gOverlay46State->x = gOverlay46InitialX;
    gOverlay46State->y = gOverlay46InitialY;
    gOverlay46State->z = -300.0f;
    gOverlay46State->value14 = 0;

    color = gOverlay46ColorEntries;
    i = 0;
    while (i < 400) {
        i++;
        color->red = 0xFF;
        color->green = 0xFF;
        color->blue = 0xFF;
        color->alpha = 0xFF;
        color++;
    }
    i = 0;

    gOverlay46Value = gOverlay46InitialValue;

    slot = &gOverlay46BufferSlots[0]; do {
        slot->value2C = -1;
        slot->value44 = -1;
        slot->value5C = -1;
        slot->value14 = -1;
        slot++;
    } while (slot != &gOverlay46BufferSlotsEnd[0]);

    gOverlay46BufferEnd = 0;
}

/* Overlay 46 text +0x1228..+0x195C, the trail and spark draw. Matched
 * 2026-10-02 (lanes o-ovl7, p-ovl8) from 415 masked words at -96, rewritten
 * from the target listing in the matched sibling
 * func_overlay_046_F0000874_188EC6C's shape: the draw point is a 0x34-byte
 * stack struct, the cursors are passed by address, each command is one
 * packet macro, the emitter is read through D_1450 at every use, and the
 * spawn and draw loops are indexed. The last 8 words were the batch vertex
 * count n+2: it is a variable k assigned on its own line (a0 in the loop),
 * and the remainder block spells `count % 16` at each use with no variable,
 * so its k web is numbered ahead of the expression webs (v1). */
static void func_overlay_046_F0001228_188F620(s32 updateRate) {
    Overlay46TrailVertex *vertex;
    s32 i;
    s32 j;
    s32 batches;
    s32 remaining;
    f32 halfX;
    f32 halfY;
    Overlay46DrawPoint point;
    s32 count;
    s32 k;

    if (D_19C < D_494) {
        D_494 -= updateRate;
        if (D_494 + updateRate > 0 && D_494 <= 0) {
            amSndPlay(0x11, NULL);
        }
    }
    if (D_494 > 0) {
        return;
    }

    if (D_19C < D_494) {
        D_2718 = 0xFF;
        for (i = 0; i < updateRate; i++) {
            if (D_1450->vertexCount < 400) {
                vertex = &D_498[D_1450->vertexCount];
                D_1450->x += D_1450->speed * func_8002A8C0(D_1450->angle);
                D_1450->y += D_1450->speed * func_8002A8BC(D_1450->angle);
                halfX = func_8002A8C0(D_1450->angle + 0x4000) * 5.0f;
                halfY = func_8002A8BC(D_1450->angle + 0x4000) * 5.0f;
                vertex->x = D_1450->x - halfX;
                vertex->y = D_1450->y - halfY;
                vertex->z = D_1450->z;
                vertex++;
                vertex->x = D_1450->x + halfX;
                vertex->y = D_1450->y + halfY;
                vertex->z = D_1450->z;
                D_1450->angle -= D_194;
                D_1450->vertexCount += 2;
            }
            for (j = 0; j < 200; j++) {
                if (D_1458[j].age < 0) {
                    D_1458[j].age = 0;
                    D_1458[j].x = D_1450->x;
                    D_1458[j].y = D_1450->y;
                    D_1458[j].z = D_1450->z;
                    D_1458[j].alpha = 0xFF;
                    D_1458[j].velocityX = (f32)mathRnd(-500, 500) / 500.0f;
                    D_1458[j].velocityY = (f32)mathRnd(-500, 500) / 500.0f;
                    j = 200;
                }
            }
        }
    } else {
        D_2718 -= updateRate * 8;
        if (D_2718 < 0) {
            D_2718 = 0;
        }
    }

    camSetView(&D_800D3140, &D_800D3144);
    point.value04 = 0;
    point.value00 = 0;
    point.value02 = 0;
    point.scale08 = 6.0f;
    point.value28 = 0.0f;
    for (i = 0; i < 200; i++) {
        if (D_1458[i].age != -1) {
            point.baseX0C = D_1458[i].x;
            point.baseY10 = D_1458[i].y;
            point.value14 = D_1458[i].z;
            gDPSetPrimColor(D_800D3140++, 0, 0, 0xA0, 0xA0, 0xA0, D_1458[i].alpha);
            camDoSprite(&D_800D3140, &D_800D3144, &D_800D3148,
                          &point, D_800D31C8[9], 7, D_1458[i].alpha);
        }
    }
    texDPTextureX(&D_800D3140, NULL, 3, 0);
    gDPSetPrimColor(D_800D3140++, 0, 0, 0xC0, 0xC0, 0xC0, 0xFF);

    vertex = D_498;
    count = D_1450->vertexCount - 2;
    batches = count / 16;
    remaining = batches * 16;
    for (i = 0; i < batches; i++) {
        if (remaining >= 16) {
            j = 16;
        } else {
            j = remaining;
        }
        k = j + 2;
        O46_SP_VERTEX(D_800D3140++, O46_PHYSICAL(vertex), k, 0);
        O46_SP_POLYGON(D_800D3140++, O46_PHYSICAL(D_230), j, 1);
        remaining -= j;
        vertex += j;
    }
    if (count % 16 != 0) {
        k = count % 16 + 2;
        O46_SP_VERTEX(D_800D3140++, O46_PHYSICAL(vertex), k, 0);
        O46_SP_POLYGON(D_800D3140++, O46_PHYSICAL(D_230), count % 16, 1);
    }

    if (D_2718 > 0) {
        point.baseX0C = D_1450->x;
        point.baseY10 = D_1450->y;
        point.value14 = D_1450->z;
        point.scale08 = 6.0f;
        gDPSetPrimColor(D_800D3140++, 0, 0, 0xA0, 0xA0, 0xA0, D_2718);
        camDoSprite(&D_800D3140, &D_800D3144, &D_800D3148,
                      &point, D_800D31C8[10], 7, D_2718);
    }

    for (i = 0; i < 200; i++) {
        if (D_1458[i].age != -1) {

            D_1458[i].age += updateRate;
            D_1458[i].x += D_1458[i].velocityX * (f32)updateRate;
            D_1458[i].y += D_1458[i].velocityY * (f32)updateRate;
            if (D_1458[i].age >= 61) {
                D_1458[i].alpha -= updateRate * 8;
                if (D_1458[i].alpha < 0) {
                    D_1458[i].age = -1;
                }
            }
        }
    }
}

/* Tier B: actual constructor/update loops and qualified runtime ranges
 * authenticate these storage capacities. Tier D: these are bounded storage
 * views, not recovered original declarations. Existing particle, root and
 * spark/group consumer views remain separate source-type debt. */
s32 gOverlay46StorageValue0;
s32 gOverlay46StorageValue4;
s32 gOverlay46StorageValue8;
s32 gOverlay46StorageStateC;
s32 gOverlay46StorageValue10;
s32 gOverlay46StorageValue14;
s32 gOverlay46StorageValue18;
Overlay46Particle gOverlay46StorageParticles[19];
s16 gOverlay46StorageCountdown;
s16 gOverlay46StorageSelection;
Overlay46TrailVertex gOverlay46StorageVertices[400];
Overlay46Emitter gOverlay46StorageEmitter;
Overlay46Emitter *gOverlay46StorageEmitterPointer;
Overlay46Spark gOverlay46StorageSparks[200];
s16 gOverlay46StorageAlpha;
