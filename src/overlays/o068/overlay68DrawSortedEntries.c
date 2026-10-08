typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

typedef struct Overlay68Command {
    u32 w0;
    u32 w1;
} Overlay68Command;

#define OVERLAY68_SET_ENV_COLOR(pkt) { Overlay68Command *_g = (Overlay68Command *)(pkt); _g->w0 = 0xFB000000; _g->w1 = 0xFFFFFF00; }

typedef struct Overlay68Vector {
    f32 x;
    f32 y;
    f32 z;
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

extern f32 overlay68MeasureVectorReloc(f32 x, f32 y, f32 z);
extern void overlay68PrepareDrawReloc(Overlay68DrawObject *object);
extern void overlay68SubmitEntryReloc(Overlay68Command **displayList, s32 mtx, s32 vertices,
                                      Overlay68DrawObject *object,
                                      s32 renderState,
                                      Overlay68DrawDescriptor *descriptor,
                                      s32 mode, s32 objectMode);

/*
 * Matched 2026-10-02 (lane x-sort) by rewriting the inherited body from the
 * listing in the shape of overlay 69's sorted renderer: the environment
 * colour as one packet macro, a plain counted collect loop ending in
 * i != 4, entry++ before count++, the bubble sort reusing i as its outer
 * index with the swap through one temporary, and the submit loop reading
 * order[i] into its own local before indexing entries[].  The 0x108 frame
 * comes from declaring five scalars above order[] and three between the
 * descriptor and entries[].
 */
void overlay68DrawSortedEntries(Overlay68Command **displayList, s32 mtx, s32 vertices,
                                Overlay68DrawObject *object) {
    Overlay68VectorOwner *owner;
    Overlay68DrawEntry *entry;
    Overlay68Vector *vector;
    s32 count;
    s32 i;
    s16 order[8];
    f32 inverseScale;
    f32 distances[8];
    Overlay68DrawDescriptor descriptor;
    s32 slot;
    s32 j;
    s32 temp;
    Overlay68DrawEntry *entries[8];

    owner = *object->vectorOwner;
    if (owner == 0) {
        return;
    }

    OVERLAY68_SET_ENV_COLOR((*displayList)++);

    entry = object->entries;
    count = 0;
    if (entry != 0) {
        for (i = 0; (i < object->entryCount) && (i != 4); i++) {
            vector = &owner->vectors[entry->vectorIndex];
            distances[count] = overlay68MeasureVectorReloc(vector->x, vector->y,
                                                           vector->z);
            entries[count] = entry;
            order[count] = count;
            entry++;
            count++;
        }
    }

    if (count > 0) {
        for (i = count - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                if (distances[order[j + 1]] < distances[order[j]]) {
                    temp = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = temp;
                }
            }
        }

        overlay68PrepareDrawReloc(object);
        inverseScale = 1.0f / *object->scale;
        descriptor.mode6 = 3;
        descriptor.angle = 0x3333;
        for (i = 0; i < count; i++) {
            slot = order[i];
            entry = entries[slot];
            vector = &owner->vectors[entry->vectorIndex];
            descriptor.zero0 = 0;
            descriptor.zero2 = 0;
            descriptor.weight = entry->weight * inverseScale;
            descriptor.one = 1.0f;
            descriptor.x = vector->x;
            descriptor.y = vector->y;
            descriptor.z = vector->z;
            descriptor.sourceWord = entry->word0;
            overlay68SubmitEntryReloc(displayList, mtx, vertices, object,
                                      object->renderState, &descriptor, 0xE,
                                      object->mode39);
        }
    }
}
