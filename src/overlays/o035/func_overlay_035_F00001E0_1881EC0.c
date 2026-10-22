#include "ultra64.h"

typedef struct O35TextureInfo {
    void *texture;
    u8 pad4[4];
} O35TextureInfo;

typedef struct O35Segment {
    u8 *vertices;
    void *triangles;
    void *unk8;
    void *groups;
    u32 *masks;
    void *collisionFacets;
    void *unk18;
    f32 *collisionPlanes;
    s16 count20;
    s16 dataOffset;
    u8 pad24[0xA];
    u8 flag2E;
    u8 pad2F;
    void *colorData;
    u8 pad34[4];
    s32 selectedValue;
    u8 pad3C[4];
} O35Segment;

typedef struct O35Bounds {
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
} O35Bounds;

typedef struct O35Model {
    O35TextureInfo *textures;
    O35Segment *segments;
    O35Bounds *bounds;
    void *unkC;
    void *unk10;
    void *unk14;
    s16 textureCount;
    s16 segmentCount;
    u8 pad1C[0x10];
    s32 modelSize;
} O35Model;

typedef struct O35LoaderState {
    u8 pad0[0xEE];
    s16 allocationCount;
    s16 segmentAllocationCount;
    u8 padF2[0x17];
    u8 finalArg0;
    u8 finalArg1;
} O35LoaderState;

typedef struct O35ColorData {
    u8 *colors;
    s16 *flags;
} O35ColorData;

extern O35LoaderState *D_800792EC;
extern s32 D_800C9D38;
extern void **D_800C9D2C;
extern s16 *D_800C9D30;
extern s16 *D_800C9D34;
extern void *D_800C9D20;
extern s32 D_800C9D3C;
extern void *D_800C95A8;
extern O35Model *D_800792E8;
extern s32 *D_o35_model_table;
extern s32 D_80079314;

extern void *mmAlloc(s32, s32);
extern void func_8002A244(void);
extern void mainPreNMI(void);
extern s32 *piRomLoad(s32);
extern void piRomLoadSection(s32, void *, s32, s32);
extern void func_8004D7E0(void *, void *);
extern void mmFree(void *);
extern void *align4(void *);
extern void *align16(void *);
extern s32 mmGetDelay(void);
extern void mmSetDelay(s32);
extern void *mmAllocAtAddr(s32, void *, s32);
extern void *texLoadTexture(s32);
extern void trackLightAllocate(u8, u8);

extern void func_overlay_035_F0000770_1882450(O35Segment *, O35Bounds *,
                                                O35Segment *);
extern s32 func_overlay_035_F0000B40_1882820(O35Segment *);
extern void func_overlay_035_F0001380_1883060(O35Segment *);

/* Matched by spelling every model access as D_800792E8 (DKR style) so
 * uopt CSEs the loop-condition reloads, testing the heap through the current
 * model, walking D_o35_model_table directly (its address web then outranks
 * 0x9F000 for s3), post-decrement copy loops (the target's dead count copy),
 * and reading the walkers back through the stored colorData fields.
 * The unreferenced pad keeps mdl's home at sp+0x38 (L99).
 * PROVENANCE: adapted from Diddy Kong Racing, src/tracks.c (generate_track). */
