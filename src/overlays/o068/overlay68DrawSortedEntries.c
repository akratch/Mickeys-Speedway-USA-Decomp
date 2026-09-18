typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

typedef struct Overlay68Vector {
    f32 x;
    f32 y;
    s32 z;
} Overlay68Vector;

typedef struct Overlay68VectorOwner {
    u8 pad00[0x40];
    Overlay68Vector *vectors;
} Overlay68VectorOwner;

typedef struct Overlay68DrawEntry {
    s32 word0;
    s8 vectorIndex;
    u8 pad05[3];
    f32 weight;
    u8 pad0C[8];
} Overlay68DrawEntry;

typedef struct Overlay68DrawObject {
    u8 pad00[0x39];
    u8 mode39;
    u8 pad3A[6];
    f32 *scale;
    u8 pad44[0x0C];
    s32 renderState;
    u8 pad54[0x0C];
    Overlay68DrawEntry *entries;
    u8 pad64[4];
    Overlay68VectorOwner **vectorOwner;
    u8 pad6C[0x20];
    u8 entryCount;
} Overlay68DrawObject;

typedef struct Overlay68DrawDescriptor {
    s16 zero0;
    s16 zero2;
    s16 pad4;
    s16 mode6;
    f32 weight;
    f32 one;
    f32 x;
    f32 y;
    f32 z;
    s32 angle;
    s32 sourceWord;
} Overlay68DrawDescriptor;

extern f32 overlay68MeasureVectorReloc(f32 x, f32 y, s32 z);
extern void overlay68PrepareDrawReloc(Overlay68DrawObject *object);
extern void overlay68SubmitEntryReloc(u32 **displayList, s32 arg1, s32 arg2,
                                      Overlay68DrawObject *object,
                                      s32 renderState,
                                      Overlay68DrawDescriptor *descriptor,
                                      s32 mode, s32 objectMode);

/*
 * L112: order[8], distances[8], entries[8] close the extra 0x20 of non-save
 * local space (frame 0x128 to 0x108) at size 0x354 with the s0-s7/ra/f20
 * save set unchanged.  The descriptor block shifts with the smaller entries
 * array.  Indexing the collect loop (Lever 51) keeps the save set but grows
 * the body by 16 bytes.
 */
#ifdef NON_MATCHING
void overlay68DrawSortedEntries(u32 **displayList, s32 arg1, s32 arg2,
                                Overlay68DrawObject *object) {
    Overlay68VectorOwner *owner;
    Overlay68DrawEntry *entry;
    Overlay68Vector *vector;
    Overlay68Vector *nextVector;
    u32 *command;
    f32 inverseScale;
    s32 count;
    s32 i;
    s32 pass;
    s16 order[8];
    f32 distances[8];
    Overlay68DrawDescriptor descriptor;
    Overlay68DrawEntry *entries[8];

    owner = *object->vectorOwner;
    if (owner == 0) {
        return;
    }

    command = *displayList;
    *displayList = command + 2;
    command[0] = 0xFB000000;
    command[1] = 0xFFFFFF00;

    entry = object->entries;
    count = 0;
    if ((entry != 0) && (object->entryCount > 0)) {
        s32 limit;

        i = 0;
        limit = 4;
        do {
            vector = &owner->vectors[entry->vectorIndex];
            distances[count] = overlay68MeasureVectorReloc(
                vector->x, vector->y, vector->z);
            entries[count] = entry;
            order[count] = count;
            i++;
            entry++;
            count++;
        } while ((i < object->entryCount) && (i != limit));
    }

    if (count > 0) {
        for (pass = count - 1; pass > 0; pass--) {
            for (i = 0; i < pass; i++) {
                if (distances[order[i + 1]] < distances[order[i]]) {
                    s16 left;

                    left = order[i];
                    order[i] = order[i + 1];
                    order[i + 1] = left;
                }
            }
        }

        overlay68PrepareDrawReloc(object);
        descriptor.mode6 = 3;
        descriptor.angle = 0x3333;
        inverseScale = 1 / *object->scale;

        for (i = 0; i < count; i++) {
            entry = entries[order[i]];
            descriptor.zero0 = 0;
            nextVector = &owner->vectors[entry->vectorIndex];
            descriptor.zero2 = 0;
            descriptor.one = 1;
            descriptor.weight = entry->weight * inverseScale;
            vector = nextVector;
            descriptor.x = vector->x;
            descriptor.y = vector->y;
            descriptor.z = *(f32 *)&vector->z;
            descriptor.sourceWord = entry->word0;
            overlay68SubmitEntryReloc(displayList, arg1, arg2, object,
                                      object->renderState, &descriptor, 0xE,
                                      object->mode39);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o068/overlay68DrawSortedEntries/func_overlay_068_F0000EFC_18C805C.s")
#endif

/* PLATEAU-HANDOFF:overlay68DrawSortedEntries:start
 * symbol: overlay68DrawSortedEntries
 * score: 144/213 words
 * frame: 0x108
 * relocations: 3
 * first-mismatch: +0x4C
 * summary: L112 8/8/8 array counts close the frame at 0x108 and improve the masked residual 146 to 144. Descriptor homes shift with the smaller entries array. Lever 51 indexing grows size by 16. Identity-gated proc-0 is p1, 38 decisions. Colour landscape floor is 131 from w80=c16.
 * PLATEAU-HANDOFF:overlay68DrawSortedEntries:end
 */
