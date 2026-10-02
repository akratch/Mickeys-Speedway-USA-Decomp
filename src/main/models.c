/*
 * Resident model loading and instance management -- ROM 0x20020-0x21DA0
 * (VRAM 0x8001F420-0x800211A0).
 *
 * The working TU is identified from three exact masked-skeleton matches to
 * Jet Force Gemini's built src/models.c.o, the first at the existing yaml
 * boundary, plus the allocator, texture, and matrix call graph of the rest of
 * the block. It is not a whole-object match; docs/resident.md section 3.7
 * records the evidence and keeps uncertain JFG correspondences as comments
 * rather than adopting names.
 *
 * PROVENANCE -- JFG's public decomp was consulted for the models.c function
 * order, names, prototypes, and structure vocabulary. No body was adapted in
 * the initial all-GLOBAL_ASM split. Any body later adapted from JFG must retain a
 * point-of-use PROVENANCE note, and Mickey's own bytes remain authoritative.
 *
 * Flags: -O2 -mips2 -32, via the measured src/main/ Makefile rule.
 */

#include "PR/ultratypes.h"
#include "game/math.h"
#include "game/models.h"
#include "n_audio/mbi.h"

extern s32 D_80079C00;
#ifdef NON_MATCHING
extern SuspendedModelTexture *D_80079C04;
#else
extern void *D_80079C04;
#endif
extern s16 D_80079C08;
extern s8 D_8007BD98;
extern s32 *D_800CB480;
extern s32 *D_800CB484;
extern s32 *D_800CB488;
extern s32 D_800CB48C;
extern s32 D_800CB490;
extern s32 D_800CB494;
extern s8 D_800CB498[];
extern s16 D_800CB49C[];
extern s16 D_800CB4A2[];
extern Gfx *D_800CB4A4;

typedef struct ModelCopySource ModelCopySource;
typedef struct ModelInstanceSource ModelInstanceSource;
typedef struct ModelConstructedInstance ModelConstructedInstance;
extern void *func_8001FBCC(ModelCopySource *source);
extern ModelConstructedInstance *func_8001FC50(ModelInstanceSource *source,
                                                s32 pointCopies);

void *func_8002B280(s32 size, s32 tag);
void *func_8002B314(s32 size, s32 tag);
s32 *piRomLoad(s32 assetId);
void *func_80034448(s16 textureId);
void func_800347A0(void *texture);
s32 func_8003484C(void *texture);
void texLoadTextureAddr(s32 id, s32 value);
void func_80034424(s32 enabled);
void func_80034920(Gfx **displayList);
void func_800349A4(Gfx **displayList, void *texture, s32 flags, s32 parameter);
void func_80020AD4(void);
void func_8005AAC0(void *animation);
extern s32 func_8004D7A8(s32 assetId, s32 assetOffset);
extern u8 *func_8004D7E0(u8 *compressed, u8 *output);
extern s32 func_8005A7A0(void *model, s32 modelId);
extern s32 piRomLoadSection(u32 assetId, u32 address, s32 offset, s32 size);
struct ModelGfxSource;
s32 func_8002057C(Gfx **out, struct ModelGfxSource *model, s32 arg2, s32 arg3,
                  s32 arg4, s32 arg5, s32 arg6);
void mmFree(void *ptr);

/*
 * PROVENANCE -- body adapted from JFG's public src/models.c
 * func_8003B870_3C470. The JFG built object carries this exact 15-word
 * skeleton at func_8003B640; Mickey's linked bytes are the authority here.
 */
void func_8001F420(u16 *src, u16 *dest, s32 len) {
    len = (len + 1) >> 1;
    while (len--) {
        *dest++ = *src++;
    }
}
/*
 * PROVENANCE -- body adapted from the initial portion of JFG's public
 * modInitModels. Mickey ends after counting the model table and does not have
 * JFG's later allocations; Mickey's globals, calls, and bytes are authoritative.
 */
void modInitModels(void) {
    D_800CB484 = func_8002B280(0x2A8, 0x8A);
    D_800CB488 = func_8002B280(0x190, 0x8A);
    D_800CB48C = 0;
    D_800CB494 = 0;
    D_800CB4A4 = func_8002B280(0x2000, 0x8A);
    D_800CB480 = piRomLoad(0x26);
    D_800CB490 = 0;
    while (D_800CB480[D_800CB490] != -1) {
        D_800CB490++;
    }
    D_800CB490--;
}
typedef struct ModelGfxPart {
    u8 textureIndex;
    s8 group;
    u8 pad2[2];
    s8 segmentEnds[2];
    s16 vertexStart;
    s16 vertexIndex;
    u8 padA;
    u8 textureParameter;
    u32 flags;
} ModelGfxPart;

/* Matched 2026-10-02 (lane x-models) by porting DKR's object_model_init
 * shape over the inherited m2c body: the ASSETCACHE shift macros (no strength
 * reduction of the cache scan), the VERSION_79 rollback flags as s8 locals,
 * struct fields for the relocated pointers, a separate counter for the nested
 * display-list loop (which keeps i caller-saved), and declaration order for
 * the home ladder. */
/* PROVENANCE: adapted from DKR's object_model_init (src/object_models.c, the
 * VERSION_79 cache rollback); Mickey's fields, nested display lists, texture
 * cleanup and instance builders are its own. */
/* DKR macros.h ASSETCACHE_ID/ASSETCACHE_PTR: one cache entry is an id word
 * followed by a pointer word. */
#define MODEL_CACHE_ID(x) ((x << 1) + 0)
#define MODEL_CACHE_PTR(x) ((x << 1) + 1)

