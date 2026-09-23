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

extern O35LoaderState *D_o35_loader_state;
extern s32 D_o35_allocation_count;
extern void **D_o35_allocations;
extern s16 *D_o35_values_a;
extern s16 *D_o35_values_b;
extern void *D_o35_segment_storage;
extern s32 D_o35_reset;
extern void *D_o35_model_heap;
extern O35Model *D_o35_current_model;
extern s32 *D_o35_model_table;
extern s32 D_o35_final_reset;

extern void *call_o0_0_2AE30(s32, s32);
extern void call_o0_0_29DF4(void);
extern void call_o0_0_26934(void);
extern s32 *call_o0_0_2DCF8(s32);
extern void call_o0_0_2DE90(s32, void *, s32, s32);
extern void call_o0_0_4D390(void *, void *);
extern void call_o0_0_2B318(void *);
extern void *call_o0_0_2B848(void *);
extern void *call_o0_0_2B810(void *);
extern s32 call_o0_0_2B6E4(void);
extern void call_o0_0_2B2A4(s32);
extern void *call_o0_0_2B0D4(s32, void *, s32);
extern void *call_o0_0_33FF8(s32);
extern void call_o0_0_CF68(u8, u8);

extern void func_overlay_035_F0000770_1882450(O35Segment *, O35Bounds *,
                                                O35Segment *);
extern s32 func_overlay_035_F0000B40_1882820(O35Segment *);
extern void func_overlay_035_F0001380_1883060(O35Segment *);

/* Matched by spelling every model access as D_o35_current_model (DKR style) so
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
    s32 temp_s4;
    s32 temp;
    O35Segment *segment;
    u8 *cursor;

    D_o35_allocation_count = D_o35_loader_state->allocationCount;
    D_o35_allocations = call_o0_0_2AE30(D_o35_allocation_count * 4, 0x91);
    D_o35_values_a = call_o0_0_2AE30(D_o35_allocation_count * 2, 0x91);
    D_o35_values_b = call_o0_0_2AE30(D_o35_allocation_count * 2, 0x91);
    D_o35_segment_storage = call_o0_0_2AE30(
        D_o35_loader_state->segmentAllocationCount * 0x2C, 0x91);
    D_o35_reset = 0;
    D_o35_model_heap = call_o0_0_2AE30(0x9F000, 0x91);
    D_o35_current_model = D_o35_model_heap;
    if (D_o35_current_model == 0) {
        call_o0_0_29DF4();
    }
    call_o0_0_26934();

    D_o35_model_table = call_o0_0_2DCF8(0x24);
    for (i = 0; D_o35_model_table[i] != -1; i++) {
    }
    i--;
    if (modelId >= i) {
        modelId = 0;
    }

    mdl = D_o35_model_table[modelId];
    temp_s4 = D_o35_model_table[modelId + 1] - mdl;
    temp = (s32)D_o35_current_model;
    temp += 0x9F000 - temp_s4;
    temp -= temp % 16;

    call_o0_0_26934();
    call_o0_0_2DE90(0x25, (void *)temp, mdl, temp_s4);
    call_o0_0_4D390((void *)temp, D_o35_current_model);
    call_o0_0_2B318(D_o35_model_table);
    call_o0_0_26934();

    mdl = (s32)D_o35_current_model;
    D_o35_current_model->textures =
        (O35TextureInfo *)((s32)D_o35_current_model->textures + mdl);
    D_o35_current_model->segments =
        (O35Segment *)((s32)D_o35_current_model->segments + mdl);
    D_o35_current_model->bounds =
        (O35Bounds *)((s32)D_o35_current_model->bounds + mdl);
    D_o35_current_model->unkC = (void *)((s32)D_o35_current_model->unkC + mdl);
    D_o35_current_model->unk10 =
        (void *)((s32)D_o35_current_model->unk10 + mdl);
    D_o35_current_model->unk14 =
        (void *)((s32)D_o35_current_model->unk14 + mdl);

    for (k = 0; k < D_o35_current_model->segmentCount; k++) {
        D_o35_current_model->segments[k].vertices =
            (u8 *)((s32)D_o35_current_model->segments[k].vertices + mdl);
        D_o35_current_model->segments[k].triangles =
            (void *)((s32)D_o35_current_model->segments[k].triangles + mdl);
        D_o35_current_model->segments[k].groups =
            (void *)((s32)D_o35_current_model->segments[k].groups + mdl);
        D_o35_current_model->segments[k].unk18 =
            (void *)((s32)D_o35_current_model->segments[k].unk18 + mdl);
    }

    cursor = (u8 *)D_o35_current_model + D_o35_current_model->modelSize;
    for (k = 0; k < D_o35_current_model->segmentCount; k++) {
        cursor = call_o0_0_2B848(cursor);
        D_o35_current_model->segments[k].masks = (u32 *)cursor;
        cursor += D_o35_current_model->segments[k].dataOffset * 4;
        D_o35_current_model->segments[k].collisionFacets = cursor;
        cursor += D_o35_current_model->segments[k].dataOffset;
        func_overlay_035_F0000770_1882450(
            &D_o35_current_model->segments[k],
            &D_o35_current_model->bounds[k],
            D_o35_current_model->segments);
        cursor = call_o0_0_2B810(cursor);
        D_o35_current_model->segments[k].collisionPlanes = (f32 *)cursor;
        cursor += func_overlay_035_F0000B40_1882820(
                      &D_o35_current_model->segments[k]) *
                  0x10;
        D_o35_current_model->segments[k].selectedValue = 0;
        func_overlay_035_F0001380_1883060(
            &D_o35_current_model->segments[k]);
    }

    cursor = call_o0_0_2B810(cursor);
    temp_s4 = cursor - (u8 *)D_o35_current_model;
    call_o0_0_26934();
    temp = call_o0_0_2B6E4();
    call_o0_0_2B2A4(0);
    call_o0_0_2B318(D_o35_model_heap);
    call_o0_0_2B0D4(temp_s4, D_o35_model_heap, 0x91);
    call_o0_0_2B2A4(temp);
    call_o0_0_26934();

    for (k = 0; k < D_o35_current_model->textureCount; k++) {
        D_o35_current_model->textures[k].texture = call_o0_0_33FF8(
            (s32)D_o35_current_model->textures[k].texture | 0x8000);
    }
    call_o0_0_26934();

    segment = D_o35_current_model->segments;
    for (k = 0; k < D_o35_current_model->segmentCount; k++) {
        O35ColorData *colorData;
        u8 *colors;
        s16 *flags;

        j = segment->count20;
        temp_s4 = ((j + 0xF) >> 4) * 2;
        colorData = call_o0_0_2AE30(temp_s4 + 8 + j * 3, 0x91);
        if (colorData != 0) {
            s32 remaining;
            u8 *source;

            segment->colorData = colorData;
            colorData->flags = (s16 *)((u8 *)colorData + 8);
            colorData->colors = (u8 *)colorData + temp_s4 + 8;
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
            remaining = temp_s4 >> 1;
            while (remaining--) {
                *flags++ = 0;
            }
            segment->flag2E = 0;
        }
        segment++;
    }
    call_o0_0_26934();
    D_o35_final_reset = 0;
    call_o0_0_CF68(D_o35_loader_state->finalArg0,
                   D_o35_loader_state->finalArg1);
}
