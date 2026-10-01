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

typedef struct O57AnimPath {
    u8 pad00[8];
    O57AnimObject *object;
} O57AnimPath;

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

typedef struct O57Spawned {
    u8 pad00[0x3C];
    s32 field3C;
    u8 pad40[0x28];
    O57SpawnState **state;
} O57Spawned;

typedef struct O57ModeObject {
    s32 value;
} O57ModeObject;

typedef struct O57ModeResult {
    u8 pad00[0x16];
    u8 flags;
} O57ModeResult;

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
extern Overlay45ResourceDescriptor *gO57Descriptors2CEndReloc[];
extern Overlay45ResourceDescriptor *gO57Descriptors30Reloc[9];
extern Overlay45ResourceDescriptor *gO57Descriptors54EndReloc[];
extern Overlay45ResourceDescriptor *gO57Descriptors80Reloc[24];
extern Overlay45ResourceDescriptor *gO57DescriptorsE0EndReloc[];
extern Overlay45ResourceDescriptor *gO57DescriptorsE0Reloc[6];
extern Overlay45ResourceDescriptor *gO57DescriptorsF8EndReloc[];
extern Overlay45ResourceDescriptor *gO57Descriptors58Reloc[5];
extern Overlay45ResourceDescriptor *gO57Descriptors6CEndReloc[];
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
extern s32 gO57Value27CReloc;
extern s32 gO57Value28CReloc;
extern s32 gO57Values29CReloc[32];

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
extern O57Spawned *gO57Spawned150Reloc[4];

extern void *func_80028F54(void);
extern void func_8004B0A4(s32 font);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void o57PrepareDescriptorReloc(Overlay45ResourceDescriptor *descriptor,
                                      s32 value);