void *func_8001F520(s32 modelID, s32 flags) {
    s32 i;
    s32 j;
    s32 cacheIndex;
    s32 highBit;
    ObjectModel *objMdl;
    s32 romOffset;
    s32 compressedSize;
    void *instance;
    s32 start;
    s8 fromFree;
    s8 cacheChanged;
    s32 unused; /* unreferenced: the target's home ladder has a cell here (L99) */
    s32 modelSize;
    s32 group;
    u32 compressedData;
    s32 last;

    highBit = modelID & 0x8000;
    modelID ^= highBit;
    if (modelID >= D_800CB490) {
        modelID = 0;
    }
    for (i = 0; i < D_800CB48C; i++) {
        if (modelID == D_800CB484[MODEL_CACHE_ID(i)]) {
            objMdl = (ObjectModel *) D_800CB484[MODEL_CACHE_PTR(i)];
            if (highBit) {
                instance = func_8001FBCC((ModelCopySource *) objMdl);
            } else {
                instance = func_8001FC50((ModelInstanceSource *) objMdl, flags & 3);
            }
            if (instance != NULL) {
                objMdl->references++;
            }
            return instance;
        }
    }
    fromFree = FALSE;
    cacheChanged = FALSE;
    if (D_800CB494 > 0) {
        D_800CB494--;
        fromFree = TRUE;
        cacheIndex = D_800CB488[D_800CB494];
    } else {
        cacheIndex = D_800CB48C;
        cacheChanged = TRUE;
        D_800CB48C++;
    }
    romOffset = D_800CB480[modelID];
    compressedSize = D_800CB480[modelID + 1] - romOffset;
    modelSize = func_8004D7A8(0x27, romOffset) + sizeof(ObjectModel);
    objMdl = (ObjectModel *) func_8002B314(modelSize, 0x8A);
    if (objMdl == NULL) {
        if (fromFree) {
            D_800CB494++;
        }
        if (cacheChanged) {
            D_800CB48C--;
        }
        return NULL;
    }
    compressedData = (u32) ((u8 *) objMdl + modelSize) - compressedSize;
    piRomLoadSection(0x27, compressedData, romOffset, compressedSize);
    func_8004D7E0((u8 *) compressedData, (u8 *) objMdl);
    if (objMdl->nestedCount != 0) {
        objMdl->nestedAllocations = (void **) func_8002B314((objMdl->nestedCount * 4) + 4, 0x8A);
        if (objMdl->nestedAllocations == NULL) {
            if (fromFree) {
                D_800CB494++;
            }
            if (cacheChanged) {
                D_800CB48C--;
            }
            return NULL;
        }
    } else {
        objMdl->nestedAllocations = NULL;
    }
    objMdl->textures = (ModelTexture *) ((s32) objMdl->textures + (u8 *) objMdl);
    objMdl->vertices = (void *) ((s32) objMdl->vertices + (u8 *) objMdl);
    objMdl->triangles = (void *) ((s32) objMdl->triangles + (u8 *) objMdl);
    objMdl->batches = (struct ModelGfxPart *) ((s32) objMdl->batches + (u8 *) objMdl);
    objMdl->nestedGroups = (u8 *) ((s32) objMdl->nestedGroups + (u8 *) objMdl);
    objMdl->unk30 = (void *) ((s32) objMdl->unk30 + (u8 *) objMdl);
    objMdl->unk34 = (void *) ((s32) objMdl->unk34 + (u8 *) objMdl);
    objMdl->unk38 = (void *) ((s32) objMdl->unk38 + (u8 *) objMdl);
    objMdl->unk60 = (void *) ((s32) objMdl->unk60 + (u8 *) objMdl);
    if (objMdl->unk64 != NULL) {
        objMdl->unk64 = (void *) ((s32) objMdl->unk64 + (u8 *) objMdl);
    }
    objMdl->unk5C = (void *) ((s32) objMdl->unk5C + (u8 *) objMdl);
    if (objMdl->unk54 != NULL) {
        objMdl->unk54 = (void *) ((s32) objMdl->unk54 + (u8 *) objMdl);
    }
    objMdl->references = 1;
    objMdl->animationCount = 0;
    objMdl->unk58 = NULL;
    objMdl->animations = NULL;
    objMdl->unk28 = NULL;
    objMdl->unk68 = NULL;
    objMdl->unk6C = NULL;
    for (i = 0; i < objMdl->numberOfTextures; i++) {
        objMdl->textures[i].texture = func_80034448(objMdl->textures[i].textureId);
        if (objMdl->textures[i].texture == NULL) {
            for (j = 0; j < i; j++) {
                func_800347A0(objMdl->textures[j].texture);
                objMdl->textures[j].texture = NULL;
            }
            for (; j < objMdl->numberOfTextures; j++) {
                objMdl->textures[j].texture = NULL;
            }
            goto block_30;
        }
    }
    for (i = 0; i < objMdl->numberOfBatches; i++) {
        if (objMdl->batches[i].textureIndex != 0xFF &&
            objMdl->batches[i].textureIndex >= objMdl->numberOfTextures) {
            goto block_30;
        }
    }
    if (func_8005A7A0(objMdl, modelID) == 0) {
        goto block_30;
    }
    if (objMdl->unk11 != 0) {
        objMdl->unk28 = func_8002B314(objMdl->numberOfTextures * 8, 0x8A);
        if (objMdl->unk28 == NULL) {
            goto block_30;
        }
    }
    if (objMdl->nestedCount != 0) {
        start = 0;
        for (group = 0; group < objMdl->nestedCount; group++) {
            last = objMdl->nestedGroups[group] - 1;
            func_8002057C((Gfx **) &objMdl->nestedAllocations[group], (struct ModelGfxSource *) objMdl, 0, 0,
                          start, last, 0);
            start = last + 1;
        }
        func_8002057C((Gfx **) &objMdl->nestedAllocations[group], (struct ModelGfxSource *) objMdl, 0, 0,
                      start, 0xFF, 0);
    } else {
        objMdl->textureAnimationCount = func_8002057C((Gfx **) &objMdl->unk68,
                                                      (struct ModelGfxSource *) objMdl, 0, 0, 0, 0xFF, 0);
        if (objMdl->unk68 == NULL) {
            goto block_30;
        }
        func_8002057C((Gfx **) &objMdl->unk6C, (struct ModelGfxSource *) objMdl, 4, 0, 0, 0xFF, 0);
        if (objMdl->unk6C == NULL) {
            goto block_30;
        }
    }
    if (highBit) {
        instance = func_8001FBCC((ModelCopySource *) objMdl);
    } else {
        instance = func_8001FC50((ModelInstanceSource *) objMdl, flags & 3);
    }
    if (instance != NULL) {
        D_800CB484[MODEL_CACHE_ID(cacheIndex)] = modelID;
        D_800CB484[MODEL_CACHE_PTR(cacheIndex)] = (s32) objMdl;
        if (D_800CB48C < 0x55) {
            return instance;
        }
    }
block_30:
    if (cacheChanged) {
        D_800CB48C--;
    }
    if (fromFree) {
        D_800CB494++;
    }
    func_80020278(objMdl);
    return NULL;
}
/*
 * PROVENANCE -- JFG's built models.c object supplies the exact corresponding
 * skeleton at func_8003BE68, but no public C body. This body is reconstructed
 * from Mickey's own function.
 */
