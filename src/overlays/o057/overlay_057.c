#include "overlays/overlay_057.h"

/* Overlay 57's BSS block, defined by the module (it was the mode trigger's).
 * IDO shares one `lui $at` across constant-index stores only for a
 * locally-defined symbol, which the shipped trigger shows. */
/* Global so the promotion proof has a named canonical .bss ownership witness;
 * it sits at offset 0, is never referenced, and every other object in the
 * block stays static so its accesses keep their section-relative addends. */
u8 gOverlay57BssPad000[0x114];

static s32 gOverlay57Countdown;

static s32 gOverlay57Timer;

static s32 gOverlay57BssPad11C;

static s32 gOverlay57BssPad120;

static s32 gOverlay57BssPad124;

static s32 gOverlay57BssPad128;

static s32 gOverlay57BssPad12C;

static s32 gOverlay57BssPad130;

static s32 gOverlay57BssPad134;

static s32 gOverlay57BssPad138;

static s32 gOverlay57SetupStatus;

static s32 gOverlay57SetupDelay;

static s32 gOverlay57ModeFlag;

static s32 gOverlay57BssPad148;

static s32 gOverlay57BssPad14C;

static s32 gOverlay57BssPad150;

static s32 gOverlay57BssPad154;

static s32 gOverlay57BssPad158;

static s32 gOverlay57BssPad15C;

static s32 gOverlay57BssPad160;

static s32 gOverlay57BssPad164;

static s32 gOverlay57SetupValues[6];

static Overlay57ModeObject gOverlay57Object;

static s32 gOverlay57BssPad184;

static s32 gOverlay57TriggerLatched;

static s32 gOverlay57BssPad18C;

static s32 gOverlay57BssPad190;

static s32 gOverlay57ObjectStatus;

static u8 gOverlay57BssPad198[0x20];

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void func_overlay_057_F0001020_18A4C18(s32 updateRate);
static void func_overlay_057_F0001AE8_18A56E0(s32 updateRate);
static void overlay57EaseAndLatch(s32 updateRate);
static void overlay57SmoothAndCheckDistance(s32 smoothingSteps);
/* Called with the s32 update rate; the definition takes it as a one-word struct. */
static void overlay57CheckDistance();
static void overlay57UpdateTransition(s32 updateRate);
static void overlay57ApplyValue(s32 id, s32 value, s32 scaled);
static void overlay57Draw32A0(s32 updateRate);
static void overlay57UpdateSelection(s32 updateRate);
static void overlay57UpdateModeState(s32 updateRate);
static void overlay57BeginMode();
static void overlay57HandleModeInput(s32 updateRate);
static void overlay57StartMode(s32 value);
static void func_overlay_057_F0004460_18A8058(s32 updateRate);
static void overlay57UpdateModeTrigger(s32 updateRate);
static void overlay57InitializeMode(s32 value);
static void func_overlay_057_F0004E18_18A8A10(s32 updateRate);
static void func_overlay_057_F00060F8_18A9CF0(s32 updateRate);
static void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
static void overlay57ApplyTable(void);
static void overlay57UpdateNode(void);

/* Overlay 45 entry points, reached through the relocation surface. */  /* (was on extern overlay45CreateDescriptor_o057Reloc) */
/* Overlay 57 text +0x0..+0x954, the module initializer.
 *
 * Every descriptor loop is a plain indexed `for (i = 0; i < N; i++)`: uopt
 * strength-reduces the subscripts and LFTR supplies the end pointer, so the
 * preheader address order (index table first) and the end/index registers
 * (s1/s2, because `i` itself spans each preheader) follow from the shape.
 * The final loop indexes the pair table as an s16 array by `i << 1`, keeps the
 * state pointer in a local (v1), and stores byte0B after the pair. `pad0` and
 * `pad1` are unreferenced frame cells (L99). */
void func_overlay_057_F0000000_18A3BF8(void) {
    s32 pad0;
    s32 *entry;
    s32 i;
    u8 choiceMask;
    O57SpawnPacket packet;
    O57SpawnState *spawnState;
    O57Thing *object;
    O57FinalSpawnPacket final;
    s32 pad1;

    gO57Current100Reloc = gO57ResidentCurrentReloc;
    gO57Pending104Reloc = 0;
    choiceMask = 0;
    gO57Previous108Reloc = gO57ResidentCurrentReloc;
    gO57Runtime1B8Reloc = func_80028F54();
    fontUseFont(3);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    gO57Descriptor00Reloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x40], 0xA0, -0x28, 4);
    gO57DescriptorF8Reloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x3C], 0xA0, -0x28, 4);
    gO57Descriptor6CReloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x41], 0xA0, 0x104, 4);
    gO57Descriptor70Reloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x42], 0xA0, 0x104, 4);
    gO57Descriptor74Reloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x43], 0xA0, 0x104, 4);
    gO57Descriptor78Reloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x44], 0xA0, 0x104, 4);
    gO57DescriptorFCReloc = overlay45CreateDescriptor_o057Reloc(
        D_8007C0B8[0x6D], 0xA0, -0x20, 4);

    for (i = 0; i < 9; i++) {
        gO57Descriptors08Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            D_8007C0B8[gO57DescriptorIndicesD8Reloc[i]], 0xA0, 0xBE,
            0x204);
        overlay45SetMode_o057Reloc(gO57Descriptors08Reloc[i], 0);
    }

    for (i = 0; i < 9; i++) {
        gO57Descriptors30Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            D_8007C0B8[gO57DescriptorIndicesECReloc[i]], 0xA0, 0xBE,
            0x204);
        overlay45SetMode_o057Reloc(gO57Descriptors30Reloc[i], 0);
    }

    for (i = 0; i < 24; i++) {
        gO57Descriptors80Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            D_8007C0B8[D_8007C11C[i]], 0xA0, 0x104, 4);
        overlay45SetMode_o057Reloc(gO57Descriptors80Reloc[i], 0);
    }

    for (i = 0; i < 6; i++) {
        gO57DescriptorsE0Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            D_8007C0B8[gO57DescriptorIndices130Reloc[i]], 0xA0, 0x104, 4);
        overlay45SetMode_o057Reloc(gO57DescriptorsE0Reloc[i], 0);
    }

    for (i = 0; i < 5; i++) {
        gO57Descriptors58Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            D_8007C0B8[0x36 + i], 0xA0, -0x28, 4);
    }

    overlay45SetMode(gO57Descriptor00PrepareReloc, 0xFF);
    overlay45SetMode(gO57DescriptorF8PrepareReloc, 0xFF);
    overlay45SetMode(gO57Descriptor6CPrepareReloc, 0xFF);
    overlay45SetMode(gO57Descriptor70PrepareReloc, 0xFF);
    overlay45SetMode(gO57Descriptor74PrepareReloc, 0xFF);
    overlay45SetMode(gO57Descriptor78PrepareReloc, 0xFF);
    o57PrepareDescriptorListReloc(&gO57DescriptorList80Reloc);
    o57PrepareDescriptorListReloc(&gO57DescriptorList130Reloc);

    gO57Value1FCReloc = ((O57SeedData *)D_800D31C8)->value0C;
    gO57Value21CReloc = ((O57SeedData *)D_800D31C8)->value10;
    gO57Value23CReloc = ((O57SeedData *)D_800D31C8)->value14;
    gO57Value25CReloc = ((O57SeedData *)D_800D31C8)->value10;
    gO57Value32CReloc = ((O57SeedData *)D_800D31C8)->value1EC;
    gO57Value34CReloc = ((O57SeedData *)D_800D31C8)->value1F4;
    for (i = 0; i < 10; i++) {
        gO57Rows27CReloc[i].value = ((O57SeedData *)D_800D31C8)->value1C;
    }

    gO57Mode11CReloc = 0;
    gO57Value128Reloc = -0x50;
    gO57Value12CReloc = 0;
    D_134 = 0x40000;
    gO57Value138Reloc = 0x41800;
    gO57Value14CReloc = 0;
    gO57Value160Reloc = 0;
    gO57Value164Reloc = 0;
    gO57Value50CReloc = 0;
    gO57Value148Reloc = 0;
    gO57Value13CReloc = 0;

    animseqStartPath(0x3C);

    object = func_800508B4(0x3C);
    if (object->object != 0) {
        packet.mode = 0x14;
        packet.flags = 0;
        packet.x = object->object->x;
        packet.y = object->object->y;
        packet.z = object->object->z;
        packet.angle = object->object->angle;
        packet.scale = object->object->scale;
        packet.kind = 0x35;
        packet.state = 0;
        object = func_8000590C(&packet, 1);
        if (object != 0) {
            object->field3C = 0;
        }
        packet.kind = 0x38;
        packet.state = 1;
        object = func_8000590C(&packet, 1);
        if (object != 0) {
            object->field3C = 0;
        }
    }

    initColourCycle(&gO57ColourCycle1A8Reloc, 0xA);
    gO57Active144Reloc = 0;
    joyResetMap();

    switch (frontGetMode()) {
    case 4:
        gO57State118Reloc = 0xB;
        gO57ModeObject180Reloc.value = 0x2E;
        animseqStartPath(((u8 *)&gO57ModeObject180Reloc)[3]);
        object = func_800508B4(((u8 *)&gO57ModeObject180Reloc)[3]);
        if (object != 0) {
            object->flags |= 2;
        }
        gO57SpecialModeReloc = 0;
        D_800D31B0 = 1;
        break;
    case 10:
        gO57State118Reloc = 7;
        entry = gO57Ids134Reloc;
        while (*entry != -1) {
            animseqStartPath(*entry);
            overlay57SetNodeValue(*entry, gO57NodeValues17CReloc[*entry], 0.007f);
            entry++;
        }
        break;
    case 12:
        gO57State118Reloc = 1;
        entry = gO57Ids134Reloc;
        while (*entry != -1) {
            animseqStartPath(*entry);
            overlay57SetNodeValue(*entry, gO57NodeValues17CReloc[*entry], 0.007f);
            entry++;
        }
        gO57Mode11CReloc = 1;
        break;
    case 17:
        gO57State118Reloc = 0xA;
        gO57ModeObject180Reloc.value = 0x50;
        animseqStartPath(((u8 *)&gO57ModeObject180Reloc)[3]);
        object = func_800508B4(((u8 *)&gO57ModeObject180Reloc)[3]);
        if (object != 0) {
            object->flags |= 2;
        }
        gO57SpecialModeReloc = 5;
        break;
    case 18:
        gO57State118Reloc = 0x14;
        gO57ModeObject180Reloc.value = 0x54;
        animseqStartPath(((u8 *)&gO57ModeObject180Reloc)[3]);
        object = func_800508B4(((u8 *)&gO57ModeObject180Reloc)[3]);
        if (object != 0) {
            object->flags |= 2;
        }
        if (D_8007C1A0 == 1) {
            gO57SpecialModeReloc = 7;
        } else {
            gO57SpecialModeReloc = 6;
        }
        break;
    }

    for (i = 0; i < 4; i++) {
        if (gO57ChoicesReloc[i].enabled != 0) {
            choiceMask |= 1 << i;
        }
    }
    if ((choiceMask != gO57ChoiceMaskReloc) ||
        (gO57ChoiceMaskReferenceReloc != gO57ChoiceSourceReloc)) {
        func_8003A754();
        gO57ChoiceDirtyReloc = 0;
        gO57ChoiceMaskReloc = choiceMask;
        gO57ChoicePublishedReloc = 1;
        gO57ChoiceMaskReferenceReloc = gO57ChoiceSourceReloc;
    }

    gO57Value188Reloc = 0;
    gO57Value18CReloc = ((gO57ResidentFlags10Reloc & 0x1C0) >> 6) >= 3;
    if (gO57Value18CReloc != 0) {
        gO57Value190Reloc = 3;
    } else {
        gO57Value190Reloc = 2;
    }
    func_80001708();
    gO57Value198Reloc = 0;
    gO57Value19CReloc = 0;
    gO57Value110Reloc = -140.0f;
    gO57Value124Reloc = 0;

    for (i = 0; i < 4; i++) {
        final.kind = 0x138;
        final.mode = 0xE;
        final.x = ((s16 *)gO57SpawnPairs3E8Reloc)[i << 1];
        final.y = ((s16 *)gO57SpawnPairs3E8Reloc)[(i << 1) + 1];
        final.z = 0;
        final.state = 0;
        final.byte0B = 0x80;
        final.byte0A = 0;
        gO57Spawned150Reloc[i] = func_8000590C(&final, 0);
        spawnState = *gO57Spawned150Reloc[i]->state;
        spawnState->mode = 2;
        func_8005AD64(gO57Spawned150Reloc[i], 0, 0, 0.0f);
    }
}

