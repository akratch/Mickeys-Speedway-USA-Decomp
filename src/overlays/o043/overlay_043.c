#include "overlays/overlay_043.h"

/* Tier B: clone submission reads these two complete float matrices through
 * mtxf_mul. Their backing is distinct from the following viewport record;
 * existing loader operands and the renderer's resource contracts are retained. */
MtxF o43CloneMatrixA = {
    {0.03125f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.03125f, 0.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

MtxF o43CloneMatrixB = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

/* Module-private helpers (no ORT row): static, so their calls are JUMP
 * records. Unprototyped here: each caller passes its own view of the clone
 * state, and no argument is a float. */
static void func_overlay_043_F0000BE4_188ABB4();
static void func_overlay_043_F00010A8_188B078();
static s32 func_overlay_043_F0001184_188B154();
static void overlay43SubmitChildren();
static void overlay43FilterImage();

s32 func_overlay_043_F0000000_1889FD0(Overlay43Input *input) {
    Overlay43Command *command;
    Overlay43State *state;
    void *selectedObject;
    void **selected;
    s8 linkIndex;

    state = input->source->state;
    if (func_overlay_043_F0001184_188B154(input, state) == 0) {
        return 0;
    }

    state->alpha = 1.0f;
    state->current = input->entries[input->currentIndex];
    linkIndex = input->parent->linkIndex;
    if (linkIndex >= 0) {
        selected = state->current;
        state->parent = input->entries[linkIndex];
    } else {
        selected = state->current;
        state->parent = selected;
    }
    selectedObject = *selected;
    if (((u8 *)selectedObject)[0x2F] != 0) {
        state->enabled = 1;
    }

    overlay43SubmitChildren(input, state, 0x800, 0xB);
    command = (Overlay43Command *)(state->bufferEnd - 0x20);
    command->type = 1;
    command->mode = 5;
    command->byte03 = 0;
    command->short04 = 0;
    command->width = 0x40;
    command->height = 0x40;
    command->short0A = 0;
    command->short0C = 0;
    command->scale = 0x1000;
    command->short10 = 0;
    command->short12 = 0;
    command->data = D_0;
    command->short18 = 0x38;
    command->byte1A = 0;
    command->byte1B = 0;
    command->byte1C = 2;
    command->byte1D = 0;
    command->byte1E = 2;
    command->byte1F = 0;
    state->command = command;

    state->x = input->source->x;
    state->y = input->source->y;
    state->flags = input->source->flags | 0x10;
    state->byte11 = input->source->byte11;
    state->byte12 = input->source->byte12;
    state->byte13 = 0;
    state->short0C = 0;
    state->short0E = 0;
    return 1;
}

void func_overlay_043_F0000194_188A164(void) {
    Overlay43PendingEntry *entry;
    Overlay43PendingEntry *previous;
    s32 message;
    s32 index;

    message = 0;
    for (index = 0; index < D_C8.pendingCount; index++) {
        entry = D_120[index];
        if (index > 0) {
            previous = D_120[index - 1];
        } else {
            previous = NULL;
        }
        if (entry->pending != 0) {
            rcpFast3d(entry->arg28, entry->arg30, 3, entry->arg20);
            entry->pending = 0;
            if (previous != NULL) {
                overlay43FilterImage(previous);
            }
            osRecvMesg(ext_4d258, &message, 1);
        }
    }
    overlay43FilterImage(entry);
    D_C8.pendingCount = 0;
}

void func_overlay_043_F0000280_188A250(Overlay43ResourceOwner *owner) {
    s32 index;

    func_8002B768(owner->resource24);
    func_8002B768(owner->resource28);
    func_8002B768(owner->resource2C);
    if (owner->resource34 != 0) {
        func_8002B768(owner->resource34);
    }
    if (owner->resource68 != 0) {
        func_80020184(owner->resource68);
    }
    for (index = 0; index < owner->childCount; index++) {
        func_8002B768(owner->children[index]);
    }
}

s32 func_overlay_043_F0000324_188A2F4(Overlay43ProjectInput *input, s32 useNodes,
                                      f32 updateRate) {
    Overlay43ModelDefinition *definition;
    Overlay43ProjectState *state;
    Overlay43Level *level;
    Overlay43ModelInstance *model;
    Overlay43Record *records[8];
    Overlay43NodeSet *nodeSet;
    /* Node x, then a vertex's scaled radius, then the image diameter. */
    f32 scratchA;
    Overlay43Transform transform;
    MtxF matrix;
    /* Node z, the blend fraction, the minimum x and the tolerance test. */
    f32 scratchB;
    f32 nodeY;
    f32 minimumZ;
    f32 maximumX;
    f32 x;
    f32 y;
    f32 z;
    f32 maximumZ;
    Overlay43Node *node;
    s32 i;
    s32 j;
    f32 extent;
    Overlay43Record *swap;
    s32 priority;
    Overlay43VertexPosition *vertex;
    s32 recordCount;
    s16 yaw;
    s16 pitch;
    s16 rotation[2];
    Overlay43Record *record;

    state = input->link->state;
    model = state->model;
    if (model->active != 0) {
        if (state->displayListEnd == NULL) {
            return 0;
        }
        state->pending = 2;
        gO43RenderPending = 1;
        gO43QueueReloc[gO43QueueCount] = state;
        gO43QueueCount++;
        return 1;
    }
    definition = model->definition;
    level = levelGetLevel();
    recordCount = 0;

    if (useNodes != 0) {
        nodeSet = input->nodeSet;
        if ((nodeSet != NULL) && (nodeSet->count >= 2)) {
            for (i = 1; i < nodeSet->count; i++) {
                gO43Records[recordCount].node = &nodeSet->nodes[i];
                recordCount++;
            }

            for (i = 0; i < recordCount; i++) {
                records[i] = &gO43Records[i];
            }

            for (i = recordCount - 1; i > 0; i--) {
                for (j = 0; j < i; j++) {
                    if (records[j]->node->priority <
                        records[j + 1]->node->priority) {
                        swap = records[j];
                        records[j] = records[j + 1];
                        records[j + 1] = swap;
                    }
                }
            }

            if (state->maximumRecords < recordCount) {
                recordCount = state->maximumRecords;
            }

            if (recordCount != 0) {
                node = records[0]->node;
                scratchA = node->x;
                scratchB = node->z;
                nodeY = node->y;
                yaw = Arctanf(-scratchA, -scratchB);
                pitch = Arctanf(
                    nodeY, sqrtf((scratchA * scratchA) + (scratchB * scratchB)));

                if (node->priority < level->priority) {
                    scratchB = (1.0f / (f32)level->priority) *
                               (f32)node->priority;
                    rotation[0] = dAngle(level->yaw, yaw, scratchB);
                    rotation[1] = dAngle(level->pitch, pitch, scratchB);
                    rotation[0] = dAngle(rotation[0], state->yaw, scratchB);
                    rotation[1] = dAngle(rotation[1], state->pitch, scratchB);
                    state->yaw = rotation[0];
                    state->pitch = rotation[1];
                    priority = level->priority;
                } else {
                    rotation[0] = yaw;
                    rotation[1] = pitch;
                    state->yaw = rotation[0];
                    state->pitch = rotation[1];
                    priority = node->priority;
                }

                func_overlay_043_F00010A8_188B078(rotation, priority, records[0]);
                for (i = 1; i < recordCount; i++) {
                    node = records[i]->node;
                    scratchA = node->x;
                    scratchB = node->z;
                    nodeY = node->y;
                    rotation[0] = Arctanf(-scratchA, -scratchB);
                    rotation[1] = Arctanf(
                        nodeY, sqrtf((scratchA * scratchA) + (scratchB * scratchB)));
                    priority = node->priority;
                    func_overlay_043_F00010A8_188B078(rotation, priority, records[i]);
                }
            }
        } else if (level->priority > 0) {
            rotation[0] = level->yaw;
            rotation[1] = level->pitch;
            state->yaw = rotation[0];
            state->pitch = rotation[1];
            priority = level->priority;
            records[0] = gO43Records;
            recordCount = 1;
            func_overlay_043_F00010A8_188B078(rotation, priority, gO43Records);
        }
    } else if (level->priority > 0) {
        records[0] = gO43Records;
        gO43Fallback.alpha = level->priority;
        recordCount = 1;
        record = records[0];
        func_8002A82C(record->matrix);
        record->matrix[1][1] = 0.0f;
    } else {
        return 0;
    }

    transform.x = -input->x;
    transform.y = -input->y;
    transform.z = -input->z;
    transform.roll = 0;
    transform.pitch = 0;
    transform.yaw = 0;
    func_8002AE10(&transform, state->transform);

    if (state->boundsMode == 0) {
        extent = input->scaleSource->scale * input->scale;
    } else if (state->boundsMode == 1) {
        scratchB = 32000.0f;
        minimumZ = 32000.0f;
        maximumX = -32000.0f;
        maximumZ = -32000.0f;
        for (j = 0; j < recordCount; j++) {
            mtxf_mul(state->transform, records[j]->matrix, matrix);
            for (i = 0; i < definition->vertexCount; i++) {
                vertex = &model->positions[i];
                scratchA = definition->bounds[i].radius * input->scale;
                mtxf_transform_point(matrix, vertex->x, vertex->y,
                                                vertex->z, &x, &y, &z);
                if (maximumX < x + scratchA) {
                    maximumX = x + scratchA;
                }
                if (x - scratchA < scratchB) {
                    scratchB = x - scratchA;
                }
                if (maximumZ < z + scratchA) {
                    maximumZ = z + scratchA;
                }
                if (z - scratchA < minimumZ) {
                    minimumZ = z - scratchA;
                }
            }
        }
        if (scratchB < 0.0f) {
            scratchB = -scratchB;
        }
        if (minimumZ < 0.0f) {
            minimumZ = -minimumZ;
        }
        if (maximumX < scratchB) {
            maximumX = scratchB;
        }
        if (maximumZ < minimumZ) {
            maximumZ = minimumZ;
        }
        if (maximumZ < maximumX) {
            scratchA = maximumX + 10.0f;
        } else {
            scratchA = maximumZ + 10.0f;
        }
        scratchA *= 2.0f;
        scratchB = (100.0f / state->diameter) * scratchA;
        scratchB -= 100.0f;
        if ((scratchB > -5.0f) && (scratchB < 5.0f)) {
            scratchA = state->diameter;
        } else {
            state->diameter = scratchA;
        }
        state->scaleX = 0.05f * scratchA;
        state->scaleY = 0.05f * scratchA;
        extent = 64.0f / scratchA;
    } else if (state->boundsMode == 2) {
        extent = state->fallbackScale;
    }

    func_8002A82C(matrix);
    matrix[0][0] = extent;
    matrix[1][1] = extent;
    matrix[2][2] = extent;
    mtxf_mul(state->transform, matrix, state->transform);
    func_overlay_043_F0000BE4_188ABB4(input, records, recordCount);
    return 1;
}

static void func_overlay_043_F0000BE4_188ABB4(Overlay43RenderInput *input,
                                       Overlay43RenderEntry **entries,
                                       s32 count) {
    Gfx *dl;
    Overlay43Model *model;
    Mtx *mtx;
    s32 i;
    s32 alpha;
    Overlay43VertexData *vertexData;
    Overlay43RenderState *state;
    MtxF matA;
    MtxF matB;
    Gfx *start;
    s32 unused;

    state = input->link->state;
    model = state->model;
    vertexData = state->vertexData;
    if (gO43QueueCount < 15) {
        start = state->displayList;
        mtx = state->matrixHeap;
        dl = start;
        diRcpTrace(start, gO43TraceFile0, 596);
        rsp_segment(&dl, 0, 0);
        rsp_segment(&dl, 1, state->framebuffer);
        gDPSetCycleType(dl++, G_CYC_FILL);
        gDPSetColorImage(dl++, G_IM_FMT_I, G_IM_SIZ_8b, 64,
                         OS_K0_TO_PHYSICAL(state->framebuffer));
        gDPSetFillColor(dl++, 0);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, 64, 64);
        gDPFillRectangle(dl++, 0, 0, 63, 63);
        rcpInitDpNoSize(&dl);
        gSPClearGeometryMode(dl++, 0x1F3204);
        gSPTexture(dl++, 0, 0, 0, 0, 0);
        gMoveWd(dl++, 0x02, 0, 0);
        gSPViewport(dl++, OS_K0_TO_PHYSICAL(gO43Viewport));
        gSPClipRatio(dl++, FRUSTRATIO_2);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 1, 1, 63, 63);
        for (i = count - 1; i >= 0; i--) {
            alpha = entries[i]->alpha;
            gDPSetPrimColor(dl++, 0, 0, alpha, alpha, alpha, 255);
            gDPSetEnvColor(dl++, alpha, alpha, alpha, 255);
            mtxf_mul(state->transform, entries[i]->matrix, matA);
            mtxf_mul(matA, gO43CloneMatrixB, matA);
            mtxf_mul(matA, gO43CloneMatrixA, matB);
            mtxf_to_mtx(matB, mtx);
            gSPMatrix(dl++, OS_K0_TO_PHYSICAL(mtx), 0);
            {
                Gfx *_g = (Gfx *)(dl++);
                _g->words.w0 = _SHIFTL(0xBF, 24, 8) |
                    _SHIFTL(OS_K0_TO_PHYSICAL(model->parts[model->partIndex]),
                            0, 24);
                _g->words.w1 = OS_K0_TO_PHYSICAL(vertexData->vertices);
            }
            gDma0p(dl++, 0x02, OS_K0_TO_PHYSICAL(gO43VertexSourceReloc), 0x50);
            gSPDisplayList(dl++, OS_K0_TO_PHYSICAL(state->modelDisplayList));
            gImmp1(dl++, 0xBF, 0);
            mtx++;
        }
        gMoveWd(dl++, 0x0A, 0, 0);
        gDPFullSync(dl++);
        gSPEndDisplayList(dl++);
        diRcpTrace(dl, gO43TraceFile1, 675);
        state->displayListEnd = dl;
        state->pending = 1;
        D_8007A3C8 = 1;
        gO43Queue[gO43QueueCount] = state;
        gO43QueueCount++;
    }
}