void func_8001FB64(s32 count, MtxF *matrices) {
    while (count > 0) {
        count--;
        (*matrices)[0][0] = 1.0f;
        (*matrices)[0][1] = 0.0f;
        (*matrices)[0][2] = 0.0f;
        (*matrices)[0][3] = 0.0f;
        (*matrices)[1][0] = 0.0f;
        (*matrices)[1][1] = 1.0f;
        (*matrices)[1][2] = 0.0f;
        (*matrices)[1][3] = 0.0f;
        (*matrices)[2][0] = 0.0f;
        (*matrices)[2][1] = 0.0f;
        (*matrices)[2][2] = 1.0f;
        (*matrices)[2][3] = 0.0f;
        (*matrices)[3][0] = 0.0f;
        (*matrices)[3][1] = 0.0f;
        (*matrices)[3][2] = 0.0f;
        (*matrices)[3][3] = 1.0f;
        matrices++;
    }
}
struct ModelCopySource {
    u8 pad0[0x12];
    s16 count;
    u8 pad14[8];
    u16 *data;
};

typedef struct ModelCopyAllocation {
    ModelCopySource *source;
    u16 *data;
    s16 unk8;
    s16 unkA;
} ModelCopyAllocation;

/*
 * PROVENANCE -- body adapted from JFG's public src/models.c
 * func_8003C12C_3CD2C. Mickey's header size, field widths, allocator, tag,
 * and linked bytes are authoritative.
 */
void *func_8001FBCC(ModelCopySource *source) {
    u16 *data;
    ModelCopyAllocation *allocation;

    allocation = func_8002B314(source->count * 0xA + 0xC, 0x8A);
    if (allocation != NULL) {
        data = (u16 *)((u8 *)allocation + 0xC);
        allocation->source = source;
        allocation->data = data;
        allocation->unk8 = 2;
        allocation->unkA = 0;
        func_8001F420(source->data, data, source->count * 0xA);
    }
    return allocation;
}

typedef struct ModelInstancePoint {
    s16 x;
    s16 y;
    s16 z;
    u8 flags[4];
} ModelInstancePoint;

typedef struct ModelInstanceCopy {
    s16 value0;
    s16 value2;
    u32 value4;
} ModelInstanceCopy;

typedef struct ModelInstancePointIndex {
    u16 pointIndex;
    u16 pad2;
} ModelInstancePointIndex;

struct ModelInstanceSource {
    u8 pad0[0x10];
    u8 copyCount;
    u8 hasCopies;
    s16 pointCount;
    u8 pad14[8];
    ModelInstancePoint *points;
    u8 pad20[8];
    ModelInstanceCopy *copies;
    u8 pad2C;
    u8 coordinateCount;
    u8 dataCount44;
    u8 dataCount48;
    ModelInstancePointIndex *pointIndices;
    u8 pad34[0x1A];
    s8 mode;
    s8 matrixCount;
};

struct ModelConstructedInstance {
    ModelInstanceSource *source;
    ModelInstancePoint *points;
    s16 status;
    u8 padA[2];
    MtxF *matricesA;
    MtxF *matricesB;
    ModelInstancePoint *pointsA;
    ModelInstancePoint *pointsB;
    u8 pad1C[8];
    void *modeData;
    u8 pad28[0x18];
    f32 *coordinates;
    void *data44;
    void *data48;
    ModelInstanceCopy *copies;
    s16 *state[2];
};

/* PROVENANCE: local size and alignment lifetimes are adapted from JFG upstream
 * efd5abb's corresponding src/models.c function, func_8003BF58. JFG retains
 * that function as GLOBAL_ASM; Mickey's layout and bytes remain authority. */
/* Workbench: size delta 0 and frame 0x78 both closed (from -12 and +8).
 * Closed by: declarations laid on the target's home ladder (every local at
 * function scope, 18 slots, instance at -0x40); no allocation-size local, so
 * the sum is a CSE temp spilled across the allocator call; the state reset
 * written as indexed stores through state[i], which reloads the pointer per
 * store as the target does (the goto keeps uopt from unrolling it); and
 * matrixBytes carrying the matrix count before it is shifted (x = f(x)).
 * Remains: register naming. The target keeps modeBytes in its home from both
 * arms and gives pointBytes ra; here modeBytes is coloured ra and spilled. */
