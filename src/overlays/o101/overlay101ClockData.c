#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "overlays/overlay_101.h"

/* Tier B: the clock's DMA-triangle consumer and vertex indices establish
 * the same flag/index/UV record layout used by Mickey's other renderers. */
typedef struct Overlay101ClockTriangle {
    u8 flags;
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    s16 s0;
    s16 t0;
    s16 s1;
    s16 t1;
    s16 s2;
    s16 t2;
} Overlay101ClockTriangle;

/* Tier D: one bounded storage view of initialized data +0x230..+0x340.
 * The renderers prove the individual fields and their extents; this grouping
 * does not claim to recover the original C declaration. */
typedef struct Overlay101ClockStorage {
    Gfx textureState[9];
    Overlay101ClockPoint points[12];
    Overlay101ClockTriangle triangles[6];
    s32 ticks;
    void *texture;
} Overlay101ClockStorage;

typedef char O101ClockPointerWidth[(sizeof(void *) == 4) ? 1 : -1];
typedef char O101ClockIntegerWidth[(sizeof(s32) == 4) ? 1 : -1];
typedef char O101ClockPointSize[(sizeof(Overlay101ClockPoint) == 8) ? 1 : -1];
typedef char O101ClockTriangleSize[(sizeof(Overlay101ClockTriangle) == 16) ? 1 : -1];
typedef char O101ClockStorageSize[(sizeof(Overlay101ClockStorage) == 0x110) ? 1 : -1];

/* The full RDP othermode command includes its four-bit blend-mask field. */
#define O101_CLOCK_OTHER_MODE(high, low) \
    {{_SHIFTL(G_RDPSETOTHERMODE, 24, 8) | (high), (low)}}

/* Tier A: the configured compiler supplies the complete physical slice.
 * Existing text relocation names remain until the full module owns its data;
 * this object adds no new text binding or address conversion. */
Overlay101ClockStorage o101ClockStorage = {
    {
        gsDPPipeSync(),
        gsDPSetTextureLOD(G_TL_TILE),
        gsDPSetTextureLUT(G_TT_NONE),
        gsDPSetAlphaCompare(G_AC_NONE),
        gsSPClearGeometryMode(G_ZBUFFER | G_FOG),
        gsDPPipeSync(),
        gsDPSetCombineMode(G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM),
        O101_CLOCK_OTHER_MODE(G_TC_FILT | _SHIFTL(15, G_MDSFT_BLENDMASK, 4),
                             G_RM_XLU_SURF | G_RM_XLU_SURF2),
        gsSPEndDisplayList(),
    },
    {
        {-2.5f, 0.0f}, {-2.5f, 10.0f}, {2.5f, 0.0f}, {2.5f, 10.0f},
        {-2.5f, 0.0f}, {-2.5f, 15.0f}, {2.5f, 0.0f}, {2.5f, 15.0f},
        {-1.5f, 0.0f}, {-1.5f, 15.0f}, {1.5f, 0.0f}, {1.5f, 15.0f},
    },
    {
        {0x40, 0, 1, 2, 0, 0, 0, 1023, 255, 0},
        {0x40, 1, 2, 3, 0, 1023, 255, 0, 255, 1023},
        {0x40, 4, 5, 6, 0, 0, 0, 1023, 255, 0},
        {0x40, 5, 6, 7, 0, 1023, 255, 0, 255, 1023},
        {0x40, 8, 9, 10, 0, 0, 0, 1023, 255, 0},
        {0x40, 9, 10, 11, 0, 1023, 255, 0, 255, 1023},
    },
    90000,
    NULL,
};

#undef O101_CLOCK_OTHER_MODE