/* Matched by writing the scale as the float literal it is. The shipped load
 * reads the overlay's own constant pool, so the two uses are one value the
 * compiler may keep across the intervening stores; read through a data
 * symbol it has to be carried in a local, and that carrier is what took the
 * divisor's register. The saved direction components are plain locals and
 * one unreferenced leading local supplies the frame cell the carrier held.
 */
static void func_overlay_043_F00010A8_188B078(Overlay43RotationInput *input,
                                      s32 owner,
                                      Overlay43MotionOutput *output) {
    s32 pad;
    Vec3f direction;
    f32 x;
    f32 y;
    f32 z;

    if (input->angle < 0) {
        input->angle = 0;
    }
    input->angle -= 0x4000;
    input->angle >>= 1;
    input->angle += 0x4000;

    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = -1.0f;
    func_80029FE4(input, &direction);
    x = direction.x;
    y = direction.y;
    z = direction.z;
    output->owner = owner;
    func_8002A82C(output);

    output->unk00 = 1.2f;
    output->unk10 = -(x / y);
    output->unk14 = 0.0f;
    output->unk18 = -(z / y);
    output->unk28 = 1.2f;
}

static s32 func_overlay_043_F0001184_188B154(Overlay43AllocInput *input,
                                      Overlay43Resources *resources) {
    s32 pad[2];
    s32 secondaryCount;
    s32 primaryCount;

    resources->primary = mmAlloc(0x10B4, 0x87);
    if (resources->primary == NULL) {
        return 0;
    }

    resources->aligned = (u8 *)
        ((((u32)resources->primary + 0x20) & ~0x3F) + 0x80);
    if (input->mode == 1) {
        primaryCount = 0x28;
        secondaryCount = 6;
    } else {
        primaryCount = 0x3C;
        secondaryCount = 0xA;
    }

    resources->secondary = mmAlloc(primaryCount * 8, 0x87);
    if (resources->secondary == NULL) {
        func_8002B768(resources->primary);
        return 0;
    }

    resources->tertiary = mmAlloc(secondaryCount << 6, 0x87);
    if (resources->tertiary == NULL) {
        func_8002B768(resources->primary);
        func_8002B768(resources->secondary);
        return 0;
    }

    return 1;
}