#ifdef NON_MATCHING
ModelConstructedInstance *func_8001FC50(ModelInstanceSource *source, s32 pointCopies) {
    ModelConstructedInstance *instanceCursor;
    s32 i;
    s32 j;
    s32 matrixBytes;
    s32 pointBytes;
    s32 dataBytes44;
    s32 dataBytes48;
    s32 coordinateBytes;
    s32 extraBytes;
    u32 *clear;
    s32 words;
    s32 modeBytes;
    u8 *end;
    ModelInstancePoint *sourcePoint;
    ModelInstancePoint *destinationPoint;
    ModelConstructedInstance *instance;
    f32 *coordinate;
    ModelInstancePoint *sourcePoint2;

    matrixBytes = 0;
    if (source->mode != 0) {
        matrixBytes = source->matrixCount;
        modeBytes = matrixBytes * 0x1C + 0xC;
        matrixBytes <<= 6;
    } else {
        modeBytes = 0;
    }

    pointBytes = source->pointCount * sizeof(ModelInstancePoint);
    if ((pointBytes & 7) != 0) {
        pointBytes = pointBytes - (pointBytes & 7) + 8;
    }
    dataBytes44 = source->dataCount44 * 0xC;
    if ((dataBytes44 & 3) != 0) {
        dataBytes44 = dataBytes44 - (dataBytes44 & 3) + 4;
    }
    dataBytes48 = source->dataCount48 * 0xC;
    if ((dataBytes48 & 3) != 0) {
        dataBytes48 = dataBytes48 - (dataBytes48 & 3) + 4;
    }
    coordinateBytes = source->coordinateCount * 0xC;
    if ((coordinateBytes & 3) != 0) {
        coordinateBytes = coordinateBytes - (coordinateBytes & 3) + 4;
    }
    extraBytes = 0;
    if (source->hasCopies != 0) {
        extraBytes = source->copyCount * 8 + 0xA8;
    }

    instance = func_8002B314((matrixBytes << 1) + (pointBytes * pointCopies) + modeBytes + dataBytes44 +
                     dataBytes48 + coordinateBytes + extraBytes + 0x58, 0x8A);
    if (instance != NULL) {
        clear = (u32 *)instance;
        words = ((matrixBytes << 1) + (pointBytes * pointCopies) + modeBytes + dataBytes44 +
                     dataBytes48 + coordinateBytes + extraBytes + 0x58) >> 2;

        while (words--) {
            *clear++ = 0;
        }

        if (source->matrixCount != 0 && matrixBytes != 0) {
            instance->matricesA = (MtxF *)((u8 *)instance + 0x58);
            instance->matricesB = (MtxF *)((u8 *)instance->matricesA + matrixBytes);
            func_8001FB64(source->matrixCount, instance->matricesA);
            func_8001FB64(source->matrixCount, instance->matricesB);
        }

        if (pointCopies > 0) {
            instance->pointsA = (ModelInstancePoint *)((u8 *)instance + (matrixBytes << 1) + 0x58);
        } else {
            instance->pointsA = source->points;
        }
        if (pointCopies >= 2) {
            instance->pointsB = (ModelInstancePoint *)((u8 *)instance->pointsA + pointBytes);
        } else {
            instance->pointsB = instance->pointsA;
        }
        if (source->matrixCount != 0 && modeBytes != 0) {
            instance->modeData = (u8 *)instance + (matrixBytes << 1) + (pointBytes * pointCopies) + 0x58;
        }
        if (source->dataCount44 != 0) {
            instance->data44 = (u8 *)instance + (matrixBytes << 1) + (pointBytes * pointCopies) + modeBytes + 0x58;
        }
        if (source->dataCount48 != 0) {
            instance->data48 = (u8 *)instance + (matrixBytes << 1) + (pointBytes * pointCopies) + modeBytes + dataBytes44 + 0x58;
        }
        if (source->coordinateCount != 0) {
            instance->coordinates = (f32 *)((u8 *)instance + (matrixBytes << 1) + (pointBytes * pointCopies) + modeBytes +
                                              dataBytes44 + dataBytes48 + 0x58);
        }
        if (source->hasCopies != 0) {
            instance->copies = (ModelInstanceCopy *)((u8 *)instance + (matrixBytes << 1) + (pointBytes * pointCopies) +
                                                       modeBytes + dataBytes44 + dataBytes48 + coordinateBytes + 0x58);
            end = (u8 *)(instance->copies + source->copyCount);
            if (((s32)end & 7) != 0) {
                end = end - ((s32)end & 7) + 8;
            }
            instance->state[0] = (s16 *)end;
            instance->state[1] = (s16 *)(end + 0x50);

            /* The natural nested loop: IDO's default unroller emits the
             * target's four-store body (byte-identical to the old goto form). */
            for (i = 0; i < 2; i++) {
                for (j = 0; j < 40; j++) {
                    instance->state[i][j] = 0;
                }
            }
        }

        i = 0;
        if (pointCopies > 0) {
            instanceCursor = instance;
            do {
                sourcePoint = source->points;
                destinationPoint = instanceCursor->pointsA;
                j = 0;
                if (source->pointCount > 0) {
                    do {
                        destinationPoint->x = sourcePoint->x;
                        destinationPoint->y = sourcePoint->y;
                        destinationPoint->z = sourcePoint->z;
                        destinationPoint->flags[0] = sourcePoint->flags[0];
                        destinationPoint->flags[1] = sourcePoint->flags[1];
                        destinationPoint->flags[2] = sourcePoint->flags[2];
                        destinationPoint->flags[3] = sourcePoint->flags[3];
                        j++;
                        sourcePoint++;
                        destinationPoint++;
                    } while (j < source->pointCount);
                }
                i++;
                instanceCursor = (ModelConstructedInstance *)((u8 *)instanceCursor + 4);
            } while (i != pointCopies);
        }

        if (source->mode == 0) {
            coordinate = instance->coordinates;
            i = 0;
            if (source->coordinateCount > 0) {
                do {
                    sourcePoint2 = &source->points[source->pointIndices[i].pointIndex];
                    *coordinate++ = sourcePoint2->x;
                    *coordinate++ = sourcePoint2->y;
                    *coordinate++ = sourcePoint2->z;
                    i++;
                } while (i < source->coordinateCount);
            }
        }

        if (instance->copies != NULL) {
            i = 0;
            if (source->copyCount > 0) {
                do {
                    instance->copies[i].value0 = source->copies[i].value0;
                    instance->copies[i].value2 = source->copies[i].value2;
                    instance->copies[i].value4 = source->copies[i].value4;
                    i++;
                } while (i < source->copyCount);
            }
        }
        instance->source = source;
        instance->status = 2;
        instance->points = instance->pointsA;
    }
    return instance;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FC50.s")
#endif
/*
 * PROVENANCE -- body adapted from JFG's public modFreeModel. Mickey omits
 * JFG's per-instance animation allocations; its model reference and cache
 * release path has the same structure and is reconstructed against Mickey.
 */
void modFreeModel(ModelInstance *modInst) {
    ObjectModel *model;
    s32 i;
    s32 modelIndex;

    if (modInst != NULL) {
        model = modInst->objModel;
        mmFree(modInst);
        model->references--;
        if (model->references <= 0) {
            i = 0;
            modelIndex = -1;
            while (i < D_800CB48C) {
                if (model == (ObjectModel *)D_800CB484[(i << 1) + 1]) {
                    modelIndex = i;
                }
                i++;
            }

            if (modelIndex != -1) {
                func_80020278(model);
                D_800CB488[D_800CB494] = modelIndex;
                D_800CB494++;
                D_800CB484[modelIndex << 1] = -1;
                D_800CB484[(modelIndex << 1) + 1] = -1;
            }
        }
    }
}
/*
 * PROVENANCE -- body adapted from JFG's public func_8003C92C_3D52C model
 * destructor. Mickey's smaller field set and offsets are reconstructed solely
 * from this function's own loads and calls.
 */
void func_80020278(ObjectModel *model) {
    s32 freed;
    s32 index;

    index = 0, freed = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (model->textures[index].texture != NULL) {
                func_800347A0(model->textures[index].texture);
            }
            freed++;
            index++;
        } while (freed < model->numberOfTextures);
    }

    if (model->unk58 != NULL) {
        mmFree(model->unk58);
    }
    if (model->unk28 != NULL) {
        mmFree(model->unk28);
    }
    if (model->unk68 != NULL) {
        mmFree(model->unk68);
    }
    if (model->unk6C != NULL) {
        mmFree(model->unk6C);
    }

    if (model->animationCount != 0 && model->animations != NULL) {
        freed = 0;
        index = 0;
        do {
            func_8005AAC0(model->animations[index]);
            freed++;
            index++;
        } while (freed < model->animationCount);
        mmFree(model->animations);
    }

    if (model->nestedAllocations != NULL) {
        freed = model->nestedCount + 1;
        while (freed--) {
            mmFree(model->nestedAllocations[freed]);
        }
        mmFree(model->nestedAllocations);
    }
    mmFree(model);
}
/* Mickey-only reconstruction; JFG supplied no adoptable helper name. */
void func_800203E0(ObjectModel *model) {
    s32 offset;
    s32 loaded;

    offset = 0;
    loaded = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (((ModelTexture *)((u8 *)model->textures + offset))->texture == NULL) {
                ((ModelTexture *)((u8 *)model->textures + offset))->texture =
                    func_80034448(((ModelTexture *)((u8 *)model->textures + offset))->textureId);
            }
            loaded++;
            offset += sizeof(ModelTexture);
        } while (loaded < model->numberOfTextures);
    }
    if (model->unk68 == NULL) {
        model->textureAnimationCount = func_8002057C((Gfx **)&model->unk68,
                                                     (struct ModelGfxSource *)model, 0, 0, 0, 0xFF, 0);
    }
    if (model->unk6C == NULL) {
        func_8002057C((Gfx **)&model->unk6C, (struct ModelGfxSource *)model, 4, 0, 0, 0xFF, 0);
    }
}
/* Mickey-only reconstruction; JFG supplied no adoptable helper name or body. */
void func_800204B8(ObjectModel *model) {
    s32 offset;
    s32 i;

    offset = 0;
    i = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (((ModelTexture *)((u8 *)model->textures + offset))->texture != NULL) {
                func_800347A0(((ModelTexture *)((u8 *)model->textures + offset))->texture);
                ((ModelTexture *)((u8 *)model->textures + offset))->texture = NULL;
            }
            i++;
            offset += sizeof(ModelTexture);
        } while (i < model->numberOfTextures);
    }
    if (model->unk68 != NULL) {
        mmFree(model->unk68);
        model->unk68 = NULL;
    }
    if (model->unk6C != NULL) {
        mmFree(model->unk6C);
        model->unk6C = NULL;
    }
}
/*
 * PROVENANCE -- name follows JFG's public models.c symbol at the same TU
 * position. The body is reconstructed from Mickey's three instructions.
 */
