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
extern volatile Overlay10Entry *gOverlay10Entries;
extern Overlay10Resource *gOverlay10Resources;
extern Overlay10Loaded *gOverlay10Loaded;
extern void *gOverlay10DataB;
extern void *gOverlay10DataC;
extern u8 *gOverlay10Buffers[4];
extern u8 gOverlay10Flag0;
extern u8 gOverlay10Flag1;
extern u8 gOverlay10Flag2;
extern u8 D_140[];
extern u8 D_400[];
extern u8 D_10[];

extern void overlay10GetDimensionsReloc(s32 *width, s32 *height);
extern void *overlay10AllocateReloc();
extern void *overlay10GetResourcesReloc();
extern void overlay10LoadReloc();
extern void overlay10ReleaseReloc();
extern void overlay10FinishReloc(void);

/* Pinned DKR v77/v80 and JFG scans contain no exact donor for this initializer.
 * This TU needs -Wo,-loopunroll,0: default unroll grows .text by 160 bytes.
 * leftover offset |= 0 keeps the entry-loop compare as slti 4096. */
#ifdef NON_MATCHING
void overlay10Initialize(void) {
    s32 width;
    s32 height;
    s32 widthValue;
    s32 heightValue;
    Overlay10Viewport *viewport;
    Overlay10Resource *resource;
    Overlay10Loaded *loaded;
    u8 **buffer;
    u8 **bufferEnd;
    s32 offset;
    s32 angle;

    overlay10GetDimensionsReloc(&width, &height);
    widthValue = width;
    heightValue = height;
    viewport = gOverlay10Viewports;
    do {
        viewport++;
        viewport[-1].left = 0;
        viewport[-1].top = 0;
        viewport[-1].right = 0;
        viewport[-1].bottom = 0;
        viewport[-1].minX = widthValue - 1;
        viewport[-1].minY = heightValue - 1;
        viewport[-1].width = widthValue;
        viewport[-1].height = heightValue;
        viewport[-1].mode0 = 0;
        viewport[-1].mode1 = 0;
        viewport[-1].mode2 = 0;
        viewport[-1].mode3 = 0;
        viewport[-1].color0[0] = 0xFF;
        viewport[-1].color0[1] = 0xFF;
        viewport[-1].color0[2] = 0xFF;
        viewport[-1].color0[3] = 0;
        viewport[-1].color1[0] = 0xFF;
        viewport[-1].color1[1] = 0xFF;
        viewport[-1].color1[2] = 0xFF;
        viewport[-1].color1[3] = 0;
        viewport[-1].color2[0] = 0xFF;
        viewport[-1].color2[1] = 0;
        viewport[-1].value0 = 0;
        viewport[-1].value1 = 0;
        viewport[-1].value2 = 0;
        viewport[-1].pointer = 0;
    } while (viewport < (Overlay10Viewport *)D_140);

    viewport = (Overlay10Viewport *)gOverlay10Descriptors;
    do {
        viewport = (Overlay10Viewport *)((u8 *)viewport -
                                         (-(sizeof(Overlay10Descriptor))));
        ((Overlay10Descriptor *)viewport)[-1].marker = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].pointer = 0;
        ((Overlay10Descriptor *)viewport)[-1].color0[0] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color0[1] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color0[2] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color0[3] = 0;
        ((Overlay10Descriptor *)viewport)[-1].color1[0] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color1[1] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color1[2] = 0xFF;
        ((Overlay10Descriptor *)viewport)[-1].color1[3] = 0;
        ((Overlay10Descriptor *)viewport)[-1].tail = 0;
    } while ((Overlay10Descriptor *)viewport < (Overlay10Descriptor *)D_400);

    gOverlay10LargeBlock = overlay10AllocateReloc(0x10010, 0x86);
    gOverlay10Entries = overlay10AllocateReloc(0x1000, 0x86);
    gOverlay10LargeBlock += 0x10;
    angle = 0;
    offset = 0;
    do {
        offset |= 0;
        ((volatile Overlay10Entry *)((u8 *)gOverlay10Entries + offset))->marker = 0xFF;
        ((volatile Overlay10Entry *)((u8 *)gOverlay10Entries + offset))->state0 = 0;
        ((volatile Overlay10Entry *)((u8 *)gOverlay10Entries + offset))->angle = angle;
        angle = angle + 0x100;
        ((volatile Overlay10Entry *)((u8 *)gOverlay10Entries + offset))->state1 = 0;
        offset += sizeof(Overlay10Entry);
    } while (offset < 0x1000);

    gOverlay10Resources = overlay10GetResourcesReloc(0x38);
    gOverlay10Loaded = overlay10AllocateReloc(0x200, 0x86);
    gOverlay10DataB = overlay10AllocateReloc(0x200, 0x86);
    gOverlay10DataC = overlay10AllocateReloc(0x200, 0x86);

    loaded = gOverlay10Loaded;
    buffer = gOverlay10Buffers;
    bufferEnd = (u8 **)D_10;
    resource = gOverlay10Resources;
    do {
        *buffer = overlay10AllocateReloc(0x400, 0x86);
        offset = 0;
        do {
            overlay10LoadReloc(0x39, loaded,
                               resource->data + offset * resource->stride, 0x20);
            (*buffer)[offset] = loaded->value;
            offset++;
        } while (offset != 0x100);
        overlay10ReleaseReloc(resource, 1);
        buffer++;
        resource++;
    } while (buffer != bufferEnd);

    gOverlay10Flag0 = 0;
    gOverlay10Flag1 = 0;
    gOverlay10Flag2 = 0;
    overlay10FinishReloc();
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o010/overlay10Initialize/func_overlay_010_F0000000_1868450.s")
#endif

/* PLATEAU-HANDOFF:overlay10Initialize:start
 * symbol: overlay10Initialize
 * score: 37/172 words
 * frame: 0x68
 * relocations: 41
 * first-mismatch: +0x0
 * summary: hoist into a1/a2 without extra s32 homes; spellings param-pin, register-copy, OR-zero; stall params 173, register inert 37/0x68, OR-zero deleted reloads +8
 * PLATEAU-HANDOFF:overlay10Initialize:end
 */