static void overlay43SubmitChildren(Overlay43Owner *owner,
                             Overlay43ChildState *state,
                             s32 renderFlags,
                             s32 renderContext) {
    Overlay43ChildRoot *root;
    Overlay43Renderable **childRef;
    Overlay43Renderable *primary;
    s32 sourceCount;
    u8 normalizedCount;
    s32 index;
    s32 childOffset;

    primary = *state->primaryRef;

    makeModelGfx(
        &state->primarySlot,
        primary,
        renderFlags,
        renderContext,
        0,
        0xFF,
        1);

    if (owner->mode == 1) {
        return;
    }
    if (owner->gate->enabled <= 0) {
        return;
    }

    root = owner->children->root;
    index = 0;
    sourceCount = root->counts->childCount;
    normalizedCount = sourceCount & 0xFF;
    state->childCount = sourceCount;
    if (normalizedCount <= 0) {
        return;
    }

    childOffset = 0;
    do {
        childRef = *(Overlay43Renderable ***)
            ((u8 *)root->childRefs + childOffset);
        makeModelGfx(
            &state->childSlots[index],
            *childRef,
            renderFlags,
            renderContext,
            0,
            0xFF,
            1);

        childOffset += 4;
        index++;
    } while (index < state->childCount);
}

/* PROVENANCE: no code adapted. The pinned DKR/JFG exact-donor scan is negative.
 * A function-specific structural scan found JFG assembly-only
 * func_overlay_4_000015A8_1EF7898 as a 0.574 masked-skeleton relative; it
 * supplies no donor C. */