void modelSetModelFlags(s32 flags) {
    D_80079C00 = flags;
}
/*
 * PROVENANCE -- name follows JFG's public models.c symbol at the same TU
 * position. The body is reconstructed from Mickey's three instructions.
 */
s32 modelGetModelFlags(void) {
    return D_80079C00;
}


typedef struct ModelGfxTextureRef {
    void *texture;
    u32 pad4;
} ModelGfxTextureRef;

typedef struct ModelGfxCacheEntry {
    s16 parameter;
    s16 frame;
    u32 flags;
} ModelGfxCacheEntry;

typedef struct ModelGfxTexture {
    u8 pad0[3];
    u8 flags;
    s16 control;
    u8 pad6[0xA];
    u16 frameLimit;
} ModelGfxTexture;

typedef struct ModelGfxSource {
    u8 pad0[0x11];
    u8 hasTextures;
    u8 pad12[4];
    s16 partCount;
    ModelGfxTextureRef *textures;
    u8 pad1C[4];
    u8 *vertices;
    ModelGfxPart *parts;
    ModelGfxCacheEntry *cacheEntries;
    u8 pad2C[0x22];
    s8 mode;
} ModelGfxSource;

struct ModelTextureUsage;
void func_80020B10(Gfx **displayList, s8 *textureIds, s8 *slots,
                   struct ModelTextureUsage *usage, s32 entryIndex,
                   u32 textureBase);

/* PROVENANCE: declaration lifetimes were first adapted from JFG upstream
 * efd5abb's corresponding makeModelGfx function, which JFG retains as
 * GLOBAL_ASM; Mickey's own layout, constants, and bytes remain authority.
 * Matched 2026-10-02 (lane x-models) by rewriting the inherited m2c shape:
 * one-line packet macros (as1's same-line tie stores w1 first), the vertex
 * length as (n << 3) + (n << 1) + 8 and the mode-0 address multiply unsigned,
 * an s32 parameter (the callee takes s32), partFlags reused for the combined
 * flags with the 0x800 test reading part->flags, an s32 texture index,
 * partIndex reused as the copy counter, and the declaration order that lays
 * the homes down from 0xCC exactly as the target does. */