void func_overlay_035_F00001E0_1881EC0(s32 modelId) {
    void *unusedPad;
    s32 mdl;
    s32 i;
    s32 j;
    s32 k;
    s32 byteCount;
    s32 temp;
    O35Segment *segment;
    u8 *cursor;

    D_800C9D38 = D_800792EC->allocationCount;
    D_800C9D2C = mmAlloc(D_800C9D38 * 4, 0x91);
    D_800C9D30 = mmAlloc(D_800C9D38 * 2, 0x91);
    D_800C9D34 = mmAlloc(D_800C9D38 * 2, 0x91);
    D_800C9D20 = mmAlloc(
        D_800792EC->segmentAllocationCount * 0x2C, 0x91);
    D_800C9D3C = 0;
    D_800C95A8 = mmAlloc(0x9F000, 0x91);
    D_800792E8 = D_800C95A8;
    if (D_800792E8 == 0) {
        func_8002A244();
    }
    mainPreNMI();

    D_o35_model_table = piRomLoad(0x24);
    for (i = 0; D_o35_model_table[i] != -1; i++) {
    }
    i--;
    if (modelId >= i) {
        modelId = 0;
    }

    mdl = D_o35_model_table[modelId];
    byteCount = D_o35_model_table[modelId + 1] - mdl;
    temp = (s32)D_800792E8;
    temp += 0x9F000 - byteCount;
    temp -= temp % 16;

    mainPreNMI();
    piRomLoadSection(0x25, (void *)temp, mdl, byteCount);
    func_8004D7E0((void *)temp, D_800792E8);
    mmFree(D_o35_model_table);
    mainPreNMI();

    mdl = (s32)D_800792E8;
    D_800792E8->textures =
        (O35TextureInfo *)((s32)D_800792E8->textures + mdl);
    D_800792E8->segments =
        (O35Segment *)((s32)D_800792E8->segments + mdl);
    D_800792E8->bounds =
        (O35Bounds *)((s32)D_800792E8->bounds + mdl);
    D_800792E8->unkC = (void *)((s32)D_800792E8->unkC + mdl);
    D_800792E8->unk10 =
        (void *)((s32)D_800792E8->unk10 + mdl);
    D_800792E8->unk14 =
        (void *)((s32)D_800792E8->unk14 + mdl);

    for (k = 0; k < D_800792E8->segmentCount; k++) {
        D_800792E8->segments[k].vertices =
            (u8 *)((s32)D_800792E8->segments[k].vertices + mdl);
        D_800792E8->segments[k].triangles =
            (void *)((s32)D_800792E8->segments[k].triangles + mdl);
        D_800792E8->segments[k].groups =
            (void *)((s32)D_800792E8->segments[k].groups + mdl);
        D_800792E8->segments[k].unk18 =
            (void *)((s32)D_800792E8->segments[k].unk18 + mdl);
    }

    cursor = (u8 *)D_800792E8 + D_800792E8->modelSize;
    for (k = 0; k < D_800792E8->segmentCount; k++) {
        cursor = align4(cursor);
        D_800792E8->segments[k].masks = (u32 *)cursor;
        cursor += D_800792E8->segments[k].dataOffset * 4;
        D_800792E8->segments[k].collisionFacets = cursor;
        cursor += D_800792E8->segments[k].dataOffset;
        func_overlay_035_F0000770_1882450(
            &D_800792E8->segments[k],
            &D_800792E8->bounds[k],
            D_800792E8->segments);
        cursor = align16(cursor);
        D_800792E8->segments[k].collisionPlanes = (f32 *)cursor;
        cursor += func_overlay_035_F0000B40_1882820(
                      &D_800792E8->segments[k]) *
                  0x10;
        D_800792E8->segments[k].selectedValue = 0;
        func_overlay_035_F0001380_1883060(
            &D_800792E8->segments[k]);
    }

    cursor = align16(cursor);
    byteCount = cursor - (u8 *)D_800792E8;
    mainPreNMI();
    temp = mmGetDelay();
    mmSetDelay(0);
    mmFree(D_800C95A8);
    mmAllocAtAddr(byteCount, D_800C95A8, 0x91);
    mmSetDelay(temp);
    mainPreNMI();

    for (k = 0; k < D_800792E8->textureCount; k++) {
        D_800792E8->textures[k].texture = texLoadTexture(
            (s32)D_800792E8->textures[k].texture | 0x8000);
    }
    mainPreNMI();

    segment = D_800792E8->segments;
    for (k = 0; k < D_800792E8->segmentCount; k++) {
        O35ColorData *colorData;
        u8 *colors;
        s16 *flags;

        j = segment->count20;
        byteCount = ((j + 0xF) >> 4) * 2;
        colorData = mmAlloc(byteCount + 8 + j * 3, 0x91);
        if (colorData != 0) {
            s32 remaining;
            u8 *source;

            segment->colorData = colorData;
            colorData->flags = (s16 *)((u8 *)colorData + 8);
            colorData->colors = (u8 *)colorData + byteCount + 8;
            remaining = segment->count20;
            flags = colorData->flags;
            colors = colorData->colors;
            source = segment->vertices;
            while (remaining--) {
                *colors++ = source[6];
                *colors++ = source[7];
                *colors++ = source[8];
                source += 0xA;
            }
            remaining = byteCount >> 1;
            while (remaining--) {
                *flags++ = 0;
            }
            segment->flag2E = 0;
        }
        segment++;
    }
    mainPreNMI();
    D_80079314 = 0;
    trackLightAllocate(D_800792EC->finalArg0,
                   D_800792EC->finalArg1);
}