/* Typed semantic baseline from preserved Epoch11 scaffold for overlay 57 +0x0954..+0x1020. */  /* (was on extern O57_D_1A8) */
#define O57_APPLY_FIXED_LIST_0954() \
    do { \
        overlay57ApplyValue(0, 10, 1); \
        overlay57ApplyValue(2, 9, 1); \
        overlay57ApplyValue(3, 10, 1); \
        overlay57ApplyValue(6, 4, 1); \
        overlay57ApplyValue(19, 4, 1); \
        overlay57ApplyValue(23, 0, 1); \
        overlay57ApplyValue(28, 3, 1); \
        overlay57ApplyValue(4, 5, 1); \
        overlay57ApplyValue(7, 9, 1); \
        overlay57ApplyValue(13, 0, 1); \
        overlay57ApplyValue(23, 3, 1); \
        overlay57ApplyValue(25, 2, 1); \
        overlay57ApplyValue(29, 0, 1); \
    } while (0)

void func_overlay_057_F0000954_18A454C(s32 updateRate) {
    s8 *cursor;
    s32 fixed;

    updateColourCycle(&O57_D_1A8, updateRate);
    func_overlay_057_F0001AE8_18A56E0(updateRate);

    if (O57_D_4CC == 0) {
        s32 query;
        O57Lookup0954 *lookup;

        overlay84SelectCurrent(O57_queryObject0954);
        query = frontGetMode();

        switch (query) {
        case 10:
            overlay84ActivateCurrent(0);
            break;
        case 12:
            overlay84InitializeCurrent();
            break;
        case 17: {
            u8 *entry;

            O57_D_194 = 0;
            O57_D_118 = 10;
            O57_D_164 = 0;
            O57_D_160 = 0;
            O57_D_184 = 47;
            O57_D_114 = 180;
            overlay84ClearActive();
            entry = func_800508B4(47);
            if ((entry != 0) && (*(void **)(entry + 8) != 0)) {
                func_8005AD64(*(void **)(entry + 8), 0, -1, 1.0f);
            }
            O57_D_188 = 1;
            O57_D_13C = 255;
            O57_D_140 = 0;
            break;
        }
        case 18:
            O57_D_118 = 20;
            O57_D_164 = 0;
            O57_D_160 = 0;
            O57_D_184 = 75;
            O57_D_114 = 180;
            overlay84ClearActive();
            O57_D_13C = 255;
            O57_D_140 = 0;
            break;
        }

        /* Inert allocation aid retained by the exact C; tracked in
         * docs/cleanup-queue.md. */
        if (O57_config0954.field08) {}

        if (((((O57_config0954.field08 & 0xFFFF) & 0x1C0) >> 6) >= 3) &&
            (((O57_config0954.field0A & 0x1C0) >> 6) >= 3) &&
            (((O57_config0954.field0C & 0x1C0) >> 6) >= 3)) {
            O57_D_198 = 1;
            if (D_8007C1A0 == 1) {
                overlay57ApplyValue(8, 0, 1);
                overlay57ApplyValue(31, 0, 1);
            } else {
                overlay57ApplyValue(8, 0, 2);
                overlay57ApplyValue(31, 0, 2);
            }
        } else {
            overlay84SetBit(3);
        }

        if (((O57_nibbleFlags0954 << 5) >> 28) == 15) {
            O57_D_19C = 1;
            if (D_8007C1A0 == 1) {
                overlay57ApplyValue(3, 11, 1);
                overlay57ApplyValue(22, 3, 1);
            } else {
                overlay57ApplyValue(3, 11, 2);
                overlay57ApplyValue(22, 3, 2);
            }
        } else {
            overlay84SetBit(4);
        }

        if (D_8007C1A0 != 1) {
            O57_APPLY_FIXED_LIST_0954();
            overlay84SetBit(7);
        }

        cursor = O57_labels390;
        do {
            overlay57ApplyValue(47, *cursor, 1);
            cursor++;
        } while (cursor < &O57_labels39CEnd);
        cursor = O57_labels39CStart;
        do {
            overlay57ApplyValue(47, *cursor, O57_D_198);
            cursor++;
        } while (cursor < &O57_labels3A0End);
        cursor = O57_labels3A0Start;
        do {
            overlay57ApplyValue(47, *cursor, O57_D_19C);
            cursor++;
        } while (cursor < &O57_labels3A4);
        overlay57ApplyValue(47, 30, (O57_node30Flags0954 << 13) >> 31);

        lookup = O57_call_0DA4(75);
        if (lookup != 0) {
            O57Owner0954 *owner = lookup->level08->level68->owner00;

            if (owner != 0) {
                O57Entry0954 *entries = owner->entries4C;

                if (entries != 0) {
                    u32 count = owner->metadata00->count2C;
                    s32 oldCount;

                    oldCount = count;
                    count--;
                    if (oldCount != 0) {
entry_loop:
                            if ((entries->flags04 & 0x00100000) != 0) {
                                entries->value00 = 0x0100;
                            }
                            entries++;
                        oldCount = count;
                        count--;
                        if (oldCount != 0) {
                            goto entry_loop;
                        }
                    }
                }
            }
        }
        O57_D_4CC = 1;
    }

    {
        O57_D_104 += updateRate << 4;
        if (O57_D_104 >= 256) {
            O57_D_104 = 255;
        }
    }

    func_8002FB34(&D_800D3140, O57_renderData21C,
                  0.0f, 0.0f, 1.0f, 1.0f, -2, 3);
    func_8002FB34(&D_800D3140, O57_renderData23C,
                  0.0f, 0.0f, 1.0f, 1.0f, -2, 3);
    func_8002FB34(&D_800D3140, &O57_renderChannel1FC,
                  (f32)O57_D_128, 0.0f, 1.0f, 1.0f, -2, 3);
    func_8002FB34(&D_800D3140, &O57_renderChannel1FC,
                  (f32)(320 - O57_D_128), 0.0f,
                  1.0f, 1.0f, -2, 0x1003);

    texAnimateTexSprite(O57_renderChannel1FC.object00, O57_channelData4D0, 12,
                  &O57_channelValue4DC, updateRate);
    O57_renderChannel1FC.fixed08 = (s32)(O57_channelValue4DC * 65536.0f);
    texAnimateTexSprite(O57_renderChannel1FC.object00, O57_channelData4D8, 2,
                  &O57_channelValue4E0, updateRate);

    fixed = (s32)(O57_channelValue4E0 * 65536.0f);
    O57_D_224 = fixed;
    O57_D_244 = fixed;
    O57_D_264 = fixed;
}

/* Resident display-list cursor, camera context, vertex cursor and resource
 * table (renamed onto their surface entries by POSTPROCESS). */  /* (was on extern D_800D3140) */
#define O57_SP_VERTEX(packet, vertex, count, first)                       \
    gDma1p(packet, G_VTX, vertex,                                         \
           (((count) << 3) + ((count) << 1)) + 8,                         \
           ((count) << 3) | (((u32) (vertex)) & 6) | (first))

#define O57_SP_POLYGON(packet, address, count, textured)                 \
    {                                                                      \
        Gfx *_g = (Gfx *) (packet);                                        \
        _g->words.w0 = _SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
                       _SHIFTL(5, 24, 8) | _SHIFTL((count) << 4, 0, 16);  \
        _g->words.w1 = (u32) (address);                                    \
    }

#define O57_PHYSICAL(p) ((u32)((u8 *)(p) + 0x80000000))

#define O57_GRID_VERTEX(px, py, pz)               \
    D_800D3148->x = (px);          \
    D_800D3148->y = (py);          \
    D_800D3148->z = (pz);          \
    D_800D3148++

/* Overlay 57 text +0x1020..+0x1978, the grid quad draw. Matched 2026-10-02
 * (lane o-ovl7) from 585 masked words (size -32): the record is read as
 * points of the model at each use with the edge differences as declared
 * locals (dz is the one the target spills to its home), the midpoints and
 * the three coordinate arrays declared in the target's frame order, the
 * colour fill walks a vertex pointer beside its count, the display-list
 * commands are one packet macro each (the track.c vertex and polygon forms)
 * with the triangle addresses as constant physical offsets, and the camera
 * context is passed by address. */
static void func_overlay_057_F0001020_18A4C18(s32 updateRate) {
    O57GridObject *object;
    f32 x01;
    f32 x02;
    f32 x12;
    f32 x13;
    f32 x23;
    f32 y01;
    f32 y02;
    f32 y12;
    f32 y13;
    f32 y23;
    f32 z01;
    f32 z02;
    f32 z12;
    f32 z13;
    f32 z23;
    f32 x[4];
    f32 y[4];
    f32 z[4];
    f32 dx;
    f32 dy;
    f32 dz;
    f32 blend;
    s32 i;
    O57GridVertex *vertex;
    s32 index;
    s32 renderMode;
    O57GridModel *model;

    model = NULL;
    camSetView(&D_800D3140, &D_800D3144);
    texAnimateTexSprite(D_800D31C8[62], D_4D4, 2, &D_4E4, updateRate);

    if (gO57GridMode184 == 0x2F || gO57GridMode184 == 0x4B) {
        object = func_800508B4(gO57GridMode184);
        if (object != NULL && object->owner != NULL) {
            model = *object->owner->model;
            index = gO57GridRecord160 * 4;
            blend = 0.15f;
            renderMode = 4;
        }
        if (gO57GridMode184 == 0x4B) {
            func_8002109C(object->owner);
            gO57GridAlpha13C = 0xFF;
        }
    } else if (gO57GridMode184 == 0x4C) {
        object = func_800508B4(0x4C);
        if (object != NULL && object->owner != NULL) {
            model = *object->owner->model;
            if (gOverlay57TableIndex != 0) {
                index = 4;
            } else {
                index = 0;
            }
            renderMode = 6;
            blend = 0.1f;
        }
        gO57GridAlpha13C = 0xFF;
    }

    if (model != NULL) {
        gDPSetPrimColor(D_800D3140++, 0, 0, 0xFF, 0xFF, 0xFF, gO57GridAlpha13C);
        O57_SP_VERTEX(D_800D3140++, (u32)D_800D3148 + 0x80000000, 9, 0);
        texDPTextureX(&D_800D3140, D_800D31C8[62], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x00]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[63], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x20]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[64], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x40]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[65], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x60]), 2, 1);

        dx = model->points[index].x - model->points[index + 3].x;
        dy = model->points[index].y - model->points[index + 3].y;
        dz = model->points[index].z - model->points[index + 3].z;
        x[0] = model->points[index].x + dx * blend;
        y[0] = model->points[index].y + dy * blend;
        z[0] = model->points[index].z + dz * blend;
        x[3] = model->points[index + 3].x - dx * blend;
        y[3] = model->points[index + 3].y - dy * blend;
        z[3] = model->points[index + 3].z - dz * blend;

        dx = model->points[index + 1].x - model->points[index + 2].x;
        dy = model->points[index + 1].y - model->points[index + 2].y;
        dz = model->points[index + 1].z - model->points[index + 2].z;
        x[1] = model->points[index + 1].x + dx * blend;
        y[1] = model->points[index + 1].y + dy * blend;
        z[1] = model->points[index + 1].z + dz * blend;
        x[2] = model->points[index + 2].x - dx * blend;
        y[2] = model->points[index + 2].y - dy * blend;
        z[2] = model->points[index + 2].z - dz * blend;

        x01 = x[0] + (x[1] - x[0]) * 0.5f;
        x02 = x[0] + (x[2] - x[0]) * 0.5f;
        x12 = x[1] + (x[2] - x[1]) * 0.5f;
        x13 = x[1] + (x[3] - x[1]) * 0.5f;
        x23 = x[2] + (x[3] - x[2]) * 0.5f;
        y01 = y[0] + (y[1] - y[0]) * 0.5f;
        y02 = y[0] + (y[2] - y[0]) * 0.5f;
        y12 = y[1] + (y[2] - y[1]) * 0.5f;
        y13 = y[1] + (y[3] - y[1]) * 0.5f;
        y23 = y[2] + (y[3] - y[2]) * 0.5f;
        z01 = z[0] + (z[1] - z[0]) * 0.5f;
        z02 = z[0] + (z[2] - z[0]) * 0.5f;
        z12 = z[1] + (z[2] - z[1]) * 0.5f;
        z13 = z[1] + (z[3] - z[1]) * 0.5f;
        z23 = z[2] + (z[3] - z[2]) * 0.5f;

        vertex = D_800D3148;
        for (i = 0; i < 9; i++) {
            vertex->r = 0xFF;
            vertex->g = 0xFF;
            vertex->b = 0xFF;
            vertex->a = 0xFF;
            vertex++;
        }

        O57_GRID_VERTEX(x[0], y[0], z[0]);
        O57_GRID_VERTEX(x01, y01, z01);
        O57_GRID_VERTEX(x[1], y[1], z[1]);
        O57_GRID_VERTEX(x02, y02, z02);
        O57_GRID_VERTEX(x12, y12, z12);
        O57_GRID_VERTEX(x13, y13, z13);
        O57_GRID_VERTEX(x[2], y[2], z[2]);
        O57_GRID_VERTEX(x23, y23, z23);
        O57_GRID_VERTEX(x[3], y[3], z[3]);
    }
}