/* PROVENANCE: the packet layouts follow DKR's f3ddkr.h gSPVertexDKR,
 * gSPPolygon and gSPSelectMatrixDKR (operand-before-opcode w0 order); the
 * vertex length and destination fields are Mickey's own encoding. */
#define gSPModelVertex(pkt, v, n, v0) { Gfx *_g = (Gfx *)(pkt); _g->words.w0 = _SHIFTL(((n) << 3) | ((u32)(v) & 6), 16, 8) | _SHIFTL(4, 24, 8) | _SHIFTL(MODEL_VTX_LEN(n) | ((v0) << 9), 0, 16); _g->words.w1 = (unsigned int)(v); }
#define MODEL_VTX_LEN(n) (((n) << 3) + ((n) << 1) + 8)
#define gSPModelPolygon(pkt, ptr, numTris, tex) { Gfx *_g = (Gfx *)(pkt); _g->words.w0 = _SHIFTL((((numTris) - 1) << 4) | (tex), 16, 8) | _SHIFTL(5, 24, 8) | _SHIFTL((numTris) * 16, 0, 16); _g->words.w1 = (unsigned int)(ptr); }
#define gSPModelSelectMatrix(pkt, num) gMoveWd(pkt, 0x0A, 0, (num) << 6)
#define MODEL_PHYS(x) ((u32)(x) & 0x0FFFFFFF)

s32 func_8002057C(Gfx **out, ModelGfxSource *model, s32 flags, s32 mask,
                  s32 lowerGroup, s32 upperGroup, s32 forceSimple) {
    s32 partIndex;
    Gfx *sourceDisplayList;
    ModelGfxPart *part;
    ModelGfxTexture *texture;
    s32 vertexCount;
    s32 triangleCount;
    u32 commandCount;
    s32 previousVertex;
    s32 slotIndex;
    s32 nextVertex;
    s32 lastParameter;
    s32 parameter;
    s32 cacheEnabled;
    s32 partFlags;
    s32 cacheCount;
    s8 slots[4];
    s32 textureIndex;
    void *lastTexture;
    s16 vertexStart;
    s16 vertexIndex;
    void *address;
    ModelGfxCacheEntry *cacheEntry;
    Gfx *displayList;

    part = model->parts;
    func_80034424(1);
    if (flags & 4) {
        D_8007BD98 = 1;
    }
    mask = ~mask;
    displayList = D_800CB4A4;
    lastTexture = (void *)-1;
    lastParameter = -1;
    if (lowerGroup == 0 && forceSimple == 0) {
        func_80020AD4();
        func_80034920(&displayList);
    } else if (forceSimple != 0) {
        func_80020AD4();
        func_80034920(NULL);
        gDPPipeSync(displayList++);
        gSPSetGeometryMode(displayList++, G_ZBUFFER | G_FOG);
    }

    partIndex = 0;
    cacheEntry = model->cacheEntries;
    cacheCount = 0;
    if (model->partCount > 0) {
        do {
            partFlags = part->flags;

            if (part->group >= lowerGroup && part->group <= upperGroup && !(part->flags & 0x800)) {

                vertexStart = part->vertexStart;
                vertexIndex = part->vertexIndex;
                vertexCount = part[1].vertexStart - vertexStart;
                triangleCount = part[1].vertexIndex - vertexIndex;
                address = model->vertices + (vertexIndex << 4);
                textureIndex = part->textureIndex;
                if (textureIndex == 0xFF || forceSimple != 0) {
                    parameter = 0;
                    texture = NULL;
                    cacheEnabled = 0;
                } else {
                    parameter = part->textureParameter << 14;
                    texture = model->textures[textureIndex].texture;
                    cacheEnabled = 1;
                }

                partFlags = (partFlags | flags | D_80079C00) & mask;
                if (model->hasTextures != 0 && texture != NULL &&
                    (texture != lastTexture || parameter != lastParameter)) {
                    cacheEntry->parameter = parameter;
                    cacheEntry->frame = -1;
                    cacheEntry->flags = (part->flags & ~0xFF) | textureIndex;
                    if (texture->control & 0x40) {
                        cacheEntry->frame = parameter + 0x100;
                        if (cacheEntry->frame >= texture->frameLimit) {
                            if (texture->flags & 2) {
                                cacheEntry->frame = 0;
                            } else {
                                cacheEntry->frame -= 0x100;
                            }
                        }
                    }
                    cacheEntry++;
                    cacheCount++;
                }
                lastTexture = texture;
                lastParameter = parameter;

                func_800349A4(&displayList, texture, partFlags, parameter);
                if (model->mode == 0) {
                    gSPModelVertex(displayList++, MODEL_PHYS(vertexStart * 10U), vertexCount, 0);
                } else {
                    func_80020B10(&displayList, &part->group, slots,
                                   (struct ModelTextureUsage *)model, partIndex, 0);
                    for (slotIndex = 0, previousVertex = 0; previousVertex < vertexCount; previousVertex = nextVertex, slotIndex++) {
                        gSPModelSelectMatrix(displayList++, slots[slotIndex]);
                        if (slotIndex < 2) {
                            nextVertex = part->segmentEnds[slotIndex];
                        } else {
                            nextVertex = vertexCount;
                        }
                        gSPModelVertex(displayList++, MODEL_PHYS((vertexStart + previousVertex) * 10),
                                       nextVertex - previousVertex, previousVertex);
                    }
                }
                gSPModelPolygon(displayList++, MODEL_PHYS(address), triangleCount, cacheEnabled);
            }
            partIndex++;
            part++;
        } while (partIndex < model->partCount);
    }

    gDPPipeSync(displayList++);
    gSPEndDisplayList(displayList++);

    commandCount = displayList - D_800CB4A4;
    displayList = *out = func_8002B314(commandCount * sizeof(Gfx), 0x8A);
    if (displayList != NULL) {
        sourceDisplayList = D_800CB4A4;
        for (partIndex = 0; partIndex < commandCount; partIndex++) {
            displayList->words.w0 = sourceDisplayList->words.w0;
            displayList->words.w1 = sourceDisplayList->words.w1;
            sourceDisplayList++;
            displayList++;
        }
    }

    func_80034424(0);
    D_8007BD98 = 0;
    return cacheCount;
}
/*
 * PROVENANCE -- JFG's built models.c object supplies the exact corresponding
 * skeleton at func_8003E100, but no public C body. This body is reconstructed
 * from Mickey's own function.
 */
