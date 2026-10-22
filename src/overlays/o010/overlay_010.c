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

extern Overlay10Viewport D_800D64E8[8];
extern Overlay10Descriptor D_800D60E8[32];
extern u8 *D_800D6638;
extern Overlay10Entry *D_800D663C;
extern Overlay10Resource *D_800D60E4;
extern Overlay10Loaded *D_800D6640;
extern void *D_800D6644;
extern void *D_800D6648;
extern u8 *D_800D6628[4];
extern u8 D_800D60E0;
extern u8 D_800D664C;
extern u8 D_800D664D;

extern void viGetCurrentSize(s32 *width, s32 *height);
extern void *mmAlloc();
extern void *piRomLoad();
extern void piRomLoadSection();
extern void fontCreateDisplayList();
extern void overlay45ResetState(void);

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

    viGetCurrentSize(&width, &height);
    for (i = 0; i < 8; i++) {
        D_800D64E8[i].left = 0;
        D_800D64E8[i].top = 0;
        D_800D64E8[i].right = 0;
        D_800D64E8[i].bottom = 0;
        D_800D64E8[i].minX = width - 1;
        D_800D64E8[i].minY = height - 1;
        D_800D64E8[i].width = width;
        D_800D64E8[i].height = height;
        D_800D64E8[i].mode0 = 0;
        D_800D64E8[i].mode1 = 0;
        D_800D64E8[i].mode2 = 0;
        D_800D64E8[i].mode3 = 0;
        D_800D64E8[i].color0[0] = 0xFF;
        D_800D64E8[i].color0[1] = 0xFF;
        D_800D64E8[i].color0[2] = 0xFF;
        D_800D64E8[i].color0[3] = 0;
        D_800D64E8[i].color1[0] = 0xFF;
        D_800D64E8[i].color1[1] = 0xFF;
        D_800D64E8[i].color1[2] = 0xFF;
        D_800D64E8[i].color1[3] = 0;
        D_800D64E8[i].color2[0] = 0xFF;
        D_800D64E8[i].color2[1] = 0;
        D_800D64E8[i].value0 = 0;
        D_800D64E8[i].value1 = 0;
        D_800D64E8[i].value2 = 0;
        D_800D64E8[i].pointer = 0;
    }
    for (i = 0; i < 32; i++) {
        D_800D60E8[i].marker = 0xFF;
        D_800D60E8[i].pointer = 0;
        D_800D60E8[i].color0[0] = 0xFF;
        D_800D60E8[i].color0[1] = 0xFF;
        D_800D60E8[i].color0[2] = 0xFF;
        D_800D60E8[i].color0[3] = 0;
        D_800D60E8[i].color1[0] = 0xFF;
        D_800D60E8[i].color1[1] = 0xFF;
        D_800D60E8[i].color1[2] = 0xFF;
        D_800D60E8[i].color1[3] = 0;
        D_800D60E8[i].tail = 0;
    }
    D_800D6638 = mmAlloc(0x10010, 0x86);
    D_800D663C = mmAlloc(0x1000, 0x86);
    D_800D6638 += 0x10;
    j = 0;
    for (i = 0; i < 256; i++) {
        D_800D663C[i].marker = 0xFF;
        D_800D663C[i].state0 = 0;
        D_800D663C[i].angle = j;
        j += 0x100;
        D_800D663C[i].state1 = 0;
    }
    D_800D60E4 = piRomLoad(0x38);
    D_800D6640 = mmAlloc(0x200, 0x86);
    D_800D6644 = mmAlloc(0x200, 0x86);
    D_800D6648 = mmAlloc(0x200, 0x86);
    loaded = D_800D6640;
    for (i = 0, resource = D_800D60E4; i < 4; i++, resource++) {
        D_800D6628[i] = mmAlloc(0x400, 0x86);
        for (j = 0; j < 256; j++) {
            piRomLoadSection(0x39, loaded, resource->data + j * resource->stride, 0x20);
            D_800D6628[i][j] = loaded->value;
        }
        fontCreateDisplayList(resource, 1);
    }
    D_800D60E0 = 0;
    D_800D664C = 0;
    D_800D664D = 0;
    overlay45ResetState();
}