/* Matched 2026-09-11.  The 16-word residual was the colouring of the row,
 * column, sum and post-decrement scratch webs plus one schedule pair, and
 * one edit closed all of it: the outer and word loops are TOP-tested
 * `while (n--)` with the count one higher, not `do { } while (n--)`.  Only
 * the top-tested form numbers the dead loop-exit copy (the old value of the
 * post-decrement) below the counter (L93), and this function's colouring is
 * the caller-saved sweep in ascending web number, so that one web number
 * moves row to $a1, column to $a2, sum to $a3 and the scratch to $v1 -- and
 * the +0x7C/+0x80 constant pair follows.  The inner loop is inert either way;
 * it is written the same way for symmetry.  Both forms run the same
 * iterations (0x3E, 0x3E, 0x400): uopt peels the first test and the loop
 * enters with the decremented count, which is why the target still
 * materialises 0x3D and 0x3FF. */
static void overlay43FilterImage(Overlay43Image *image) {
    u8 *pixel;
    u32 *word;
    s32 row;
    s32 column;
    u16 sum;

    pixel = image->pixels;
    row = 0x3E;
    while (row--) {
        column = 0x3E;
        while (column--) {
            sum = pixel[0];
            sum += pixel[1];
            sum += pixel[2];
            sum += pixel[0x40];
            sum += pixel[0x42];
            sum += pixel[0x80];
            sum += pixel[0x81];
            sum += pixel[0x82];
            pixel[0x41] = sum >> 3;
            pixel++;
        }
        pixel += 2;
    }

    word = (u32 *)image->pixels;
    row = 0x400;
    while (row--) {
        *word = (*word & 0xF0F0F0F0) >> 4;
        word++;
    }
}