void func_80020AD4(void) {
    s32 i;

    i = 0;
    do {
        i++;
        D_800CB498[i - 1] = -1;
        D_800CB49C[i - 1] = 1000;
    } while (D_800CB4A2 != &D_800CB49C[i]);
}

typedef struct ModelTextureUsageEntry {
    u8 pad0;
    s8 textureIds[3];
    u8 pad4[0xC];
} ModelTextureUsageEntry;

typedef struct ModelTextureUsage {
    u8 pad0[0x16];
    s16 entryCount;
    u8 pad18[0xC];
    ModelTextureUsageEntry *entries;
} ModelTextureUsage;

/* PROVENANCE: declaration and cursor lifetimes are adapted from JFG upstream
 * efd5abb's corresponding src/models.c function, func_8003E13C. JFG retains
 * that function as GLOBAL_ASM; Mickey's own bytes and behavior are authority.
 * Matched 2026-10-01: three scalar webs decide it. The cached texture id is an
 * s32 copy taken only in the aging loop; the eviction loop reads the table
 * directly so it does not extend that copy's web into a second colouring
 * class; and the free-slot search and eviction scan are `for` loops whose
 * `slot`/`bestCount` initialisers sit where they are used. */
void func_80020B10(Gfx **displayList, s8 *textureIds, s8 *slots,
                   ModelTextureUsage *usage, s32 entryIndex, u32 textureBase) {
    ModelTextureUsageEntry *entry;
    s32 usageIndex;
    s32 slot;
    s32 bestCount;
    s32 textureIndex;
    s32 i;
    s8 *slotCursor;
    s8 *textureIdCursor;
    s32 cachedId;

    for (i = 0; i < 3; i++) {
        if (D_800CB498[i] != -1) {
            cachedId = D_800CB498[i];
            D_800CB49C[i]--;
            if (D_800CB49C[i] <= 0) {
                D_800CB49C[i] = 1;
                for (usageIndex = entryIndex + 1; usageIndex < usage->entryCount; usageIndex++) {
                    entry = &usage->entries[usageIndex];
                    if (cachedId == entry->textureIds[0] ||
                        cachedId == entry->textureIds[1] ||
                        cachedId == entry->textureIds[2]) {
                        usageIndex = usage->entryCount;
                    } else {
                        D_800CB49C[i]++;
                    }
                }
            }
        }
    }

    textureIndex = 0;
    slotCursor = slots;
    textureIdCursor = textureIds;
    do {
        *slotCursor = -1;
        textureIndex++;
        i = 0;
        if (*textureIdCursor != -1) {
            do {
                if (D_800CB498[i] == *textureIdCursor) {
                    *slotCursor = i + 1;
                }
                i++;
            } while (i < 3);

            if (*slotCursor == -1) {
                slot = -1;
                i = 0;
                for (; i < 3 && slot == -1; i++) {
                    if (D_800CB498[i] == -1) {
                        slot = i;
                    }
                }

                if (slot == -1) {
                    for (i = 0, bestCount = 0; i < 3; i++) {
                        if (D_800CB498[i] != textureIds[0] &&
                            D_800CB498[i] != textureIds[1] &&
                            D_800CB498[i] != textureIds[2]) {
                            if (bestCount < D_800CB49C[i]) {
                                bestCount = D_800CB49C[i];
                                slot = i;
                            }
                        }
                    }
                }

                D_800CB498[slot] = *textureIdCursor;
                D_800CB49C[slot] = 0;
                *slotCursor = slot + 1;
                gSPMatrix((*displayList)++, ((*textureIdCursor << 6) + textureBase) & 0x0FFFFFFF,
                          *slotCursor | 0x80);
            }
        }
        slotCursor++;
        textureIdCursor++;
    } while (textureIndex != 3);
}
typedef struct ModelFrameEntry {
    s16 frame;
    s16 nextFrame;
    u32 textureIndex;
} ModelFrameEntry;

typedef struct ModelTextureHeader {
    u8 pad0[0xE];
    u16 frameScale;
    u16 frameCount;
} ModelTextureHeader;

typedef struct ModelFrameInstance {
    ObjectModel *model;
    u8 pad4[6];
    s16 outputIndex;
    u8 padC[0x40];
    ModelFrameEntry *entries;
    u16 *outputs[1];
} ModelFrameInstance;

/* PROVENANCE: the authorized audit of JFG upstream efd5abb confirms that its
 * corresponding modSetTextureFrame remains GLOBAL_ASM, so no donor C body is
 * adopted here. This remains a Mickey-only reconstruction. */
/* Matched as a plain counted-down while loop: the post-decrement is the loop
 * test, the entry cursor advances last, the frame and next-frame values are
 * s16 locals, and the frame scale is widened into an s32 local, which is
 * what keeps the shifted next-frame value out of a coloured web.
 * ORT 374 authenticates eight overlay calls across overlays 57, 60, and 82;
 * resident func_8001BB10 passes an unused fourth owner/context argument that
 * this callee overwrites. */
void func_80020D8C(ModelFrameInstance *instance, s32 textureIndex, s32 frame) {
    ObjectModel *model;
    ModelFrameEntry *entry;
    u16 *output;
    s32 remaining;

    model = instance->model;
    entry = instance->entries;
    output = instance->outputs[instance->outputIndex];
    remaining = model->textureAnimationCount;
    while (remaining--) {
        s32 index = entry->textureIndex & 0xFF;
        ModelTextureHeader *texture = model->textures[index].texture;
        s16 nextFrame;
        s32 frameScale;
        s16 value;

        if (index == textureIndex && frame < texture->frameCount) {
            entry->frame = frame;
        }
        frameScale = texture->frameScale;
        value = entry->frame;
        nextFrame = entry->nextFrame;
        *output++ = (value >> 8) * frameScale;
        if (nextFrame >= 0) {
            *output++ = (nextFrame >> 8) * frameScale;
        }
        entry++;
    }
}
/* PROVENANCE: the authorized audit of JFG upstream efd5abb confirms that its
 * corresponding modSuspendModelTextures remains GLOBAL_ASM, so no donor C
 * body is adopted here. This remains a Mickey-only reconstruction. */
