#include "PR/ultratypes.h"

typedef struct Overlay10Viewport {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    s16 minX;
    s16 minY;
    s16 width;
    s16 height;
    u8 mode0;
    u8 mode1;
    u8 mode2;
    u8 mode3;
    u8 color0[4];
    u8 color1[4];
    u8 color2[2];
    s16 value0;
    s16 value1;
    s16 value2;
    void *pointer;
} Overlay10Viewport;

typedef struct Overlay10Descriptor {
    u8 pad00;
    u8 marker;
    u8 pad02[2];
    void *pointer;
    u8 pad08[8];
    u8 color0[4];
    u8 color1[4];
    u8 pad18[4];
    s32 tail;
} Overlay10Descriptor;

typedef struct Overlay10Entry {
    u8 marker;
    u8 pad01;
    s16 angle;
    u8 state0;
    u8 state1;
    u8 pad06[10];
} Overlay10Entry;

typedef struct Overlay10Resource {
    u8 pad00[4];
    u8 *data;
    s16 stride;
    u8 pad0A[10];
} Overlay10Resource;

typedef struct Overlay10Loaded {
    u8 pad00[8];
    u8 value;
} Overlay10Loaded;

extern Overlay10Viewport gOverlay10Viewports[8];
extern Overlay10Descriptor gOverlay10Descriptors[32];
extern u8 *gOverlay10LargeBlock;
extern Overlay10Entry *gOverlay10Entries;
extern Overlay10Resource *gOverlay10Resources;
extern Overlay10Loaded *gOverlay10Loaded;
extern void *gOverlay10DataB;
extern void *gOverlay10DataC;
extern u8 *gOverlay10Buffers[4];
extern u8 gOverlay10Flag0;
extern u8 gOverlay10Flag1;
extern u8 gOverlay10Flag2;

extern void overlay10GetDimensionsReloc(s32 *width, s32 *height);
extern void *overlay10AllocateReloc();
extern void *overlay10GetResourcesReloc();
extern void overlay10LoadReloc();
extern void overlay10ReleaseReloc();
extern void overlay10FinishReloc(void);

/* Pinned DKR v77/v80 and JFG scans contain no exact donor for this initializer.
 * Matched 2026-10-01 by writing every loop as a plain subscript loop.  The
 * 37-word plateau walked declared pointers with stores through [-1], copied
 * width and height into two more locals so the stores could not alias them,
 * read the entry table through a volatile pointer, and needed the unroller
 * switched off for the file.  With `array[i]` stores uopt knows the targets
 * are the static arrays, hoists width and height without copies, builds the
 * end-pointer tests itself and declines to unroll, so the per-file flag is
 * gone.  `j` is both the angle accumulator of the entry loop and the inner
 * index of the load loop, which is why the angle takes a saved register.  The
 * resource cursor is initialised and stepped in the last loop's own header;
 * initialised on the line above, its load is scheduled three words early. */
void overlay10Initialize(void) {
    s32 i;
    s32 j;
    s32 width;
    s32 height;
    Overlay10Loaded *loaded;
    Overlay10Resource *resource;

    overlay10GetDimensionsReloc(&width, &height);
    for (i = 0; i < 8; i++) {
        gOverlay10Viewports[i].left = 0;
        gOverlay10Viewports[i].top = 0;
        gOverlay10Viewports[i].right = 0;
        gOverlay10Viewports[i].bottom = 0;
        gOverlay10Viewports[i].minX = width - 1;
        gOverlay10Viewports[i].minY = height - 1;
        gOverlay10Viewports[i].width = width;
        gOverlay10Viewports[i].height = height;
        gOverlay10Viewports[i].mode0 = 0;
        gOverlay10Viewports[i].mode1 = 0;
        gOverlay10Viewports[i].mode2 = 0;
        gOverlay10Viewports[i].mode3 = 0;
        gOverlay10Viewports[i].color0[0] = 0xFF;
        gOverlay10Viewports[i].color0[1] = 0xFF;
        gOverlay10Viewports[i].color0[2] = 0xFF;
        gOverlay10Viewports[i].color0[3] = 0;
        gOverlay10Viewports[i].color1[0] = 0xFF;
        gOverlay10Viewports[i].color1[1] = 0xFF;
        gOverlay10Viewports[i].color1[2] = 0xFF;
        gOverlay10Viewports[i].color1[3] = 0;
        gOverlay10Viewports[i].color2[0] = 0xFF;
        gOverlay10Viewports[i].color2[1] = 0;
        gOverlay10Viewports[i].value0 = 0;
        gOverlay10Viewports[i].value1 = 0;
        gOverlay10Viewports[i].value2 = 0;
        gOverlay10Viewports[i].pointer = 0;
    }
    for (i = 0; i < 32; i++) {
        gOverlay10Descriptors[i].marker = 0xFF;
        gOverlay10Descriptors[i].pointer = 0;
        gOverlay10Descriptors[i].color0[0] = 0xFF;
        gOverlay10Descriptors[i].color0[1] = 0xFF;
        gOverlay10Descriptors[i].color0[2] = 0xFF;
        gOverlay10Descriptors[i].color0[3] = 0;
        gOverlay10Descriptors[i].color1[0] = 0xFF;
        gOverlay10Descriptors[i].color1[1] = 0xFF;
        gOverlay10Descriptors[i].color1[2] = 0xFF;
        gOverlay10Descriptors[i].color1[3] = 0;
        gOverlay10Descriptors[i].tail = 0;
    }
    gOverlay10LargeBlock = overlay10AllocateReloc(0x10010, 0x86);
    gOverlay10Entries = overlay10AllocateReloc(0x1000, 0x86);
    gOverlay10LargeBlock += 0x10;
    j = 0;
    for (i = 0; i < 256; i++) {
        gOverlay10Entries[i].marker = 0xFF;
        gOverlay10Entries[i].state0 = 0;
        gOverlay10Entries[i].angle = j;
        j += 0x100;
        gOverlay10Entries[i].state1 = 0;
    }
    gOverlay10Resources = overlay10GetResourcesReloc(0x38);
    gOverlay10Loaded = overlay10AllocateReloc(0x200, 0x86);
    gOverlay10DataB = overlay10AllocateReloc(0x200, 0x86);
    gOverlay10DataC = overlay10AllocateReloc(0x200, 0x86);
    loaded = gOverlay10Loaded;
    for (i = 0, resource = gOverlay10Resources; i < 4; i++, resource++) {
        gOverlay10Buffers[i] = overlay10AllocateReloc(0x400, 0x86);
        for (j = 0; j < 256; j++) {
            overlay10LoadReloc(0x39, loaded, resource->data + j * resource->stride, 0x20);
            gOverlay10Buffers[i][j] = loaded->value;
        }
        overlay10ReleaseReloc(resource, 1);
    }
    gOverlay10Flag0 = 0;
    gOverlay10Flag1 = 0;
    gOverlay10Flag2 = 0;
    overlay10FinishReloc();
}