void overlay57ReleaseAll(void) {
    void **cursor;
    void **end;

    cursor = gOverlay57Range150Start, end = gOverlay57Range150End;
    do {
        func_80006EA0(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    overlay45ReleaseDescriptor(gOverlay57Resource06C);
    overlay45ReleaseDescriptor(gOverlay57Resource074);
    overlay45ReleaseDescriptor(gOverlay57Resource070);
    overlay45ReleaseDescriptor(gOverlay57Resource078);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource000);
    overlay45ReleaseDescriptor(gOverlay57Resource0F8);
    overlay45ReleaseDescriptor(gOverlay57Resource0FC);

    cursor = gOverlay57Range058Start, end = gOverlay57Range058End;
    do {
        overlay45ReleaseDescriptor(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range008Start, end = gOverlay57Range008End;
    do {
        overlay45ReleaseDescriptor(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range030Start, end = gOverlay57Range030End;
    do {
        overlay45ReleaseDescriptor(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range080Start, end = gOverlay57Range080End;
    do {
        overlay45ReleaseDescriptor(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range0E0Start, end = gOverlay57Range0E0End;
    do {
        overlay45ReleaseDescriptor(*cursor);
        cursor++;
    } while (cursor != end);

    freeFrontEndList(gOverlay57Object100);
}

/* Overlay 57 bss. Every name below is the one already adopted by a matched
 * overlay 57 translation unit for the same runtime relocation identity; the
 * three that are new here (+0x100, +0x108, +0x10C) are the selection indices
 * this function shares with the +0x104 mode value. */  /* (was on extern gOverlay57Resource000) */
/* Overlay 57 initialized data. */  /* (was on extern gOverlay57UpdateSelection) */
/* Runtime-linked globals shared with the resident module. */  /* (was on extern D_8007C1A0) */
/* Runtime-linked calls: overlay 45 +0x314 and +0x1BE0, and the resident
 * trackSetFog, amSndStop and amSndPlay. Each is spelled as an overlay
 * placeholder so the stored `jal 0` addend is preserved. */  /* (was on extern overlay45ConfigureLayout) */
/* Overlay 57 text +0x1AE8..+0x28B4, the per-frame mode dispatch and layout
 * pass. Matched 2026-10-01 (lane d-o057) from 614 masked words at size delta
 * -132 by rewriting inherited shape, not by allocator work:
 *
 *  - the descriptor walks are indexed loops over each range (`count < 9`,
 *    `count != 6` as shipped); uopt creates the walking pointers itself, so
 *    the shared counter no longer outranks the +0xE0 base for s0 and is
 *    split into a2 with the target's 0x34 spill cell;
 *  - the fog easing reads each value by name and stores through a pointer
 *    to it, so the address constants carry the loads in t0/t1, every store
 *    is a direct symbol store, and nothing is forwarded between the four
 *    unrolled copies (this was the whole -132);
 *  - the `(s32)` on the first operand orders the shipped `addu`.
 *
 * The dispatch is a real compiler jump table; its retained copy and the
 * two text relocations are bound in mk/overlays.mk. */
static void func_overlay_057_F0001AE8_18A56E0(s32 updateRate) {
    s32 count;
    s32 *near;
    s32 *far;

    D_130 = 0;
    switch (gOverlay57Timer) {
        case 11:
            overlay57EaseAndLatch(updateRate);
            break;
        case 14:
            overlay57SmoothAndCheckDistance(updateRate);
            break;
        case 5:
            overlay57CheckDistance(updateRate);
            break;
        case 7:
            overlay57UpdateTransition(updateRate);
            break;
        case 1:
            overlay57UpdateSelection(updateRate);
            break;
        case 4:
            overlay57UpdateModeState(updateRate);
            break;
        case 12:
            overlay57BeginMode(updateRate);
            break;
        case 13:
            overlay57HandleModeInput(updateRate);
            break;
        case 15:
            overlay57StartMode(updateRate);
            break;
        case 16:
            func_overlay_057_F0004460_18A8058(updateRate);
            break;
        case 9:
            overlay57UpdateModeTrigger(updateRate);
            break;
        case 21:
            overlay57InitializeMode(updateRate);
            break;
        case 10:
            func_overlay_057_F0004E18_18A8A10(updateRate);
            break;
        case 20:
            func_overlay_057_F00060F8_18A9CF0(updateRate);
            break;
    }

    overlay57Draw32A0(updateRate);

    if (gOverlay57State == 1) {
        overlay45ConfigureLayout(gOverlay57Resource000, 0xA0, 0x20, 0x104);
        if (D_8007C1A0 == 1) {
            overlay45ConfigureLayout(
                gOverlay57Range008Start[gOverlay57Selection100], 0xA0, 0xBE, 0x104);
        } else {
            overlay45ConfigureLayout(
                gOverlay57Range030Start[gOverlay57Selection100], 0xA0, 0xBE, 0x104);
        }

        for (count = 0; count < 9; count++) {
            overlay45SetMode(gOverlay57Range008Start[count], 0);
        }
        for (count = 0; count < 9; count++) {
            overlay45SetMode(gOverlay57Range030Start[count], 0);
        }

        if (D_8007C1A0 == 1) {
            overlay45SetMode(
                gOverlay57Range008Start[gOverlay57Selection108], gOverlay57Mode10C);
            overlay45SetMode(
                gOverlay57Range008Start[gOverlay57Selection100], O57_D_104);
        } else {
            overlay45SetMode(
                gOverlay57Range030Start[gOverlay57Selection108], gOverlay57Mode10C);
            overlay45SetMode(
                gOverlay57Range030Start[gOverlay57Selection100], O57_D_104);
        }

        for (count = 0; count < 24; count++) {
            overlay45SetMode(gOverlay57Range080Start[count], 0);
        }
        for (count = 0; count != 6; count++) {
            overlay45SetMode(gOverlay57Range0E0Start[count], 0);
        }
    } else {
        overlay45ConfigureLayout(gOverlay57Resource000, 0xA0, -0x1E, 0x104);
        if (D_8007C1A0 == 1) {
            overlay45ConfigureLayout(
                gOverlay57Range008Start[gOverlay57Selection100], 0xA0, 0x104, 0x104);
        } else {
            overlay45ConfigureLayout(
                gOverlay57Range030Start[gOverlay57Selection100], 0xA0, 0x104, 0x104);
        }
    }

    if (gOverlay57State == 2) {
        if (gOverlay57Timer == 10) {
            if (O57_D_194 == 1) {
                overlay45ConfigureLayout(
                    gOverlay57Range080Start[O57_D_160], 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource0FC, 0xA0, -0x20, 0x104);
            } else {
                overlay45ConfigureLayout(
                    gOverlay57Range080Start[O57_D_160], 0xA0, 0xBE, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource0FC, 0xA0, 0x20, 0x104);
            }

            for (count = 0; count < 0x18; count++) {
                if ((count != gOverlay57Selection108) &&
                    (count != gOverlay57Selection100)) {
                    overlay45SetMode(gOverlay57Range080Start[count], 0);
                }
            }
            overlay45SetMode(
                gOverlay57Range080Start[O57_D_164], gOverlay57Mode10C);
            overlay45SetMode(
                gOverlay57Range080Start[O57_D_160], O57_D_104);
        }

        for (count = 0; count != 9; count++) {
            overlay45SetMode(gOverlay57Range008Start[count], 0);
        }
    } else {
        if (O57_D_160 < 0x18) {
            overlay45ConfigureLayout(
                gOverlay57Range080Start[O57_D_160], 0xA0, 0x104, 0x104);
        }
        overlay45ConfigureLayout(gOverlay57Resource0FC, 0xA0, -0x20, 0x104);
    }

    if (gOverlay57State == 6) {
        overlay45ConfigureLayout(gOverlay57Range0E0Start[O57_D_160], 0xA0, 0xBE, 0x104);
        overlay45ConfigureLayout(gOverlay57Resource0FC, 0xA0, 0x20, 0x104);
        for (count = 0; count < 6; count++) {
            if ((count != gOverlay57Selection108) &&
                (count != gOverlay57Selection100)) {
                overlay45SetMode(gOverlay57Range0E0Start[count], 0);
            }
        }
        overlay45SetMode(gOverlay57Range0E0Start[O57_D_164], gOverlay57Mode10C);
        overlay45SetMode(gOverlay57Range0E0Start[O57_D_160], O57_D_104);

        for (count = 0; count != 9; count++) {
            overlay45SetMode(gOverlay57Range008Start[count], 0);
        }
    } else if (O57_D_160 < 6) {
        overlay45ConfigureLayout(gOverlay57Range0E0Start[O57_D_160], 0xA0, 0x104, 0x104);
    }

    if (gOverlay57State == 3) {
        overlay45ConfigureLayout(gOverlay57Resource0F8, 0xA0, 0x1E, 0x104);
    } else {
        overlay45ConfigureLayout(gOverlay57Resource0F8, 0xA0, -0x1E, 0x104);
    }

    if (gOverlay57State == 5) {
        switch (gOverlay57TableIndex) {
            case 0:
                overlay45ConfigureLayout(gOverlay57Resource06C, 0xA0, 0xBE, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 1:
                overlay45ConfigureLayout(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource070, 0xA0, 0xBE, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 2:
                overlay45ConfigureLayout(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource074, 0xA0, 0xBE, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource078, 0xA0, 0x104, 0x104);
                break;
            case 3:
                overlay45ConfigureLayout(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource070, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource074, 0xA0, 0x104, 0x104);
                overlay45ConfigureLayout(gOverlay57Resource078, 0xA0, 0xBE, 0x104);
                break;
        }
        overlay45ConfigureLayout(
            gOverlay57Range058Start[gOverlay57UpdateSelection], 0xA0, 0x20, 0x104);
    } else {
        overlay45ConfigureLayout(
            gOverlay57Range058Start[gOverlay57UpdateSelection], 0xA0, -0x28, 0x104);
        overlay45ConfigureLayout(gOverlay57Resource074, 0xA0, 0x104, 0x104);
        overlay45ConfigureLayout(gOverlay57Resource06C, 0xA0, 0x104, 0x104);
        overlay45ConfigureLayout(gOverlay57Resource070, 0xA0, 0x104, 0x104);
        overlay45ConfigureLayout(gOverlay57Resource078, 0xA0, 0x104, 0x104);
    }

    D_500 += D_504 * updateRate;
    if (D_500 < 0) {
        D_500 = -D_500;
        D_504 = -D_504;
    } else if (D_500 >= 0x100) {
        D_500 = 0x1FE - D_500;
        D_504 = -D_504;
    }

    if ((gOverlay57State != 0) && (gOverlay57State != 3) &&
        (gOverlay57State != 4) && (gOverlay57State != 5) &&
        ((gOverlay57State != 2) || (O57_D_194 != 1))) {
        O57_D_128 += updateRate * 4;
        if (O57_D_128 > 0) {
            O57_D_128 = 0;
        }
    } else {
        O57_D_128 -= updateRate * 4;
        if (O57_D_128 < -0x50) {
            O57_D_128 = -0x50;
        }
    }

    near = &D_134;
    far = &D_138;
    if (gOverlay57ModeFlag != 0) {
        for (count = 0; count < updateRate; count++) {
            *near = (s32)D_134 + ((0x3DE00 - D_134) >> 4);
            *far = (s32)D_138 + ((0x3E800 - D_138) >> 4);
        }
    } else {
        for (count = 0; count < updateRate; count++) {
            *near = (s32)D_134 + ((0x3FA00 - D_134) >> 6);
            *far = (s32)D_138 + ((0x3FF00 - D_138) >> 6);
        }
    }

    trackSetFog(0, D_134 >> 8, D_138 >> 8, D_134 >> 8, 0, 0, 0, 0);
    if (D_130 != 0) {
        if (D_148 != NULL) {
            amSndStop(D_148);
        }
        amSndPlay(0xF, &D_148);
    }
}

/* Overlay 57 text +0x28B4..+0x2C28. Exact: 221 words, frame 0x28, 36/36
 * relocation identities.
 *
 * The plateau here was an s32 local per halfword field. Each one is its own
 * uopt web, and the two of them moved the record pointer's colour off v0 and
 * rotated the block-local temp ring behind it -- 86 of the 92 masked words
 * were that one substitution and its knock-on. Reading and writing the s16
 * fields in place removes the webs entirely; the horizontal statement is
 * written before the vertical one because the pair's load order follows it. */
static void overlay57EaseAndLatch(s32 updateRate) {
    O57Ease28B4Primary *primary;
    O57Ease28B4Secondary *secondary;
    O57Ease28B4Transform *target;
    u32 flags;
    s32 i;

    gO57Ease28B4State144 = 0;

    i = 0;
    if (updateRate > 0) {
        do {
            i++;
            gO57Ease28B4Record21C.x = (s16)(gO57Ease28B4Record21C.x + ((0x104 - gO57Ease28B4Record21C.x) >> 3));
            gO57Ease28B4Record21C.y = (s16)(gO57Ease28B4Record21C.y + ((0xBE - gO57Ease28B4Record21C.y) >> 3));
        } while (i != updateRate);
    }

    i = 0;
    if (updateRate > 0) {
        do {
            i++;
            gO57Ease28B4Record23C.x = (s16)(gO57Ease28B4Record23C.x + ((0x32 - gO57Ease28B4Record23C.x) >> 3));
            gO57Ease28B4Record23C.y = (s16)(gO57Ease28B4Record23C.y + ((0xBE - gO57Ease28B4Record23C.y) >> 3));
        } while (i != updateRate);
    }

    flags = D_800D31B8;
    if (((flags & 0x9000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        primary = func_800508B4(gO57Ease28B4Id183);
        if (primary == NULL) {
            return;
        }
        amSndPlay(0xC, 0);
        (void)animseqStopPath(gO57Ease28B4Id183);
        secondary = o57Ease28B4LookupSecondaryReloc(0x29);
        if (secondary != NULL) {
            target = secondary->target;
            target->x = primary->source->x;
            target->y = primary->source->y;
            target->z = primary->source->z;
            target->angleX = primary->source->angleX;
            target->angleY = primary->source->angleY;
            target->angleZ = primary->source->angleZ;
            func_8005055C(0x29);
            animseqStartPath(0x29);
            gO57Ease28B4State180 = 0x29;
            secondary->flags |= 2;
        }
        gO57Ease28B4Latch50C = 1;
        gO57Ease28B4State118 = 14;
        return;
    }

    if (((flags & 0x4000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        amSndPlay(0xD, 0);
        mainTitlePageInit(1);
        gO57Ease28B4Latch50C = 1;
    }
}

/* Overlay 57 text +0x2C28..+0x2F48. Exact: 200 words, frame 0x30, 24/24
 * relocation identities.
 *
 * Same two smoothing loops as overlay57EaseAndLatch and the same fix: no s32
 * locals, s16 fields read and written in place, horizontal before vertical. */
static void overlay57SmoothAndCheckDistance(s32 smoothingSteps) {
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    O57Tail2C28Position *position;
    O57Tail2C28Position *reference;

    i = 0;
    if (smoothingSteps > 0) {
        do {
            i++;
            gO57Tail2C28Record21C.x = (s16)(gO57Tail2C28Record21C.x + ((0x17C - gO57Tail2C28Record21C.x) >> 3));
            gO57Tail2C28Record21C.y = (s16)(gO57Tail2C28Record21C.y + ((0xBE - gO57Tail2C28Record21C.y) >> 3));
        } while (i != smoothingSteps);
    }

    i = 0;
    if (smoothingSteps > 0) {
        do {
            i++;
            gO57Tail2C28Record23C.x = (s16)(gO57Tail2C28Record23C.x + ((-0x46 - gO57Tail2C28Record23C.x) >> 3));
            gO57Tail2C28Record23C.y = (s16)(gO57Tail2C28Record23C.y + ((0xBE - gO57Tail2C28Record23C.y) >> 3));
        } while (i != smoothingSteps);
    }

    camSetNo(0);
    position = camGetPtr();
    reference = gO57Tail2C28Reference14C;
    if (reference != NULL) {
        dx = reference->x - position->x;
        dy = reference->y - position->y;
        dz = reference->z - position->z;
        if (((dx * dx) + (dy * dy)) + (dz * dz) <
            gO57Tail2C28DistanceLimit104) {
            func_800498FC(4, 0x3EAE147B, 0xBF800000,
                                 0, 0, 0, 0);
        }
    }

    if (overlay41IsUnitScale(gO57Tail2C28QueryWord180) != NULL) {
        gO57Tail2C28Timer118 = 0x11;
        mainChangeLevel(0x1D, 0, 0, 9, 1, 0);
        gO57Tail2C28Completion50C = 1;
        overlay84Mark();
    }
}

/* DKR v77/v80 and JFG contain no donor for this distance/state transition. */
static void overlay57CheckDistance(Overlay57ModeValue value) {
    Overlay57Position *position;
    Overlay57Position *reference;
    f32 dx;
    f32 dy;
    f32 dz;

    gOverlay57ModeFlag = 0;
    camSetNo(0);
    position = camGetPtr();
    reference = gOverlay57Reference;
    if (reference != NULL) {
        dx = reference->x - position->x;
        dy = reference->y - position->y;
        dz = reference->z - position->z;
        if (dx * dx + dy * dy + dz * dz < gOverlay57DistanceLimit) {
            func_800498FC(4, 0x3EAE147B, 0xBF800000, 0, 0, 0, 0);
        }
    }
    if (overlay84GetActive() == 0) {
        gOverlay57Timer = 0x11;
        mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    }
}

/*
 * Overlay 57 +0x3048. Pinned DKR v77/v80 and JFG object/source checks found
 * no exact donor or semantic match for this three-field easing transition.
 */
static void overlay57UpdateTransition(s32 updateRate) {
    s32 i;

    gOverlay57ModeFlag = 1;
    for (i = 0; i < updateRate; i++) {
        gOverlay57Transition.horizontal +=
            (0x17C - gOverlay57Transition.horizontal) >> 2;
        gOverlay57Transition.vertical +=
            (0xBE - gOverlay57Transition.vertical) >> 2;
        gOverlay57Delay += (0xA00 - gOverlay57Delay) >> 3;
    }
    if (overlay84GetActive() == 2) {
        gOverlay57Timer = 1;
        gOverlay57State = 1;
    }
    func_overlay_057_F0001020_18A4C18(updateRate);
    overlay57UpdateNode();
}

/* Pinned DKR v77/v80 and JFG checks found no matching donor. */
static void overlay57ApplyValue(s32 id, s32 value, s32 scaled) {
    Overlay57Entry *entry;
    Overlay57Child *child;

    entry = func_800508B4((u8) id);
    if (entry != NULL && entry->child != NULL) {
        child = entry->child;
        modSetTextureFrame(child->items[child->index], value, scaled << 8);
    }
}

/* Overlay 57 text +0x32A0..+0x35E0. Exact: 208 words, frame 0x70, 53/53
 * relocation identities.
 *
 * Both envelope arms are compound assignments through the global rather than
 * a local assigned and then stored. Written the other way, uopt materialises
 * the value a second time -- the saturating arm hoists a duplicate `li 254`
 * into the guard's delay slot, and the decay arm keeps a separate carrier --
 * and each duplicate rotates the whole block-local temp ring behind it.
 *
 * The mask and shift tables are indexed by the loop counter. Declared as
 * pointer cursors they reserve two frame cells the target does not have,
 * which is the 8 bytes by which the frame used to exceed 0x70. */
static void overlay57Draw32A0(s32 updateRate) {
    Overlay57Draw32A0Record *records;
    Overlay57Draw32A0Record *cursor;
    s32 envelope;
    s32 rowCount[1];
    s32 position;
    s32 i;
    s32 rising;

    if (gO57Draw32A0Available4CC == 0) {
        return;
    }

    rising = 0;
    if ((D_8007C1A0 == 1) &&
        (overlay84GetCurrent() < 5) &&
        (overlay84IsUnitScale() != 0)) {
        gO57Draw32A0Selection1A0 = overlay84GetCurrent();
        if (gO57Draw32A0State118 == 1) {
            rising = 1;
        }
    }

    if (rising != 0) {
        envelope = (gO57Draw32A0Envelope124 += updateRate * 8);
        if (envelope >= 0xFF) {
            gO57Draw32A0Envelope124 = 0xFE;
            envelope = gO57Draw32A0Envelope124;
        }
    } else {
        envelope = (gO57Draw32A0Envelope124 =
                    gO57Draw32A0Envelope124 - updateRate * 32);
        if (envelope < 0) {
            gO57Draw32A0Envelope124 = 0;
            return;
        }
    }

    if (gO57Draw32A0TableMode18C[0] != 0) {
        rowCount[0] = 4;
        records = gO57Draw32A0Records3F8;
        position = 0x3E;
    } else {
        rowCount[0] = 3;
        records = gO57Draw32A0Records458;
        position = 0x4D;
    }

    rising = gO57Draw32A0WordTable4B8[gO57Draw32A0Selection1A0] |
             ((envelope * 5) >> 3);
    i = 1;
    cursor = &records[1];
    do {
        i++;
        cursor->packed8 = rising;
        cursor++;
    } while (i < 8);

    frontDrawRectangles(&D_800D3140, 8, records, 0);
    fontUseFont(2);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF,
                          gO57Draw32A0Envelope124);
    fontPrintXY(&D_800D3140, 0xF8, position,
                            gO57Draw32A0StripeReloc, 4);
    position += 0x1E;
    fontPrintXY(&D_800D3140, 0xF8, position,
                            gO57Draw32A0Local08, 4);
    position += 0x1E;
    fontPrintXY(&D_800D3140, 0xF8, position,
                            gO57Draw32A0Local18, 4);
    position += 0x1E;
    if (gO57Draw32A0TableMode18C[0] != 0) {
        fontPrintXY(&D_800D3140, 0xF8, position,
                                gO57Draw32A0Local28, 4);
    }

    i = 0;
    position -= 0x48;
    if (rowCount[0] > 0) {
        do {
            gO57Draw32A0Work34C[0].packed8 =
                ((((u16 *)&D_800D3128)[4 + gO57Draw32A0Selection1A0] &
                  gO57Draw32A0Masks3C8[i]) >> gO57Draw32A0Shifts3D8[i]) << 16;
            func_8002FB34(&D_800D3140,
                                   gO57Draw32A0Work34C, 248.0f,
                                   (f32)position, 1.0f, 1.0f,
                                   gO57Draw32A0Envelope124 | -256, 0x2003);
            i++;
            position += 0x1E;
        } while (i != rowCount[0]);
    }
}

/* Overlay 57 text +0x35E0..+0x3A4C. Matched 2026-10-01 (lane d-o057) from 162
 * masked words by discarding the inherited shape: no volatile primary-state
 * pointer and no sentinel local (uopt shares the one read across the head
 * comparisons itself), the id walks are `while (*list != -1)` with the list
 * read at each use, the tail is three early returns rather than an else-if
 * chain (which is what keeps the float literals and the table base out of
 * callee-saved registers), and the published index is read back after the
 * store. The two trailing locals are frame cells (L99). */
static void overlay57UpdateSelection(s32 updateRate) {
    s32 newPrimary;
    s32 newSecondary;
    Overlay57SelectionResult *result;
    s32 *list;
    s32 entry;
    s32 unused1;
    s32 unused2;

    if (overlay84GetActive() != 2) {
        return;
    }
    gO57SelectionActive144Reloc = 1;
    overlay84GetValues(&newPrimary, &newSecondary);
    if ((newPrimary != gO57SelectionPrimary4E8Reloc) ||
        (newSecondary != gO57SelectionSecondary4ECReloc)) {
        if ((gO57SelectionPrimary4E8Reloc != 0xFF) &&
            (newPrimary != gO57SelectionPrimary4E8Reloc)) {
            list = gO57SelectionIds134Reloc;
            while (*list != -1) {
                if (*list == gO57SelectionPrimary4E8Reloc) {
                    result = func_800508B4(*list);
                    if ((result != NULL) && (result->child != NULL) &&
                        (newSecondary != result->child->selector3B)) {
                        overlay57SetNodeValue(*list, gO57SelectionValues17CReloc[*list], 0.007f);
                    }
                }
                list++;
            }
        }
        if (newPrimary != 0xFF) {
            if (overlay57Call36F4Reloc(newPrimary) != NULL) {
                overlay57SetNodeValue(newPrimary, newSecondary, 0.01f);
                gO57SelectionChanging4F0Reloc = 1;
            }
        }
        gO57SelectionPrimary4E8Reloc = newPrimary;
        gO57SelectionSecondary4ECReloc = newSecondary;
    }

    if ((gO57SelectionChanging4F0Reloc != 0) && (gO57SelectionPrimary4E8Reloc != 0xFF)) {
        result = func_800508B4(gO57SelectionPrimary4E8Reloc);
        if ((result != NULL) && (gO57SelectionValue10CReloc.value < result->child->value28)) {
            gO57SelectionChanging4F0Reloc = 0;
            list = gO57SelectionIds134Reloc;
            while (*list != -1) {
                if (*list == gO57SelectionPrimary4E8Reloc) {
                    overlay57SetNodeValue(*list, gO57SelectionValues17CReloc[*list], 0.007f);
                }
                list++;
            }
        }
    }

    if (overlay84GetEnabledCurrent() != -1) {
        list = gO57SelectionIds134Reloc;
        while (*list != -1) {
            overlay57SetNodeValue(*list, gO57SelectionValues158Reloc[*list], 0.01f);
            list++;
        }
        gO57SelectionState118Reloc = 4;
        gO57SelectionTimer120Reloc = 10;
        return;
    }
    if ((D_800D31B8 & 0x4000) && (gO57SelectionDistance50CReloc == 0)) {
        amSndPlay(0xD, 0);
        overlay84LoadCurrent(0);
        gO57SelectionState118Reloc = 5;
        gO57SelectionMode11CReloc = 0;
        return;
    }
    entry = overlay84GetCurrent();
    if (entry == gO57SelectionCurrent100Reloc[0]) {
        return;
    }
    if (D_8007C1A0 == 1) {
        if ((gOverlay57Threshold38D8Reloc < -16) && (gO57SelectionDistance50CReloc == 0)) {
            overlay45ConfigureLayout(((void **)gO57SelectionList8Reloc)[entry], -0xA0, 0xBE, 4);
        } else {
            overlay45ConfigureLayout(((void **)gO57SelectionList8Reloc)[entry], 0x1E0, 0xBE, 4);
        }
        overlay45ConfigureLayout(((void **)gO57SelectionList8Reloc)[gO57SelectionCurrent100Reloc[0]], 0xA0, 0x104, 0x104);
    } else {
        if ((gOverlay57Threshold3970Reloc < -16) && (gO57SelectionDistance50CReloc == 0)) {
            overlay45ConfigureLayout(((void **)gO57SelectionList30Reloc)[entry], -0xA0, 0xBE, 4);
        } else {
            overlay45ConfigureLayout(((void **)gO57SelectionList30Reloc)[entry], 0x1E0, 0xBE, 4);
        }
        overlay45ConfigureLayout(((void **)gO57SelectionList30Reloc)[gO57SelectionCurrent100Reloc[0]], 0xA0, 0x104, 0x104);
    }
    gO57SelectionPrevious108Reloc = gO57SelectionCurrent100Reloc[0];
    gO57SelectionCurrent100Reloc[0] = entry;
    gO57SelectionValue10CReloc.word = gO57SelectionPending104Reloc;
    gO57SelectionPending104Reloc = 0xFF;
    gOverlay57PublishedIndex3A28Reloc = gO57SelectionCurrent100Reloc[0];
}

#undef O57_S32

#define D_0x118 gO57ModeState118BaseReloc[0x118 / 4]

#define D_0x11C gO57ModeState11CBaseReloc[0x11C / 4]

#define D_0x120 gO57ModeCountdown120

#define D_0x144 gO57ModeState144BaseReloc[0x144 / 4]

#define D_0x160 gO57ModeState160BaseReloc[0x160 / 4]

#define D_0x164 gO57ModeState164BaseReloc[0x164 / 4]

#define D_0x184 gO57ModeState184BaseReloc[0x184 / 4]

#define D_0x4F8 gO57ModeChoice4F8

#define D_0x50C gO57ModeGate50C

#define O57_JOIN_RAW(left, right) left##right

#define O57_JOIN(left, right) O57_JOIN_RAW(left, right)

#define O57_S32(offset) O57_JOIN(D_, offset)

#define overlay57Call3BC4Reloc overlay84GetCurrent

#define overlay57Call3C08Reloc overlay84GetCurrent

#define overlay57Call3C5CReloc overlay84GetCurrent

#define overlay57Call3C94Reloc overlay84GetCurrent

#define overlay57Call3D08Reloc overlay84GetCurrent

#define overlay57Call3D34Reloc func_800508B4

#define overlay57Call3DD4Reloc overlay84GetCurrent

#define overlay57Call3E18Reloc overlay84GetCurrent

#define overlay57Call3E6CReloc overlay84GetCurrent

#define overlay57Call3EE0Reloc overlay84GetCurrent

#define overlay57Call3F0CReloc func_800508B4

/* Matched 2026-10-01 by discarding the inherited shape, not by allocator
 * work.  The 5-word plateau was recorded as a floor of 2; that floor belonged
 * to the shape.  What got it there, each measured:
 *
 *   - ONE symbol for the timer.  The candidate reached the timer through seven
 *     alias arrays (one per store site, one for the reload after the call) so
 *     that only three sites shared an address web.  With every site naming
 *     gO57ModeTimer114 the address is one web spanning the dispatch; uopt
 *     splits it, and a split piece re-enters after the call with the value
 *     reload first and the address second, which is the order the scheduler
 *     tie needed and no line numbering could supply.
 *   - No `timer` local: the test and the decrement read the global, and the
 *     reload after the call is the compiler's own.
 *   - The loop's dead post-decrement copy is the function-scope `eligible`
 *     instead of a per-arm `prev`.  Redefining `eligible` in the arms stops
 *     the copy into `savedEligible` from being propagated, so the store stays
 *     where it is written, and the address piece is coloured ahead of it.
 *   - Declaration order eligible, savedEligible, entries lands the frame.
 *
 * Also natural and byte-inert here: the in-place countdown decrement, the
 * if/else dispatch in place of two gotos, and storing the call result straight
 * into gO57ModeChoice4F8. */
static void overlay57UpdateModeState(s32 updateRate) {
    s32 eligible;
    s32 savedEligible;
    Overlay57MarkedEntry *entries;

    O57_S32(0x144) = 1;
    {
        s32 *list;
        u8 id;

        if (gO57ModeCountdown120 > 0) {
            gO57ModeCountdown120 -= updateRate;
            if (gO57ModeCountdown120 > 0) {
                return;
            }

            list = gO57ModePrimaryIds134;
            while (*list != -1) {
                id = (u8)*list;
                animseqStopPath(id);
                list++;
            }

            list = gO57ModeSecondaryIds1A0;
            while (*list != -1) {
                id = (u8)*list;
                animseqStartPath(id);
                overlay57SetNodeValue(*list, 1, 0.0079f);
                list++;
            }

            gO57ModeTimer114 = 60;
            O57_S32(0x11C) = 0;
            return;
        }
    }

    if ((gO57ModeTimer114 >= 31) && ((gO57ModeTimer114 - updateRate) < 31)) {
        amSndPlay(0x10, 0);
    }
    gO57ModeTimer114 -= updateRate;

    eligible = (D_800D31B8 & 0xD000) != 0;
    if (eligible != 0) {
        eligible = gO57ModeGate50C == 0;
    }
    savedEligible = eligible;
    if (gO57ModeTimer114 > 0) {
        if (eligible == 0) {
            return;
        }
    }

    if (D_8007C1A0 == 1) {
        if (overlay57Call3BC4Reloc() == 8) {
            mainChangeLevel(0x2A, 0, 0, 0xE, 1, 0);
            gO57ModeGate50C = 1;
            overlay84Mark();
        } else if (overlay57Call3C08Reloc() == 5) {
            O57_S32(0x118) = 9;
            O57_S32(0x164) = 0;
            O57_S32(0x160) = 0;
            O57_S32(0x184) = 0x2F;
            gO57ModeTimer114 = 0xB4;
            overlay84LoadCurrent(1);
        } else if (overlay57Call3C5CReloc() == 6) {
            O57_S32(0x118) = 12;
            gO57ModeTimer114 = 0xB4;
            overlay84LoadCurrent(2);
        } else if (overlay57Call3C94Reloc() == 7) {
            O57_S32(0x118) = 21;
            O57_S32(0x160) = 0;
            O57_S32(0x164) = 0;
            O57_S32(0x184) = 0x4B;
            gO57ModeTimer114 = 0xB4;
            overlay84LoadCurrent(1);
        } else {
            Overlay57LookupResult *result;
            Overlay57SelectionChild *child;
            Overlay57Selection *selection;
            s32 count;

            O57_S32(0x118) = 15;
            gO57ModeSetup21C.horizontal = 0x17C;
            gO57ModeSetup21C.vertical = 0xBE;
            gO57ModeChoice4F8 = overlay57Call3D08Reloc();
            gOverlay57Byte3D1CReloc = gO57ModeChoice4F8;
            animseqStopPath(0x4D);
            overlay57UpdateNode();
            result = overlay57Call3D34Reloc(0x4C);
            if (result != 0) {
                child = result->child08;
                if (child != 0) {
                    child->selector3A = gO57ModeChoice4F8;
                    child = result->child08;
                    selection = child->selections68[child->selector3A];
                    if (selection != 0) {
                        entries = selection->entries4C;
                        if (entries != 0) {
                            count = selection->countOwner00->count2C;
                            eligible = count--;
                            if (eligible) {
                                do {
                                    if ((entries->flags04 & 0x00100000) != 0) {
                                        entries->value00 = 0x100;
                                    }
                                    entries++;
                                    eligible = count--;
                                } while (eligible);
                            }
                        }
                    }
                }
            }
            overlay84LoadCurrent(3);
        }
    } else {
        if (overlay57Call3DD4Reloc() == 8) {
            mainChangeLevel(0x2A, 0, 0, 0xE, 1, 0);
            gO57ModeGate50C = 1;
            overlay84Mark();
        } else if (overlay57Call3E18Reloc() == 5) {
            O57_S32(0x118) = 9;
            O57_S32(0x164) = 0;
            O57_S32(0x160) = 0;
            O57_S32(0x184) = 0x2F;
            gO57ModeTimer114 = 0xB4;
            overlay84LoadCurrent(1);
        } else if (overlay57Call3E6CReloc() == 6) {
            O57_S32(0x118) = 21;
            O57_S32(0x160) = 0;
            O57_S32(0x164) = 0;
            O57_S32(0x184) = 0x4B;
            gO57ModeTimer114 = 0xB4;
            overlay84LoadCurrent(1);
        } else {
            Overlay57LookupResult *result;
            Overlay57SelectionChild *child;
            Overlay57Selection *selection;
            s32 count;

            O57_S32(0x118) = 15;
            gO57ModeSetup21C.horizontal = 0x17C;
            gO57ModeSetup21C.vertical = 0xBE;
            gO57ModeChoice4F8 = overlay57Call3EE0Reloc();
            gOverlay57Byte3EF4Reloc = gO57ModeChoice4F8;
            animseqStopPath(0x4D);
            overlay57UpdateNode();
            result = overlay57Call3F0CReloc(0x4C);
            if (result != 0) {
                child = result->child08;
                if (child != 0) {
                    child->selector3A = gO57ModeChoice4F8;
                    child = result->child08;
                    selection = child->selections68[child->selector3A];
                    if (selection != 0) {
                        entries = selection->entries4C;
                        if (entries != 0) {
                            count = selection->countOwner00->count2C;
                            eligible = count--;
                            if (eligible) {
                                do {
                                    if ((entries->flags04 & 0x00100000) != 0) {
                                        entries->value00 = 0x100;
                                    }
                                    entries++;
                                    eligible = count--;
                                } while (eligible);
                            }
                        }
                    }
                }
                overlay57ApplyTable();
            }
            overlay84LoadCurrent(3);
        }
    }

    if (savedEligible != 0) {
        overlay84ResetCurrent();
    }
}

/* DKR v77/v80 and JFG have no matching donor; 0xA00 is generic-only. */
static void overlay57BeginMode(Overlay57ModeValue value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay84GetActive(value.value) == 0) {
        gOverlay57Delay = 0xA00;
        object = (Overlay57Object *)&gOverlay57Object;
        gOverlay57Timer = 13;
        object->value = 0x3F;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
        value.value = 1;
        overlay94SetValue(value.value);
    }
}

/* Overlay 57 text +0x4064..+0x43C8. */
/* THE JOINED LINE IS LOAD-BEARING. `outputIndex = 0; do {` shares one physical
 * line deliberately; splitting it costs the three-word schedule residual back.
 *
 * IDO hoists both loop base addresses into the loop preheader and stamps the
 * hoisted records with the *loop header's* source line, not their use sites'.
 * The index initializer and those two addresses are independent -- no
 * dependence edge orders them -- so as1's reorganizer separates them on its
 * minimized `lineno` key, and an initializer written on an earlier line wins.
 * Sharing the loop header's line removes the separation. This is why the
 * 119-entry flag lattice and ten source variants could not move it: the
 * deciding key was never a flag or a statement order, it was the line number.
 *
 * Measured with `decomp-workbench trace-emit` on an emit-provenance
 * instrumented ugen (workbench improvement-backlog item #3(b)): block 0 emits
 * the initializers at lines 90 and 91 and both hoisted addresses at line 92.
 * Canonical -O2 -mips2 stays exact-size at 0x364 with the exact 0x58 frame;
 * the three relocation-masked differences at +0xE0..+0xE8 are gone. What
 * remains is the pre-existing relocation-surface difference (four words where
 * the target resolves a shared base plus offset and the C object emits
 * per-symbol LO16 records), which is a symbol-layout question, not a
 * schedule one. */
static void overlay57HandleModeInput(s32 updateRate) {
    s32 i;
    s32 outputIndex;
    s32 *idPtr;
    s8 enabled[4];

    gOverlay57ModeFlag = 0;
    gOverlay57State = 3;

    if (((D_800D31B8 & 0x9000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(12, 0);

        gO57ModeOutputs[0].value =
            (s8) gO57ModeValueTable[gO57ModeChoices[0].tableIndex];
        gO57ModeOutputs[0].mode = 2;
        D_8007BEF8 = 1;
        D_8007BEFC = 0;
        D_8007BF0C = 0;
        D_8007BF04 = 0;
        gO57ModeShortValue = 0x3FC;
        D_8007BF08 = 0;

        i = 0;
        outputIndex = 0; do {
            enabled[i] = gO57ModeChoices[i].enabled;
            if (gO57ModeChoices[i].enabled != 0) {
                gO57ModeOutputs[outputIndex].value =
                    (s8) gO57ModeValueTable[
                        gO57ModeChoices[i].tableIndex];
                outputIndex++;
            }
            i++;
        } while (gO57ModeChoicesEnd != &gO57ModeChoices[i]);

        joyCreateMap(enabled);
        mainSetMode(0);
        mainChangeLevel(0x1E,
                               gO57ModeValueTable[
                                   gO57ModeChoices[0].tableIndex],
                               0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    } else if (((D_800D31B8 & 0x4000) != 0) &&
               (gOverlay57DistanceState == 0)) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gO57ModeExternalByte);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(2);

        idPtr = (s32 *)gO57ModePrimaryIds;
        while (*idPtr != -1) {
            animseqStartPath((u8) *idPtr);
            overlay57SetNodeValue(*idPtr, gO57ModePrimaryValues[*idPtr],
                                  0.0070000002f);
            idPtr++;
        }

        idPtr = (s32 *)gO57ModeSecondaryIds;
        while (*idPtr != -1) {
            animseqStopPath((u8) *idPtr);
            idPtr++;
        }
        overlay94SetValue(0);
    }

    for (i = 0; i < updateRate; i++) {
        gOverlay57Delay -= gOverlay57Delay >> 3;
    }

    func_8002FB34(&D_800D3140, gO57ModeFinalArg1,
                            (f32) ((gOverlay57Delay >> 5) + 0x104), 184.0f,
                            1.0f, 1.0f, -2, 3);
}

/* DKR v77/v80 and JFG have no matching donor for this mode transition. */
static void overlay57StartMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay84GetActive(value) == 0) {
        object = (Overlay57Object *)&gOverlay57Object;
        gOverlay57Timer = 16;
        object->value = 68;
        gOverlay57State = 5;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    overlay57ApplyTable();
    func_overlay_057_F0001020_18A4C18(value);
}

/* Cross-overlay callees reached through the module's relocation table
 * (stored target zero): overlay 84 +0x12FC, +0x1350, +0x1060 and +0x1398. */  /* (was on extern overlay84GetEnabledCurrent) */
/* Overlay 57 text +0x4460..+0x4C18, the one-player menu step and start.
 * Matched 2026-10-02 (lane o-ovl7) from 356 masked words on the matched
 * sibling func_overlay_057_F00060F8_18A9CF0's shape: the resident player
 * count is one word read at every use (it had been split across three
 * names, and the two course tables and the two unlock callees had been
 * crossed), the choice walk and fill read the source table and controller
 * map directly, and the declared locals give the 0x58 frame ladder. The
 * smoothing loop and both later loops share one index local, which coalesces the count web with its scaled induction
 * (the `or a3,a0` copy), and the unlock test is written bit-first. */
static void func_overlay_057_F0004460_18A8058(s32 updateRate) {
    s8 activePlayers[10];
    O57MenuLink *link;
    s32 i;
    s32 current;
    s32 index;
    s8 playerOrder[4];

    gOverlay57ModeFlag = 0;
    for (index = 0; index < updateRate; index++) {
        gO57ModeSetup21C.horizontal += (0x104 - gO57ModeSetup21C.horizontal) >> 3;
        gO57ModeSetup21C.vertical += (0xBE - gO57ModeSetup21C.vertical) >> 3;
    }

    if (D_800D31BE < -16 && gOverlay57DistanceState == 0) {
        if (gOverlay57TableIndex < gO57MenuSelectionCountReloc) {
            gOverlay57TableIndex++;
            gOverlay57LayoutBusy = 1;
        }
    } else if (D_800D31BE >= 17 && gOverlay57DistanceState == 0) {
        if (gOverlay57TableIndex > 0) {
            gOverlay57TableIndex--;
            gOverlay57LayoutBusy = 1;
        }
    }

    overlay57ApplyTable();
    if ((D_800D31B8 & 0x9000) && gOverlay57DistanceState == 0) {
        amSndPlay(12, 0);
        current = overlay84GetEnabledCurrent();
        mainChangeCameras(D_8007C1A0);
        if (D_8007C1A0 == 1) {
            gOverlay57MenuEntries[0].type = 0;
            D_8007BEF8 = 6;
            D_8007BEFC = 5;
            D_8007BF0C = 0;
            if (gOverlay57TableIndex == 3) {
                D_8007BF04 = 2;
                D_8007BF08 = 1;
            } else {
                D_8007BF04 = gOverlay57TableIndex;
                D_8007BF08 = 0;
            }
            gOverlay57MenuHalf = 0x3FC;
        } else {
            gOverlay57MenuEntries[0].type = 3;
            if ((D_8007C1A0 == 2 || D_8007C1A0 == 3) &&
                gOverlay57MirrorFlag != 0) {
                D_8007BEFC = 4 - D_8007C1A0;
                D_8007BEF8 = 4;
            } else {
                D_8007BEFC = 0;
                D_8007BEF8 = D_8007C1A0;
            }
            D_8007BF0C = 1;
            D_8007BF04 = 2;
            gOverlay57MenuHalf = gO57MiddleData31B4;
            if (gOverlay57TableIndex == 3) {
                D_8007BF08 = 1;
            } else {
                D_8007BF08 = 0;
            }
        }

        for (i = 0; i < 10; i++) {
            activePlayers[i] = 1;
        }
        if (D_8007C1A0 == 1) {
            func_8003A754();
        }
        for (i = 0, index = 0; &gOverlay57MenuSources[index] < &gOverlay57MenuSources[4]; index++) {
            playerOrder[index] = gOverlay57MenuSources[index].active;
            if (gOverlay57MenuSources[index].active != 0) {
                gOverlay57MenuEntries[i].controller =
                    gOverlay57ControllerMap[gOverlay57MenuSources[index].index];
                activePlayers[gOverlay57ControllerMap[gOverlay57MenuSources[index].index]] = 0;
                i++;
            }
        }
        i = 0;
        for (index = D_8007C1A0; index < 6; index++) {
            while (activePlayers[i] == 0) {
                i++;
            }
            gOverlay57MenuEntries[index].controller = i;
            i++;
        }
        joyCreateMap(playerOrder);
        gOverlay57MenuEntries[0].mode = current;
        gOverlay57MenuEntries[0].variant = 0;
        gOverlay57MenuEntries[0].subtype = 3;
        if (D_8007BF0C == 0 || (D_8007BF0C != 0 && gO57ChoicePublishedReloc != 0)) {
            func_80005548(D_8007BEF8);
            if (gO57ChoicePublishedReloc != 0) {
                gO57ChoicePublishedReloc = 0;
            }
        }

        D_8007BF44 = 0;
        if (D_8007C1A0 == 1) {
            if (D_800D3128.unlockPending) {
                D_800D3128.unlockPending = 0;
                func_800291B4();
                func_8003A680(0);
            }
            i = gO57UnlockTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant];
            if (i != -1) {
                if (!((1 << i) & D_800D3128.unlockMask)) {
                    D_800D3128.unlockMask |= 1 << i;
                    func_800291B4();
                    func_8003A680(
                        gO57UnlockTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant] + 0xE);
                }
            }
        }

        if (D_8007BF44 > 0) {
            gO57MiddleData31E8 = gO57CourseTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant];
            gO57MiddleData31EC = gOverlay57ControllerMap[gOverlay57MenuSources[0].index];
            gO57MiddleData31F0 = 5;
            gO57MiddleData31F4 = 0;
            mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
            mainSetAnimGroup(1);
        } else {
            mainSetMode(0);
            mainChangeLevel(gO57CourseTable[gOverlay57MenuEntries[0].mode][gOverlay57MenuEntries[0].variant],
                            gOverlay57ControllerMap[gOverlay57MenuSources[0].index], 0, 5, 1, 0);
        }
        gOverlay57DistanceState = 1;
        overlay84Mark();
    } else if ((D_800D31B8 & 0x4000) && gOverlay57DistanceState == 0) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(3);

        link = gO57ModePrimaryIds;
        while (link->index != -1) {
            animseqStartPath(link->index);
            overlay57SetNodeValue(link->index, gO57ModePrimaryValues[link->index], 0.007f);
            link++;
        }
        link = gO57ModeSecondaryIds;
        while (link->index != -1) {
            animseqStopPath(link->index);
            link++;
        }
        gOverlay57State = 0;
    }
}


/* Physical a0 retains updateRate at this site; consumption is not proven. */  /* (was on extern overlay57TailQueryModeReloc) */
/* Overlay 57 text +0x4C18..+0x4D90. Exact: 94 words, frame 0x28, 38/38
 * relocation identities.
 *
 * The setup array is one six-iteration constant loop. uopt unrolls it by four,
 * so it peels 6 % 4 = 2 iterations -- those fold to the literal stores of 0 and
 * 0x30 -- and emits a single unrolled body whose induction variable stays
 * register-resident at 2 rather than being propagated. A straight-line
 * reconstruction of the six stores cannot reach that shape. The body is on the
 * `for` line because as1 breaks scheduling ties on the source line, and a
 * braced body puts the unrolled copies on a different line than the peeled
 * ones.
 *
 * framePad is declared before `trigger` because the declaration order fixes
 * which frame cells the two spill homes take; declared after, the frame closes
 * at 0x20 instead of 0x28.
 *
 * The BSS block below is this TU's: IDO shares one `lui $at` across the two
 * constant-index setup stores only for a locally-defined symbol, and that
 * shared materialisation is in the shipped bytes. `static` makes every access
 * a section-relative record whose addend is already the module-relative offset
 * the shipped word carries, so the static link must not adjust it a second
 * time -- overlay 57's own runtime relocation table owns those 31 records, and
 * mk/overlays.mk drops them with filter_elf_relocations.py. IDO 4-aligns .bss
 * scalars and 8-aligns arrays, which is why the interior pads are scalars:
 * 0x13C, 0x144 and 0x194 are not 8-aligned. */
static void overlay57UpdateModeTrigger(s32 updateRate) {
    s32 framePad[2];
    s32 trigger;

    gOverlay57ModeFlag = 1;

    if (gOverlay57Countdown > 0) {
        gOverlay57Countdown = gOverlay57Countdown - updateRate;
        if (gOverlay57Countdown <= 0) {
            trigger = 1;
        }
    }

    gOverlay57SetupStatus = 0;
    if (overlay57TailQueryModeReloc() == 0) {
        s32 index;
        s32 i;
        Overlay57ModeResult *result;

        index = 2;

        for (i = 0; i < 6; i++) gOverlay57SetupValues[i] = i * 0x30;

        trigger = 1;
        if (overlay57TailQueryChoiceReloc(index) == 5) {
            gOverlay57Timer = 10;
        }

        gOverlay57ObjectStatus = 0;
        gOverlay57Object.value = 80;
        overlay57TailPrepareObjectReloc(((u8 *)&gOverlay57Object)[3]);
        overlay57TailStartObjectReloc(((u8 *)&gOverlay57Object)[3]);
        result = overlay57TailFindObjectReloc(((u8 *)&gOverlay57Object)[3]);
        if (result != 0) {
            result->flags16 |= 2;
        }

        if (gOverlay57TriggerLatched != 0) {
            gOverlay57SetupDelay = 0;
        } else {
            gOverlay57SetupDelay = 60;
        }
    }

    /* Retail reads this deliberately uninitialized value when neither path
     * above assigns it. Initializing it changes the physical function. */
    if ((trigger != 0) && (gOverlay57TriggerLatched == 0)) {
        overlay57SetNodeValue(0x2F, 0, 0.012f);
        gOverlay57TriggerLatched = 1;
    }

    overlay57AdvanceReloc(updateRate);
}

/* DKR v77/v80 and JFG checks found no matching mode-initialization donor. */
static void overlay57InitializeMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 1;
    if (overlay84GetActive(value) == 0) {
        object = (Overlay57Object *)&gOverlay57Object;
        gOverlay57Timer = 20;
        object->value = 84;
        func_8005055C(((u8 *) object)[3]);
        animseqStartPath(gOverlay57ObjectId);
        result = func_800508B4(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    func_overlay_057_F0001020_18A4C18(value);
}

/* Tier B: calls and globals separated by overlay 57 runtime relocations.
 * Reserved data/BSS selectors remain separate from overlay-local storage. */  /* (was on extern D_800D3128) */
/* Overlay 57 text +0x4E18..+0x60F8. Matched 2026-10-01 (lane c-o057big) by
 * discarding inherited carriers rather than by allocator work; 86 -> 0 masked:
 *
 *  - the flag word is a bitfield (unsigned halfword test, 0xFFFE byte clear);
 *  - the +/-6 page steps read and update the selection global in place, and
 *    the page number compared against `state` is its own local;
 *  - the record pointer is `records[index].items` with the first call's
 *    result in a declared local, so the countdown fill's post-decrement
 *    temporary does not share a compiler temporary with it;
 *  - `panelX + 0xA0` and `panelX + 0x56` are written at each use. uopt spills
 *    the two commoned values itself, and the row and column loops are indexed
 *    so the strength-reduction temporaries interleave with those spill slots
 *    exactly as the target's frame has them (homes 0x64 and 0x5C);
 *  - the empty-row arm formats before it sets the icon index;
 *  - the fill is `count = 10; while (count--)`, the choice walk subscripts
 *    sourceState, and the tail reads the player count and the choice table
 *    directly instead of through carrier locals.
 *
 * Locals that are declared and never referenced are frame cells: every
 * declared scalar reserves a home in declaration order (L99), and the homed
 * arrays sit at the target's offsets only with this count between them.
 *
 * The seven o57MiddleO<NN>F<offset>Reloc externs are cross-overlay callees
 * reached through the module's relocation table (stored target zero). */
static void func_overlay_057_F0004E18_18A8A10(s32 updateRate) {
    s32 i;
    s32 index;
    s32 limit;
    s16 input;
    s32 remainder;
    s32 previousGroup;
    s32 currentGroup;
    s32 page;
    s8 sourceState[4];
    u8 activePlayers[10];
    s32 nextSelection;
    s32 panelX;
    s32 characterId;
    s32 nextValue;
    s32 cursorValue;
    s32 outputIndex;
    s32 state;
    s32 valueA;
    s32 valueB;
    s32 valueC;
    s32 *list;
    s32 row;
    O57MiddleRenderItem *renderItems;
    O57MiddleRecord *records;
    O57MiddleTextureNode textureNodes[2];
    O57MiddleChoice *choice;
    char stackB0[2];
    s16 *color;
    char *palette;
    char renderState[24];
    u8 *active;
    s8 *source;
    O57MiddleOutput *output;
    s32 stack80;
    s32 stack7C;
    s32 stack78;
    s8 rank;

    if (overlay84GetActive() == 0) {
        if (gO57MiddleMode != 7) {
            gO57MiddleMode = 2;
        }

        if (D_800D3128.allUnlocked) {
            limit = 0x14;
            state = 3;
        } else if (gO57MiddleUnlocked19C != 0) {
            limit = 0x13;
            state = 3;
        } else if (gO57MiddleUnlocked198 != 0) {
            limit = 0xF;
            state = 2;
        } else {
            state = 1;
            limit = 0xB;
        }

        if (gO57MiddleState194 == 0) {
            page = gO57MiddleSelection / 6;
            if ((D_800D31BC < -16) && (gO57MiddleSelection > 0) && (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection -= 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 5) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x5A;
                    overlay57SetNodeValue(
                        0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                }
            } else if ((D_800D31BC >= 17) && (gO57MiddleSelection < limit) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection += 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 0) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    overlay57SetNodeValue(
                        0x2F, gO57MiddleSelection / 6, 0.012f);
                }
            } else if ((D_800D31BE < -16) && (gO57MiddleSelection < (limit - 2)) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                previousGroup = gO57MiddleSelection / 6;
                gO57MiddleSelection += 3;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                currentGroup = gO57MiddleSelection / 6;
                if (previousGroup != currentGroup) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    overlay57SetNodeValue(
                        0x2F, currentGroup, 0.012f);
                }
            } else if ((D_800D31BE >= 17) && (gO57MiddleSelection >= 3) &&
                    (gO57MiddleTransition == 0)) {
                    gO57MiddleMoving = 1;
                    overlay45ConfigureLayout(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                    gO57MiddlePreviousSelection = gO57MiddleSelection;
                    previousGroup = gO57MiddleSelection / 6;
                    gO57MiddleSelection -= 3;
                    overlay45ConfigureLayout(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                    gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                    gO57MiddleAlpha = 0xFF;
                    currentGroup = gO57MiddleSelection / 6;
                    if (previousGroup != currentGroup) {
                        gO57MiddleFade = 0;
                        gO57MiddleFadeDelay = 0x3C;
                        overlay57SetNodeValue(
                            0x2F, currentGroup + 5, 0.012f);
                    }
                } else if ((D_800D31B8 & 0x2020) && (gO57MiddleSelection >= 6) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection -= 6;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                overlay57SetNodeValue(
                    0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                } else if ((D_800D31B8 & 0x10) && (page < state) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection += 6;
                overlay45ConfigureLayout(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                overlay57SetNodeValue(
                    0x2F, gO57MiddleSelection / 6, 0.012f);
                if (gO57MiddleSelection > limit) {
                    gO57MiddleSelection = limit;
                }
                }

            if ((D_800D31B8 & 0x4000) && (gO57MiddleTransition == 0) &&
                (gO57MiddleCanLeave != 0)) {
                func_80000F94(0xD, 0);
                overlay84ClearMode();
                func_80050704(gO57MiddlePathId);
                gO57MiddleNextMode = 7;
                overlay57SetNodeValue(
                    0x2F, 0, -0.012f);
                gO57MiddleState188 = 0;
                gO57MiddleMode = 0;
                overlay84ActivateCurrent(1);

                list = gO57MiddlePathList;
                while (*list != -1) {
                    func_80050688((u8)*list);
                    overlay57SetNodeValue(
                        *list, gO57MiddlePathIndices[*list], 0.007f);
                    list++;
                }
                list = gO57MiddleStopList;
                while (*list != -1) {
                    func_80050704((u8)*list);
                    list++;
                }
            }

            for (i = 0; i < updateRate; i++) {
                gO57MiddlePanelPosition += (-0x1400 - gO57MiddlePanelPosition) >> 2;
            }
        } else {
            if (gO57MiddleState194 == 1) {
                for (i = 0; i < updateRate; i++) {
                    gO57MiddlePanelPosition += (-gO57MiddlePanelPosition) >> 2;
                }
            }
            if ((D_800D31B8 & 0x4000) && (gO57MiddleTransition == 0)) {
                func_80000F94(0xD, 0);
                gO57MiddleState194 = 0;
                gO57MiddleMode = 2;
            }
        }

        func_overlay_057_F0001020_18A4C18(updateRate);
        if ((gO57MiddlePanelPosition >> 4) >= -0x135) {
            records = func_800291C4();
            renderItems = records[func_80025D60(D_8007C0E8[gO57MiddleSelection])].items;
            fontUseFont(0);
            row = 0x51;
            panelX = gO57MiddlePanelPosition >> 4;
            D_800D3550.position = (f32) panelX;
            D_800D3550.scale = 7.0f;
            func_80022A50(&D_800D3140, &D_800D3144);
            frontDrawObj(8);
            func_8004B0B8(
                gO57MiddleInfo.value08, gO57MiddleInfo.value09, gO57MiddleInfo.value0A,
                0xFF, 0xFF);
            fontPrintXY(
                &D_800D3140, panelX + 0xA0, 0x23,
                D_8007C0B8[D_8007C11C[gO57MiddleSelection]], 4);
            func_8004B0B8(0xFF, 0x80, 0, 0xFF, 0xFF);
            fontPrintXY(
                &D_800D3140, panelX + 0xA0, 0x37,
                D_8007C0B8[0xD0 / 4], 4);
            func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            if (overlay68CheckKind(
                    D_8007C0E8[gO57MiddleSelection]) != 0) {
                    func_8002F618(
                        &D_800D3140, gO57MiddleBadge, panelX + 0x30,
                        0x26, 0xFF, 0xFF, 0xFF, 0xFF);
            }

            for (i = 0; i < 4; i++) {
                overlay56SplitTime(
                    renderItems[i].type, &stack80, &stack7C, &stack78);
                if (renderItems[i].type == 0) {
                    func_800429A4(renderState, gO57MiddleEmptyFormat);
                    valueA = 0x4A;
                } else {
                    valueB = func_8003A700(
                                 renderItems[i].value04) & 0xFF;
                    valueC = func_8003A700(
                                 renderItems[i].value05) & 0xFF;
                    func_800429A4(
                        renderState, gO57MiddleResultFormat, valueB, valueC,
                        func_8003A700(
                            renderItems[i].value06),
                        stack80, stack7C, stack78);
                    valueA = renderItems[i].value07 + 0x51;
                }
                if (i < 3) {
                    fontPrintXY(
                        &D_800D3140, panelX + 0x2E, row,
                        gO57MiddleLabels[i], 0);
                }
                textureNodes[0].texture = D_800D31C8[valueA];
                textureNodes[0].alternate = NULL;
                textureNodes[0].x = panelX + 0x56;
                textureNodes[0].y = (s16)(row - 4);
                textureNodes[0].packedOffset = 0;
                textureNodes[1].texture = NULL;
                func_8002F618(
                    &D_800D3140, textureNodes, 0, 0,
                    0xFF, 0xFF, 0xFF, 0xFF);
                for (index = 0; index < 11; index++) {
                    stackB0[0] = renderState[index];
                    stackB0[1] = 0;
                    fontPrintXY(
                        &D_800D3140, gO57MiddleColumns[index] + panelX, row, stackB0, 0);
                }
                if (i == 2) {
                    row += 0x1B;
                    func_8004B0B8(
                        0xFF, 0x80, 0, 0xFF, 0xFF);
                    fontPrintXY(
                        &D_800D3140, panelX + 0xA0, row,
                        D_8007C0B8[0xD4 / 4], 4);
                    func_8004B0B8(
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                row += 0x1B;
            }
            func_8002FB34(
                &D_800D3140, gO57MiddleCaption, (f32)(panelX + 0x2D),
                184.0f, 1.0f, 1.0f, -2, 3);
        }

        if (gO57MiddleFadeDelay > 0) {
            gO57MiddleFadeDelay -= updateRate;
        } else {
            gO57MiddleFade += updateRate * 4;
            if (gO57MiddleFade >= 0x100) {
                gO57MiddleFade = 0xFF;
            }
        }

        if ((D_800D31B8 & 0x9000) && (gO57MiddleTransition == 0)) {
            func_80000F94(0xC, 0);
            if (D_8007C1A0 >= 2 || gO57MiddleState194 == 1) {
                outputIndex = 10;
                while (outputIndex--) {
                    activePlayers[outputIndex] = 1;
                }
                i = 0;
                for (index = 0; &gO57MiddleChoices[index] < &gO57MiddleChoices[4]; index++) {
                    rank = gO57MiddleChoices[index].active;
                    sourceState[index] = rank;
                    if (rank != 0) {
                        gO57MiddleOutput[i].controller =
                            gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex];
                        activePlayers[gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex]] = 0;
                        i++;
                    }
                }
                i = 0;
                for (outputIndex = D_8007C1A0; outputIndex < 6; outputIndex++) {
                    while (activePlayers[i] == 0) {
                        i++;
                    }
                    gO57MiddleOutput[outputIndex].controller = i;
                    i++;
                }
                func_80025444(
                    sourceState);
                func_80028D24(0);
                func_80028540(D_8007C1A0);
                D_8007BF44 = 0;
                if (D_8007C1A0 == 1) {
                    gO57MiddleOutput[0].value0 = 1;
                    if (D_800D3128.pending) {
                        D_800D3128.pending = 0;
                        func_800291B4();
                        func_8003A680(0x14);
                    }
                } else {
                    gO57MiddleOutput[0].value0 = 4;
                }
                if (((D_8007C1A0 == 2) || (D_8007C1A0 == 3)) &&
                    (gO57MiddleData3214 != 0)) {
                    D_8007BEFC = (s8)(4 - D_8007C1A0);
                    D_8007BEF8 = 4;
                } else {
                    D_8007BEFC = 0;
                    D_8007BEF8 = (u8)D_8007C1A0;
                }
                D_8007BF0C = (u8)(D_8007C1A0 >= 2);
                D_8007BF04 = 2;
                gO57MiddleData31B8 = gO57MiddleData31B4;
                D_8007BF08 = 0;
                if (D_8007BF44 > 0) {
                    gO57MiddleData31E8 = D_8007C0E8[gO57MiddleSelection];
                    gO57MiddleData31EC = gO57MiddleCharacterIds[gO57MiddleChoices[0].tableIndex];
                    gO57MiddleData31F0 = 5;
                    gO57MiddleData31F4 = 0;
                    func_80028374(0x12, 0, 0, 0xF, 1, 0);
                    func_80028528(1);
                } else {
                    func_80028374(
                        D_8007C0E8[gO57MiddleSelection],
                        gO57MiddleCharacterIds[gO57MiddleChoices[0].tableIndex],
                        0, 5, 1, 0);
                }
                gO57MiddleTransition = 1;
                overlay84Mark();
                if ((D_8007BF0C == 0) ||
                    ((D_8007BF0C != 0) && (gO57MiddleData3208 != 0))) {
                    func_80005548(D_8007BEF8);
                    if (gO57MiddleData3208 != 0) {
                        gO57MiddleData3208 = 0;
                    }
                }
            } else {
                gO57MiddleState194 = 1;
            }
        }
    }
}

/* Cross-overlay callees reached through the module's relocation table
 * (stored target zero): overlay 84 +0xC74, +0x1350, +0x1060, +0x1398 and
 * overlay 45 +0x314. The resident calls are renamed onto their generated
 * surface entries by the object's POSTPROCESS rule. */  /* (was on extern overlay84GetActive) */
/* Overlay 57 text +0x60F8..+0x67DC, the four-way menu step and start. Matched
 * 2026-10-01 (lane d-o057) from 252 masked words on overlay 57's middle-panel
 * template (func_overlay_057_F0004E18_18A8A10): the start and stop id walks
 * are `while (link->index != -1)` with the link read at each use, the player
 * setup is that function's countdown fill and choice walk reading the source
 * table and the controller map directly, the player count is one resident
 * word read at every use (it was split across three names), the entry value
 * reset is a loop IDO unrolls, and the entrance argument indexes the
 * controller map by the first source. */
static void func_overlay_057_F00060F8_18A9CF0(s32 updateRate) {
    s32 count;
    s32 i;
    Overlay57IndexLink *link;
    s8 playerOrder[4];
    u8 activePlayers[10];
    s32 index;
    s8 rank;

    if (overlay84GetActive() != 0) {
        return;
    }

    gOverlay57State = 6;
    if (D_800D31BC < -16 && gOverlay57Selection > 0 &&
        gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection--;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], -0xA0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (D_800D31BC >= 17 && gOverlay57Selection < 3 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection++;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0x1E0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (D_800D31BE < -16 && gOverlay57Selection < 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection += 2;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (D_800D31BE >= 17 && gOverlay57Selection >= 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection -= 2;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    }

    if ((D_800D31B8 & 0x4000) && gOverlay57DistanceState == 0 &&
        gOverlay57ObjectReady != 0) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(1);

        link = gOverlay57StartLinks;
        while (link->index != -1) {
            animseqStartPath(link->index);
            overlay57SetNodeValue(link->index, gOverlay57NodeArguments[link->index], 0.007f);
            link++;
        }
        link = gOverlay57StopLinks;
        while (link->index != -1) {
            animseqStopPath(link->index);
            link++;
        }
    }

    func_overlay_057_F0001020_18A4C18(updateRate);
    if ((D_800D31B8 & 0x9000) && gOverlay57DistanceState == 0) {
        amSndPlay(12, 0);
        count = 10;
        while (count--) {
            activePlayers[count] = 1;
        }
        for (i = 0, index = 0; &gOverlay57MenuSources[index] < &gOverlay57MenuSources[4]; index++) {
            rank = gOverlay57MenuSources[index].active;
            playerOrder[index] = rank;
            if (rank != 0) {
                gOverlay57MenuEntries[i].controller =
                    gOverlay57ControllerMap[gOverlay57MenuSources[index].index];
                activePlayers[gOverlay57ControllerMap[gOverlay57MenuSources[index].index]] = 0;
                i++;
            }
        }
        i = 0;
        for (count = D_8007C1A0; count < 6; count++) {
            while (activePlayers[i] == 0) {
                i++;
            }
            gOverlay57MenuEntries[count].controller = i;
            i++;
        }
        joyCreateMap(playerOrder);
        mainSetMode(0);
        mainChangeCameras(D_8007C1A0);
        gOverlay57MenuEntries[0].type = 5;
        if (D_8007C1A0 == 1) {
            D_8007BEF8 = 4;
            D_8007BEFC = 3;
        } else if (gOverlay57MirrorFlag != 0) {
            D_8007BEF8 = 4;
            D_8007BEFC = 4 - D_8007C1A0;
        } else {
            D_8007BEF8 = D_8007C1A0;
            D_8007BEFC = 0;
        }
        D_8007BF0C = D_8007C1A0 >= 2;
        D_8007BF04 = 2;
        for (i = 0; i < 6; i++) {
            gOverlay57MenuEntries[i].value08 = 0;
        }
        gOverlay57MenuHalf = 0x3FC;
        D_8007BF08 = 0;
        mainChangeLevel(gOverlay57LevelBySelection[gOverlay57Selection],
                        gOverlay57ControllerMap[gOverlay57MenuSources[0].index],
                        0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    }
}

/* DKR v77/v80 and JFG have no exact donor for this node-update routine. */
static void overlay57SetNodeValue(s32 id, s32 argument, f32 value) {
    Overlay57Result *node;

    node = func_800508B4(id);
    if ((node != NULL) && (((Overlay57Target *)node->node) != NULL)) {
        if (value < 0.0f) {
            func_8005AD64(((Overlay57Target *)node->node), argument, -1,
                                        ((Overlay57Target *)node->node)->parameter);
        } else {
            func_8005AD64(((Overlay57Target *)node->node), argument, -1, 0.0f);
        }
        node->valueC = value;
        node->byte14 = 0;
    }
}

/*
 * DKR v77/v80 and JFG object scans have no exact donor for this table
 * dispatcher.  Source searches find only generic five-element loops and DKR
 * vehicle-audio tables; none share this mode-selected apply sequence.
 */
static void overlay57ApplyTable(void) {
    s32 i;

    if (gOverlay57TableMode != 0) {
        for (i = 0; i != 5; i++) {
            overlay57ApplyValue(0x4C, gOverlay57TableLabels[i],
                                gOverlay57TableB[gOverlay57TableIndex][i]);
        }
    } else {
        for (i = 0; i != 5; i++) {
            overlay57ApplyValue(0x4C, gOverlay57TableLabels[i],
                                gOverlay57TableA[gOverlay57TableIndex][i]);
        }
    }
    overlay57ApplyValue(0x4C, 0, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 1, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 2, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 3, gOverlay57TableIndex);
    overlay57UpdateNode();
}

/* DKR v77/v80 and JFG contain only unrelated generic mask/shift idioms. */
static void overlay57UpdateNode(void) {
    register u32 flag;
    void *first;
    Overlay57Result *result;

    flag = (gOverlay57UpdateFlags[gOverlay57UpdateSelection] &
            gOverlay57UpdateMasks[gOverlay57UpdateIndex]) >>
           gOverlay57UpdateShifts[gOverlay57UpdateIndex];
    if (flag == 0) {
        animseqStopPath(0x4D);
        return;
    }
    first = func_8005077C(0x4D);
    animseqStartPath(0x4D);
    result = func_800508B4(0x4D);
    if (result != NULL && result->node != NULL) {
        if (first != NULL) {
            result->valueC = gOverlay57UpdateScale;
            result->byte14 = 0;
            result->node->byte3A = D_8007BF2C;
            func_8005AD64_o057Reloc(result->node, 0, 0, result->node->value28);
        }
        result->node->value88 = gOverlay57UpdateValues[flag];
    }
}
