#ifndef OVERLAY_020_H
#define OVERLAY_020_H

#include "ultra64.h"
#include "game/math.h"

typedef struct Overlay20State {
    u8 pad00[0x84];
    s32 handle;
} Overlay20State;

typedef struct Overlay20LookupResult {
    s16 x0;
    s16 pad02;
    s16 y0;
    s16 x1;
    s16 pad08;
    s16 y1;
} Overlay20LookupResult;

typedef struct Overlay20TrackEntry {
    void *owner;
    void *unk04;
} Overlay20TrackEntry;

typedef struct Overlay20Context {
    Overlay20TrackEntry *entries;
    u8 pad04[0x14];
    s16 count;
} Overlay20Context;

typedef struct Overlay20Config {
    u8 pad00[0xA];
    s8 useLookup;
    s8 entryIndex;
    u8 columns;
    u8 rows;
    u8 value0E;
    u8 value0F;
    s16 width;
    s16 height;
    u8 start;
    u8 current;
    u8 end;
    u8 scaleDivisor;
} Overlay20Config;

typedef struct Overlay20Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 lookupIndex;
    u8 pad30[4];
    f32 radius;
    u8 pad38;
    u8 alpha;
    u8 pad3A[0x2E];
    void **fallbackEntry;
    u8 pad6C[0x18];
    struct Overlay20InitGrid *resource;
} Overlay20Object;

typedef struct Overlay20InitVertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay20InitVertex;

typedef struct Overlay20TexCoords {
    s16 u;
    s16 v;
} Overlay20TexCoords;

typedef struct Overlay20Triangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    Overlay20TexCoords uv0;
    Overlay20TexCoords uv1;
    Overlay20TexCoords uv2;
} Overlay20Triangle;

typedef struct Overlay20InitTexture {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay20InitTexture;

typedef struct Overlay20Grid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    s16 textureScaleX;
    s16 textureScaleY;
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20InitVertex *buffers[2];
} Overlay20Grid;

/* Genuine nested views: the updater needs the grid prefix, the destructor
 * also owns triangles, and the allocator adds its texture reference. */
typedef struct Overlay20TreeNode {
    Overlay20Grid prefix;
    Overlay20Triangle *triangles;
} Overlay20TreeNode;

typedef struct Overlay20InitGrid {
    Overlay20TreeNode prefix;
    Overlay20InitTexture *texture;
} Overlay20InitGrid;

typedef Overlay20InitGrid Overlay20Resource;
typedef Overlay20InitGrid Overlay20TileSource;

typedef struct Overlay20Command {
    u32 w0;
    u32 w1;
} Overlay20Command;

typedef struct Overlay20Entry {
    f32 x;
    f32 y;
    f32 radius;
    f32 radiusSquared;
    s16 minX;
    s16 minY;
    s16 maxX;
    s16 maxY;
    s16 phase;
    s16 lifetime;
    f32 frequency;
    f32 radiusRatio;
} Overlay20Entry;

typedef struct Overlay20Vertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad9;
} Overlay20Vertex;

typedef struct Overlay20ReleaseOwner {
    u8 pad0[0x84];
    s32 entry;
    s32 marked;
} Overlay20ReleaseOwner;

typedef struct Overlay20NestedOwner {
    u8 pad00[0x84];
    void *nested;
    s32 marked;
} Overlay20NestedOwner;

typedef struct Overlay20Data {
    u8 pad00[0x24];
    f32 lifetimeScale;
} Overlay20Data;

typedef struct Overlay20Owner {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x6C];
    Overlay20Entry *entry;
} Overlay20Owner;

typedef struct Overlay20EntryConfig {
    u8 pad0[0xA];
    s16 heading;
    s16 mode;
    s16 scale;
    s16 value0;
    s16 value1;
} Overlay20EntryConfig;

typedef struct Overlay20RemoveOwner {
    u8 pad0[0x84];
    void *entry;
} Overlay20RemoveOwner;

typedef struct Overlay20TailVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay20TailVector;

typedef Overlay20Vertex Overlay20TailVertex;

typedef struct Overlay20TailGrid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    u8 pad0E[4];
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20TailVertex *buffers[2];
} Overlay20TailGrid;

/* The draw's private command type was this one. */
typedef Overlay20Command Overlay20Gfx;

extern s32 gOverlay20EntryCount;
extern Overlay20Entry *gOverlay20Entries[];
extern u32 gOverlay20ActiveBits;
extern Overlay20Entry gOverlay20Pool[];
extern u8 gOverlay20MarkerEnd;
extern Overlay20Data D_0;
extern f32 gOverlay20Scale;
extern f32 gOverlay20TailXLimit;
extern f32 gOverlay20TailYLimit;

/* Typed SYMBOL aliases preserve same-module runtime export calls.  The init
 * alias already has an authenticated sized function definition. */
extern void overlay20ReleaseTree_o020Reloc(Overlay20TreeNode *node);
extern Overlay20Resource *overlay20ConfigureResource_o020Reloc(
    Overlay20Resource *, s32, s32, s32, s32, s32, s32, s32,
    Overlay20InitTexture *, s32, s32, s32, s32, s32, s32);
extern Overlay20InitGrid *func_overlay_020_F000038C_1876964Reloc(Overlay20InitGrid *grid);
extern void overlay20BuildTileCommands_o020Reloc(Overlay20Gfx **gfxP,
                                               Overlay20TileSource *source, s32 mode);
/* Remaining ABI debt: the two integer guard views still use their original
 * proxy contracts.  Typed pointer views change their compiler register shape. */
extern void func_overlay_020_F0000000_18765D8Reloc();
extern void func_overlay_020_F0000A68_1877040Reloc(void *grid);
extern Overlay20Entry *overlay20ConfigureEntry_o020Reloc(
    Overlay20Entry *, f32, f32, f32, s32, f32, f32, f32);

extern void *mmAlloc(s32 size, u32 colourTag);
extern void mmFree(void *data);
extern Overlay20Context *trackGetTrack(void);
extern Overlay20LookupResult *func_8000FEEC(s16 index);
extern f32 sqrtf(f32 value);
extern void texDPTextureX(Overlay20Command **commands, void *resource,
                          s32 arg2, s32 arg3);

#endif
