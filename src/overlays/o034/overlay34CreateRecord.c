#include "PR/ultratypes.h"

typedef struct Overlay34Input {
    s16 angleX;
    s16 angleY;
    f32 depth;
    f32 x;
    f32 y;
    f32 z;
    f32 value;
    s16 resourceId;
    u8 mode;
    u8 pad1B;
    s32 word1C;
    s32 word20;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
} Overlay34Input;

typedef struct Overlay34Resource {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay34Resource;

typedef struct Overlay34Record {
    u8 byte00;
    u8 byte01;
    u8 byte02;
    u8 byte03;
    s16 short04;
    s16 short06;
    s16 short08;
    s16 short0A;
    s16 short0C;
    s16 short0E;
    u8 byte10;
    u8 byte11;
    u8 byte12;
    u8 byte13;
    s16 short14;
    s16 short16;
    s16 short18;
    s16 short1A;
    s16 short1C;
    s16 short1E;
    Overlay34Resource *resource;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
    s32 word34;
    s32 word38;
    u8 byte3C;
    u8 byte3D;
    u8 active;
    u8 pad3F;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 direction[2];
    f32 depth;
    f32 value;
} Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern s32 gOverlay34Count;
extern Overlay34Resource *func_80034448(s16 resourceId);
extern void func_80029FE4(Overlay34Input *input, f32 direction[3]);

/* Matched 2026-09-17, lane w3-o034. 125/125 words, frame 0x30, 12 relocs.
 * uopt closes a straight-line block after twenty cfe Ulod of non-veqv isvars.
 * Stores through the declared `candidate` spent that budget at byte12, so
 * short16 opened the next block and the pair could not share the ROM order.
 * Rebinding `record = candidate` after the resource call gives the fill
 * pointer an assigned tree with no ILOD; forwarding then skips the base
 * Ulods, the whole fill including short16 and byte12 stays in one block,
 * and height (one block, save 4) outranks the resource copy (save 3) without
 * the self-defs that previously bought that ranking. */
Overlay34Record *overlay34CreateRecord(Overlay34Input *input) {
    Overlay34Record *record;
    Overlay34Record *current;
    s32 width;
    s32 height;
    Overlay34Record *candidate;
    s32 index;

    candidate = NULL;
    if (gOverlay34ActiveCount < gOverlay34Count) {
        if (gOverlay34Count > 0) {
            index = 0;
            do {
                record = &gOverlay34Records[index];
                current = record;
                if (current->active == 0) {
                    candidate = current;
                    break;
                }
                index++;
            } while (index < gOverlay34Count);
        }
        if (candidate != NULL) {
            candidate->resource = func_80034448(input->resourceId);
            if (candidate->resource != NULL) {
                record = candidate;
                width = (record->resource->width - 1) << 5;
                height = (record->resource->height - 1) << 5;
                record->byte00 = 0x40;
                record->byte01 = 0;
                record->short04 = width;
                record->short06 = 0;
                record->byte02 = 1;
                record->short08 = width;
                record->short0A = height;
                record->byte03 = 2;
                record->short0C = 0;
                record->short0E = 0;
                record->byte10 = 0x40;
                record->byte11 = 1;
                record->short14 = width;
                record->short16 = height;
                record->byte12 = 2;
                record->short18 = 0;
                record->short1A = 0;
                record->byte13 = 3;
                record->short1C = 0;
                record->short1E = height;
                record->word24 = input->word1C;
                record->word28 = input->word28;
                record->word2C = input->word20;
                record->word30 = input->word2C;
                record->word34 = input->word24;
                record->word38 = input->word30;
                record->byte3C = 0;
                record->byte3D = input->mode * 6;
                record->x1 = input->x;
                record->y1 = input->y;
                record->z1 = input->z;
                record->x2 = input->x;
                record->y2 = input->y;
                record->z2 = input->z;
                record->depth = -input->depth;
                func_80029FE4(input, record->direction);
                record->value = input->value;
                record->active = 1;
                gOverlay34Pointers[gOverlay34ActiveCount] = record;
                gOverlay34ActiveCount++;
            }
        }
    }
    return candidate;
}