/* Matched 2026-09-16 (lane nx-b), 24 -> 0 masked words at delta 0 in six
 * measured batches, no colour force:
 *   - the exception scan subscripts the array (`exceptions[i]`) instead of
 *     the byte-offset spelling, so uopt strength-reduces it into the
 *     target's stepping cursor and the parameter web falls to s7 (the
 *     s6/s7 swap closes; the object is four bytes short, first +0x64).
 *   - the scan is a `while`, not a guarded `do`: uopt's rotated form keeps
 *     the index as a variable read by the cursor's preheader init, which
 *     as1 then prints as the target's scaled zero (`sll zero,1`) and the
 *     value-first `bnel`; the guard load stays a separate ring temp.
 *   - the cache address is scaled twice, `(modelIndex << 1) << 2`: ugen
 *     draws a ring temp for each shift and as1 folds the pair into the
 *     single `sll` the target shows, with the draw still spent (L149).
 *     That one folded draw is the whole ten-register ring rotation
 *     downstream of the model-loop head (34 -> 6).
 *   - one index `i` serves both the exception scan and the texture loop:
 *     a separate scan index is a two-block web (save 10) that is coloured
 *     a2 ahead of the D_80079C08 value web (save 6.4), where the target
 *     has that value in a2; merged into the texture index it is s1 and
 *     its dead init vanishes.
 *   - no cache pointer: the id read through the global expression is
 *     numbered after the scanned value, which is what puts the value
 *     first in the `bnel` (6 -> 0 together with the shared index). */
void func_80020E4C(s16 *exceptions) {
    SuspendedModelTexture *saved;
    s32 modelIndex;
    s32 i;

    D_80079C08 = 0;
    saved = D_80079C04 = func_8002B280(0x3E8, 0x8A);
    modelIndex = 0;
    if (D_800CB48C > 0) {
        if (D_80079C08 < 0x7D) {
            do {
                if (*(s32 *)((u8 *)D_800CB484 + ((modelIndex << 1) << 2)) != -1) {
                    ObjectModel *model = *(ObjectModel **)((u8 *)D_800CB484 + ((modelIndex << 1) << 2) + 4);
                    s32 excluded = 0;

                    i = 0;
                    while (exceptions[i] != -1 && excluded == 0) {
                        if (exceptions[i] == *(s32 *)((u8 *)D_800CB484 + ((modelIndex << 1) << 2))) {
                            excluded = 1;
                        }
                        i++;
                    }
                    if (excluded == 0) {
                        i = 0;
                        if (model->numberOfTextures > 0) {
                            if (D_80079C08 < 0x7D) {
                                do {
                                    saved->value = (s32)model->textures[i].texture;
                                    saved->id = func_8003484C(model->textures[i].texture);
                                    func_800347A0(model->textures[i].texture);
                                    i++;
                                    D_80079C08++;
                                    saved++;
                                } while (i < model->numberOfTextures && D_80079C08 < 0x7D);
                            }
                        }
                    }
                }
                modelIndex++;
            } while (modelIndex < D_800CB48C && D_80079C08 < 0x7D);
        }
    }
}
/*
 * PROVENANCE -- name and TU position follow JFG's public
 * modResumeModelTextures symbol. JFG has no public C body; Mickey is the body
 * and global-layout authority.
 */
void modResumeModelTextures(void) {
    SuspendedModelTexture *saved = D_80079C04;

    if (saved != NULL) {
        SuspendedModelTexture *entry = saved;
        s32 i = 0;
        if (D_80079C08 > 0) {
            do {
                if (entry->value != 0) {
                    texLoadTextureAddr(entry->id, entry->value);
                }
                i++;
                entry++;
            } while (i < D_80079C08);
        }
        mmFree(D_80079C04);
        D_80079C08 = 0;
    }
}

typedef struct ModelPointRecord {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[4];
} ModelPointRecord;

typedef struct ModelPointIndex {
    u16 pointIndex;
    u16 pad2;
} ModelPointIndex;

typedef struct ModelPointSource {
    u8 pad0[0x1C];
    ModelPointRecord *points;
    u8 pad20[0xD];
    u8 pointCount;
    u8 pad2E[2];
    ModelPointIndex *indices;
} ModelPointSource;

typedef struct ModelPointOutput {
    ModelPointSource *source;
    u8 pad4[0x3C];
    f32 *points;
} ModelPointOutput;

typedef struct ModelPointOwner {
    u8 pad0[0x68];
    ModelPointOutput **output;
} ModelPointOwner;

void func_8002AA50(void *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);

/* Mickey-only reconstruction; JFG's candidate model helpers remain assembly. */
void func_8002109C(ModelPointOwner *owner) {
    MtxF matrix;
    ModelPointOutput *output;
    ModelPointSource *source;
    ModelPointRecord *point;
    f32 *outputPoint;
    s32 i;

    output = *owner->output;
    source = output->source;
    func_8002AA50(owner, matrix);
    outputPoint = output->points;
    /* IDO's zero-initialization register schedule depends on this line grouping. */
    i = 0; if (source->pointCount > 0) { do {
            point = &source->points[source->indices[i].pointIndex];
            mtxf_transform_point(matrix, point->x, point->y, point->z,
                                 outputPoint, outputPoint + 1, outputPoint + 2);
            i++;
            outputPoint += 3;
        } while (i < source->pointCount);
    }
}

/* PLATEAU-HANDOFF:func_8001FC50:start
 * symbol: func_8001FC50
 * score: 277 differing words
 * frame: 0x78
 * relocations: 3
 * first-mismatch: +0x18
 * summary: Delta 0, frame 0x78; naming left: target keeps modeBytes homed in both arms and gives pointBytes ra, here modeBytes takes ra
 * PLATEAU-HANDOFF:func_8001FC50:end
 */
