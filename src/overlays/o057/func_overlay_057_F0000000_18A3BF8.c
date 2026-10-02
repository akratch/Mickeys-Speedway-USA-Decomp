#include "PR/ultratypes.h"
#include "overlays/overlay_045.h"

typedef struct O57ResourceTable {
    void *entries[0x70];
} O57ResourceTable;

typedef struct O57SeedData {
    u8 pad00[0xC];
    s32 value0C;
    s32 value10;
    s32 value14;
    u8 pad18[4];
    s32 value1C;
    u8 pad20[0x1CC];
    s32 value1EC;
    s32 value1F0;
    s32 value1F4;
} O57SeedData;

typedef struct O57AnimObject {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} O57AnimObject;

typedef struct O57SpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
    s32 state;
    f32 scale;
} O57SpawnPacket;

typedef struct O57FinalSpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    u8 byte0A;
    u8 byte0B;
    s8 state;
} O57FinalSpawnPacket;

typedef struct O57SpawnState {
    u8 pad00[8];
    s16 mode;
} O57SpawnState;

typedef struct O57Thing {
    u8 pad00[8];
    O57AnimObject *object;
    u8 pad0C[0xA];
    u8 flags;
    u8 pad17[0x25];
    s32 field3C;
    u8 pad40[0x28];
    O57SpawnState **state;
} O57Thing;

typedef struct O57ModeObject {
    s32 value;
} O57ModeObject;

typedef struct O57Choice {
    u8 pad00[0x2A];
    s8 enabled;
    u8 pad2B[9];
} O57Choice;

typedef struct O57Pair {
    s16 first;
    s16 second;
} O57Pair;

extern void *gO57Current100Reloc;
extern s32 gO57Pending104Reloc;
extern void *gO57Previous108Reloc;
extern void *gO57Runtime1B8Reloc;
extern void *gO57ResidentCurrentReloc;
extern O57ResourceTable *gO57ResourceTableReloc;

extern Overlay45ResourceDescriptor *gO57Descriptor00Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor00PrepareReloc;
extern Overlay45ResourceDescriptor *gO57DescriptorF8Reloc;
extern Overlay45ResourceDescriptor *gO57DescriptorF8PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor6CReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor6CPrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor70Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor70PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor74Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor74PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor78Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor78PrepareReloc;
extern Overlay45ResourceDescriptor *gO57DescriptorFCReloc;
extern Overlay45ResourceDescriptor *gO57Descriptors08Reloc[9];
extern Overlay45ResourceDescriptor *gO57Descriptors30Reloc[9];
extern Overlay45ResourceDescriptor *gO57Descriptors80Reloc[24];
extern Overlay45ResourceDescriptor *gO57DescriptorsE0Reloc[6];
extern Overlay45ResourceDescriptor *gO57Descriptors58Reloc[5];
extern Overlay45ResourceDescriptor *gO57DescriptorList80Reloc;
extern s16 gO57DescriptorIndicesD8Reloc[9];
extern s16 gO57DescriptorIndicesECReloc[9];
extern s16 gO57DescriptorIndices100Reloc[24];
extern s16 gO57DescriptorIndices130Reloc[6];
extern s16 gO57DescriptorList130Reloc;

extern O57SeedData gO57SeedDataReloc;
extern s32 gO57Value1FCReloc;
extern s32 gO57Value21CReloc;
extern s32 gO57Value23CReloc;
extern s32 gO57Value25CReloc;
extern s32 gO57Value32CReloc;
extern s32 gO57Value34CReloc;
typedef struct O57Row {
    s32 value;
    u8 pad04[0xC];
} O57Row;
extern O57Row gO57Rows27CReloc[10];

extern s32 gO57Mode11CReloc;
extern s32 gO57Value128Reloc;
extern s32 gO57Value12CReloc;
extern s32 gO57Ids134Reloc[];
extern s32 gO57Value138Reloc;
extern s32 D_134;
extern s32 gO57Value13CReloc;
extern s32 gO57Active144Reloc;
extern s32 gO57Value148Reloc;
extern s32 gO57Value14CReloc;
extern s32 gO57Value160Reloc;
extern s32 gO57Value164Reloc;
extern s32 gO57Value50CReloc;
extern s32 gO57NodeValues17CReloc[];
extern s16 gO57ColourCycle1A8Reloc;

extern s32 gO57State118Reloc;
extern O57ModeObject gO57ModeObject180Reloc;
extern s32 gO57SpecialModeReloc;
extern u8 gO57SpecialByteReloc;
extern s32 gO57SpecialConditionReloc;
extern O57Choice gO57ChoicesReloc[4];
extern u8 gO57ChoiceMaskReloc;
extern u8 gO57ChoiceMaskReferenceReloc;
extern u8 gO57ChoiceSourceReloc;
extern s32 gO57ChoiceDirtyReloc;
extern u8 gO57ChoicePublishedReloc;

