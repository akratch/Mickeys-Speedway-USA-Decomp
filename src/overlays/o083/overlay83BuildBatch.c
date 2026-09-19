#include "PR/ultratypes.h"

typedef struct O83SourceRecord {
    s8 x;
    s8 y;
    s8 z;
    s8 velocityX;
    s8 velocityY;
    s8 velocityZ;
    u8 red;
    u8 green;
    u8 blue;
    u8 scale;
    u8 halfLength;
    u8 alpha;
    u16 flags;
} O83SourceRecord;

typedef struct O83Source {
    u8 pad00[0xA];
    u8 count;
    u8 alpha;
    O83SourceRecord records[1];
} O83Source;

typedef struct O83LinkedInit {
    s8 mode;
    s8 count;
    s8 flags;
    s8 pad03;
    s16 x;
    s16 y;
    s16 z;
    s16 width;
    s16 height;
    s16 height2;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    s16 resource;
    s8 pad12;
    s8 pad13;
} O83LinkedInit;

typedef struct O83OutputRecord {
    s16 x;
    s16 y;
    s16 z;
    s16 height;
    f32 scale;
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    s16 halfLength;
    s16 velocityX;
    s16 velocityY;
    s16 velocityZ;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    f32 zero24;
    void *linked;
    u8 first;
    u8 last;
    u8 active;
    u8 enabled;
    u8 pad30[0x228];
} O83OutputRecord;

typedef struct O83Batch {
    u8 count;
    u8 pad01;
    s16 alpha;
    O83OutputRecord *records;
} O83Batch;

typedef struct O83Parent {
    u8 pad00[0xC];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x24];
    s32 clear3C;
    u8 pad40[0x24];
    O83Batch *batch;
} O83Parent;

extern void *overlay83AllocateBatchReloc();
extern void overlay83TransformWorldReloc();
extern void *overlay83CreateLinkedReloc();
extern f32 gOverlay83ScaleReloc;

/* Mickey-local reconstruction; pinned DKR/JFG scans found no exact donor. */
/* Frame 0x80 vs target 0x78. Six function-scope homes sit above linkedInit at
 * +0x50; target linkedInit is +0x58. Dummy-after-linkedInit slides the home
 * and grows the frame; reusing batch as remaining shrinks the frame and
 * leaves the home. scaleFactor keeps f22. */
#ifdef NON_MATCHING
void overlay83BuildBatch(O83Parent *parent, O83Source *source) {
    O83Batch *batch;
    O83OutputRecord *allocated;
    O83OutputRecord *output;
    O83SourceRecord *input;
    s32 remaining;
    f32 scaleFactor;

    batch = parent->batch;
    allocated = overlay83AllocateBatchReloc(source->count * 0x258, (void *)0x87);
    batch->records = allocated;
    if (allocated != 0) {
        batch->count = source->count;
        output = allocated;
        input = source->records;
        batch->alpha = source->alpha;
        remaining = source->count - 1;
        if (source->count != 0) {
            scaleFactor = gOverlay83ScaleReloc;
            do {
                output->x = input->x << 8;
                output->y = input->y << 8;
                output->z = input->z << 8;
                output->height = input->alpha * 4;
                output->worldX = 0.0f;
                output->worldZ = 0.0f;
                output->scale = input->scale * scaleFactor;
                output->worldY = output->height;
                overlay83TransformWorldReloc(1, output, &output->worldX, &output->worldX);
                output->worldX += parent->worldX;
                output->worldY += parent->worldY;
                output->worldZ += parent->worldZ;
                output->halfLength = input->halfLength;
                output->velocityX = input->velocityX * 0x10;
                output->velocityY = input->velocityY * 0x10;
                output->velocityZ = input->velocityZ * 0x10;
                output->red = input->red;
                output->green = input->green;
                output->blue = input->blue;
                output->zero24 = 0.0f;
                output->alpha = source->alpha;
                if (input->flags & 1) {
                    O83LinkedInit linkedInit;

                    linkedInit.mode = 1;
                    linkedInit.count = 3;
                    linkedInit.flags = input->flags;
                    linkedInit.pad03 = -1;
                    linkedInit.x = 0;
                    linkedInit.y = 0;
                    linkedInit.z = 0;
                    linkedInit.width = 0;
                    linkedInit.height = 0xC0;
                    linkedInit.height2 = 0xC0;
                    linkedInit.red = output->red;
                    linkedInit.green = output->green;
                    linkedInit.blue = output->blue;
                    linkedInit.alpha = output->alpha;
                    linkedInit.resource = -1;
                    linkedInit.pad12 = 0;
                    linkedInit.pad13 = 0;
                    output->linked = overlay83CreateLinkedReloc(0, &linkedInit);
                } else {
                    output->linked = 0;
                }
                output->first = 0;
                output->last = 0;
                output->active = 0;
                output->enabled = 1;
                input++;
                output++;
            } while (remaining--);
        }
    } else {
        batch->count = 0;
    }
    parent->clear3C = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o083/overlay83BuildBatch/func_overlay_083_F000053C_18CFCFC.s")
#endif

/* PLATEAU-HANDOFF:overlay83BuildBatch:start
 * symbol: overlay83BuildBatch
 * score: 98/168 words
 * frame: 0x80
 * relocations: 5
 * first-mismatch: +0x0
 * summary: o34 comma inert. dummy-after-linkedInit 91/0x88 home +0x58. batch-as-remaining frame 0x78 at 107. No size-0 net-zero slide.
 * PLATEAU-HANDOFF:overlay83BuildBatch:end
 */