extern void o57PrepareDescriptorListReloc(void *list);
extern void animseqStartPath(u8 pathId);
extern O57AnimPath *func_800508B4(u8 pathId);
extern O57Spawned *func_8000590C(void *packet, s32 mode);
extern void initColourCycle(void *cycle, s32 count);
extern void joyResetMap(void);
extern s32 o57QueryModeReloc(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void o57PublishChoicesReloc(void);
extern void func_8003A754(void);
extern void func_8005AD64(O57Spawned *spawned, s32 mode, s32 index,
                          f32 value);

/* Overlay 57 text +0x0..+0x954, the module initializer.
 *
 * 2026-10-01 (lane d-o057): 276 masked at size delta -20 -> 100 at -8 by
 * rewriting inherited shape. The 0x134/0x138 stores are the bss fog pair
 * (D_134), not the data id list the switch walks (the relocation records
 * separate the two sections); the spawn packets are two locals, not a union,
 * and the final one stores z; the id walks are `while (*entry != -1)` reading
 * the entry at each use with the f32 prototype; the choice mask is a plain
 * `|=`; func_8005AD64's last argument is a float zero; the initial packet
 * assigns scale before kind and state.
 *
 * Open: the -8 is the final spawn loop. The target keeps `i` live in s1 and
 * indexes the pair table from it each pass while walking only the spawned
 * array; uopt here strength-reduces both and drops `i` (loop form and the
 * spawned carrier measured flat). The frame is 0x88 against 0x78: two
 * reserved cells below the final packet that no declared local accounts for. */
#ifdef NON_MATCHING
void func_overlay_057_F0000000_18A3BF8(void) {
    u8 choiceMask;
    O57FinalSpawnPacket final;
    Overlay45ResourceDescriptor **descriptor;
    s16 *resourceIndex;
    Overlay45ResourceDescriptor **descriptorEnd;
    s32 i;
    O57SpawnPacket packet;
    s32 value;
    s32 stride;
    O57Spawned *spawned;
    O57AnimPath *path;
    O57ModeResult *result;
    s32 *entry;
    s32 id;

    gO57Current100Reloc = gO57ResidentCurrentReloc;
    gO57Pending104Reloc = 0;
    choiceMask = 0;
    gO57Previous108Reloc = gO57ResidentCurrentReloc;
    gO57Runtime1B8Reloc = func_80028F54();
    func_8004B0A4(3);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);


    gO57Descriptor00Reloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x40], 0xA0, -0x28, 4);
    gO57DescriptorF8Reloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x3C], 0xA0, -0x28, 4);
    gO57Descriptor6CReloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x41], 0xA0, 0x104, 4);
    gO57Descriptor70Reloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x42], 0xA0, 0x104, 4);
    gO57Descriptor74Reloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x43], 0xA0, 0x104, 4);
    gO57Descriptor78Reloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x44], 0xA0, 0x104, 4);
    gO57DescriptorFCReloc = overlay45CreateDescriptor(
        gO57ResourceTableReloc->entries[0x6D], 0xA0, -0x20, 4);

    resourceIndex = gO57DescriptorIndicesD8Reloc;
    descriptor = gO57Descriptors08Reloc;
    descriptorEnd = gO57Descriptors2CEndReloc;
    do {
        *descriptor = overlay45CreateDescriptor(
            gO57ResourceTableReloc->entries[*resourceIndex], 0xA0, 0xBE,
            0x204);
        overlay45SetMode(*descriptor, 0);
        descriptor++;
        resourceIndex++;
    } while (descriptor < descriptorEnd);

    resourceIndex = gO57DescriptorIndicesECReloc;
    descriptor = gO57Descriptors30Reloc;
    descriptorEnd = gO57Descriptors54EndReloc;
    do {
        *descriptor = overlay45CreateDescriptor(
            gO57ResourceTableReloc->entries[*resourceIndex], 0xA0, 0xBE,
            0x204);
        overlay45SetMode(*descriptor, 0);
        descriptor++;
        resourceIndex++;
    } while (descriptor < descriptorEnd);

    resourceIndex = gO57DescriptorIndices100Reloc;
    descriptor = gO57Descriptors80Reloc;
    descriptorEnd = gO57DescriptorsE0EndReloc;
    do {
        *descriptor = overlay45CreateDescriptor(
            gO57ResourceTableReloc->entries[*resourceIndex], 0xA0, 0x104, 4);
        overlay45SetMode(*descriptor, 0);
        descriptor++;
        resourceIndex++;
    } while (descriptor < descriptorEnd);

    resourceIndex = gO57DescriptorIndices130Reloc;
    descriptor = gO57DescriptorsE0Reloc;
    descriptorEnd = gO57DescriptorsF8EndReloc;
    do {
        *descriptor = overlay45CreateDescriptor(
            gO57ResourceTableReloc->entries[*resourceIndex], 0xA0, 0x104, 4);
        overlay45SetMode(*descriptor, 0);
        descriptor++;
        resourceIndex++;
    } while (descriptor < descriptorEnd);

    descriptor = gO57Descriptors58Reloc;
    descriptorEnd = gO57Descriptors6CEndReloc;
    i = 0;
    do {
        *descriptor = overlay45CreateDescriptor(
            gO57ResourceTableReloc->entries[0x36 + i], 0xA0, -0x28, 4);
        descriptor++;
        i++;
    } while (descriptor < descriptorEnd);

    o57PrepareDescriptorReloc(gO57Descriptor00PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57DescriptorF8PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor6CPrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor70PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor74PrepareReloc, 0xFF);
    o57PrepareDescriptorReloc(gO57Descriptor78PrepareReloc, 0xFF);
    o57PrepareDescriptorListReloc(&gO57DescriptorList80Reloc);
    o57PrepareDescriptorListReloc(&gO57DescriptorList130Reloc);


    gO57Value1FCReloc = gO57SeedDataReloc.value0C;
    value = gO57SeedDataReloc.value10;
    gO57Value21CReloc = value;
    gO57Value23CReloc = gO57SeedDataReloc.value14;
    gO57Value25CReloc = value;
    gO57Value32CReloc = gO57SeedDataReloc.value1EC;
    gO57Value34CReloc = gO57SeedDataReloc.value1F4;
    value = gO57SeedDataReloc.value1C;
    gO57Value27CReloc = value;
    gO57Value28CReloc = value;
    for (stride = 0; stride < 32; stride += 4) {
        gO57Values29CReloc[stride] = value;
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

    path = func_800508B4(0x3C);
    if (path->object != 0) {
        packet.mode = 0x14;
        packet.flags = 0;
        packet.x = path->object->x;
        packet.y = path->object->y;
        packet.z = path->object->z;
        packet.angle = path->object->angle;
        packet.scale = path->object->scale;
        packet.kind = 0x35;
        packet.state = 0;
        spawned = func_8000590C(&packet, 1);
        if (spawned != 0) {
            spawned->field3C = 0;
        }
        packet.kind = 0x38;
        packet.state = 1;
        spawned = func_8000590C(&packet, 1);
        if (spawned != 0) {
            spawned->field3C = 0;
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
        result = (O57ModeResult *)func_800508B4(
            ((u8 *)&gO57ModeObject180Reloc)[3]);
        if (result != 0) {
            result->flags |= 2;
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
        result = (O57ModeResult *)func_800508B4(
            ((u8 *)&gO57ModeObject180Reloc)[3]);
        if (result != 0) {
            result->flags |= 2;
        }
        gO57SpecialModeReloc = 5;
        break;
    case 18:
        gO57State118Reloc = 0x14;
        gO57ModeObject180Reloc.value = 0x54;
        animseqStartPath(((u8 *)&gO57ModeObject180Reloc)[3]);
        result = (O57ModeResult *)func_800508B4(
            ((u8 *)&gO57ModeObject180Reloc)[3]);
        if (result != 0) {
            result->flags |= 2;
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
    if ((gO57ChoiceMaskReloc != choiceMask) ||
        (gO57ChoiceMaskReferenceReloc != gO57ChoiceSourceReloc)) {
        o57PublishChoicesReloc();
        gO57ChoiceDirtyReloc = 0;
        gO57ChoiceMaskReloc = choiceMask;
        gO57ChoicePublishedReloc = 1;
        gO57ChoiceMaskReferenceReloc = gO57ChoiceSourceReloc;
    }

    gO57Value188Reloc = 0;
    gO57Value18CReloc =
        ((((gO57ResidentFlags10Reloc & 0x1C0) >> 6) < 3) ^ 1);
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


    i = 0;
    do {
        final.kind = 0x138;
        final.mode = 0xE;
        final.z = 0;
        final.byte0A = 0;
        final.state = 0;
        final.byte0B = 0x80;
        final.x = gO57SpawnPairs3E8Reloc[i].first;
        final.y = gO57SpawnPairs3E8Reloc[i].second;
        spawned = func_8000590C(&final, 0);
        gO57Spawned150Reloc[i] = spawned;
        (*spawned->state)->mode = 2;
        func_8005AD64(spawned, 0, 0, 0.0f);
        i++;
    } while (i != 4);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0000000_18A3BF8/func_overlay_057_F0000000_18A3BF8.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_057_F0000000_18A3BF8:start
 * symbol: func_overlay_057_F0000000_18A3BF8
 * score: 100/597 words
 * frame: 0x88
 * relocations: 246
 * first-mismatch: +0x8
 * summary: 100 at -8: target keeps i live in s1 in the final spawn loop, uopt here strength-reduces both arrays; frame 0x88 vs 0x78.
 * PLATEAU-HANDOFF:func_overlay_057_F0000000_18A3BF8:end
 */