extern s32 gO57Value188Reloc;
extern s32 gO57Value18CReloc;
extern s32 gO57Value190Reloc;
extern u16 gO57ResidentFlags10Reloc;
extern s32 gO57Value198Reloc;
extern s32 gO57Value19CReloc;
extern f32 gO57Value110Reloc;
extern s32 gO57Value124Reloc;
extern O57Pair gO57SpawnPairs3E8Reloc[4];
extern O57Thing *gO57Spawned150Reloc[4];

extern void *func_80028F54(void);
extern void func_8004B0A4(s32 font);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void o57PrepareDescriptorReloc(Overlay45ResourceDescriptor *descriptor,
                                      s32 value);
extern void o57PrepareDescriptorListReloc(void *list);
extern void animseqStartPath(u8 pathId);
extern O57Thing *func_800508B4(u8 pathId);
extern O57Thing *func_8000590C(void *packet, s32 mode);
extern void initColourCycle(void *cycle, s32 count);
extern void joyResetMap(void);
extern s32 o57QueryModeReloc(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void o57PublishChoicesReloc(void);
extern void func_8003A754(void);
extern void func_8005AD64(O57Thing *spawned, s32 mode, s32 index,
                          f32 value);
/* Overlay 45 entry points, reached through the relocation surface. */
extern Overlay45ResourceDescriptor *overlay45CreateDescriptor_o057Reloc(
    const char *text, s16 width, s16 height, s32 flags);
extern void overlay45SetMode_o057Reloc(Overlay45ResourceDescriptor *descriptor,
                                       s32 mode);

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
    func_8004B0A4(3);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    gO57Descriptor00Reloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x40], 0xA0, -0x28, 4);
    gO57DescriptorF8Reloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x3C], 0xA0, -0x28, 4);
    gO57Descriptor6CReloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x41], 0xA0, 0x104, 4);
    gO57Descriptor70Reloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x42], 0xA0, 0x104, 4);
    gO57Descriptor74Reloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x43], 0xA0, 0x104, 4);
    gO57Descriptor78Reloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x44], 0xA0, 0x104, 4);
    gO57DescriptorFCReloc = overlay45CreateDescriptor_o057Reloc(
        gO57ResourceTableReloc->entries[0x6D], 0xA0, -0x20, 4);

    for (i = 0; i < 9; i++) {
        gO57Descriptors08Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            gO57ResourceTableReloc->entries[gO57DescriptorIndicesD8Reloc[i]], 0xA0, 0xBE,
            0x204);
        overlay45SetMode_o057Reloc(gO57Descriptors08Reloc[i], 0);
    }

    for (i = 0; i < 9; i++) {
        gO57Descriptors30Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            gO57ResourceTableReloc->entries[gO57DescriptorIndicesECReloc[i]], 0xA0, 0xBE,
            0x204);
        overlay45SetMode_o057Reloc(gO57Descriptors30Reloc[i], 0);
    }

    for (i = 0; i < 24; i++) {
        gO57Descriptors80Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            gO57ResourceTableReloc->entries[gO57DescriptorIndices100Reloc[i]], 0xA0, 0x104, 4);
        overlay45SetMode_o057Reloc(gO57Descriptors80Reloc[i], 0);
    }

    for (i = 0; i < 6; i++) {
        gO57DescriptorsE0Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            gO57ResourceTableReloc->entries[gO57DescriptorIndices130Reloc[i]], 0xA0, 0x104, 4);
        overlay45SetMode_o057Reloc(gO57DescriptorsE0Reloc[i], 0);
    }

    for (i = 0; i < 5; i++) {
        gO57Descriptors58Reloc[i] = overlay45CreateDescriptor_o057Reloc(
            gO57ResourceTableReloc->entries[0x36 + i], 0xA0, -0x28, 4);
    }

    o57PrepareDescriptorReloc(gO57Descriptor00PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57DescriptorF8PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor6CPrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor70PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor74PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor78PrepareReloc, 0xFF);
    o57PrepareDescriptorListReloc(&gO57DescriptorList80Reloc);
    o57PrepareDescriptorListReloc(&gO57DescriptorList130Reloc);

    gO57Value1FCReloc = gO57SeedDataReloc.value0C;
    gO57Value21CReloc = gO57SeedDataReloc.value10;
    gO57Value23CReloc = gO57SeedDataReloc.value14;
    gO57Value25CReloc = gO57SeedDataReloc.value10;
    gO57Value32CReloc = gO57SeedDataReloc.value1EC;
    gO57Value34CReloc = gO57SeedDataReloc.value1F4;
    for (i = 0; i < 10; i++) {
        gO57Rows27CReloc[i].value = gO57SeedDataReloc.value1C;
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

    switch (o57QueryModeReloc()) {
    case 4:
        gO57State118Reloc = 0xB;
        gO57ModeObject180Reloc.value = 0x2E;
        animseqStartPath(((u8 *)&gO57ModeObject180Reloc)[3]);
        object = func_800508B4(((u8 *)&gO57ModeObject180Reloc)[3]);
        if (object != 0) {
            object->flags |= 2;
        }
        gO57SpecialModeReloc = 0;
        gO57SpecialByteReloc = 1;
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
        if (gO57SpecialConditionReloc == 1) {
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
        o57PublishChoicesReloc();
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
    func_8003A754();
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
