#ifndef OVERLAY_059_H
#define OVERLAY_059_H

#include "PR/ultratypes.h"

/* Overlay 59: four animated value lists with per-entry textures. */

typedef struct Overlay59Value {
    u8 first;
    s8 second;
} Overlay59Value;

/* One list entry (0x44 bytes). */
typedef struct Overlay59Entry {
    f32 x;
    f32 y;
    Overlay59Value values[8];
    s16 count;
    s16 position;
    s16 timer;
    u16 state;
    void *owner;
    void *handles[8];
} Overlay59Entry;

/* overlay59Update reads an entry's state as signed. */
typedef struct Overlay59Record {
    u8 pad0[8];
    Overlay59Value values[8];
    s16 count;
    s16 field1A;
    s16 field1C;
    s16 state;
    u8 pad20[0x24];
} Overlay59Record;

/* The release walk's sliding view of an entry: `handle` is handles[i] as
 * the cursor steps four bytes at a time. */
typedef struct Overlay59State {
    u8 pad0[0x20];
    void *owner;
    void *handle;
} Overlay59State;

typedef struct Overlay59Descriptor {
    void *first;
    void *second;
    u8 pad08[8];
} Overlay59Descriptor;

typedef struct Overlay59DescriptorGroup {
    void ***descriptors;
    u32 unused;
} Overlay59DescriptorGroup;

typedef struct Overlay59TableInfo {
    s32 count;
    s32 unused;
} Overlay59TableInfo;

typedef struct Overlay59Node {
    s32 active;
    s32 includeSecond;
    s32 value8;
    s16 valueC;
    s16 valueE;
} Overlay59Node;

typedef struct Overlay59Output {
    s32 first;
    s32 second;
    s32 value8;
    s16 valueC;
    s16 valueE;
} Overlay59Output;

/* Module data, still owned by the raw image (phase 3). */
extern Overlay59Entry gOverlay59Entries[4];
extern Overlay59Entry gOverlay59EntriesOneEnd[];
extern Overlay59Entry gOverlay59EntriesEnd[];
extern Overlay59DescriptorGroup gOverlay59DescriptorTables[];
extern u8 *gOverlay59ValueTables[11];
extern Overlay59TableInfo gOverlay59TableInfo[];
extern s32 gOverlay59Styles[];

/* Resident functions. overlay59ReleaseReloc is the resident texture release
 * (ORT row 140). */
extern void overlay59ReleaseReloc(void *handle);
extern void *texLoadTexture(s32 textureId);
extern void fontUseFont(s32);
extern void fontBackground(s32, s32, s32, s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern void fontPrintXY(s32, s32, s32, s32, s32);

/* This module's exports (ORT rows). */
void overlay59ResetEntries(void);
void overlay59ReleaseAll(void);
void overlay59AppendValue(s32 index, s32 tableIndex, s32 offset, s32 mode);
void overlay59Update(s32 index);
void overlay59Advance(s32 steps);
s32 overlay59Interpolate(s32 index, s32 x, s32 y, s32 targetX, s32 targetY,
                         s32 *outX, s32 *outY, s32 useFirstScale);
void overlay59BuildList(s32 index, Overlay59Output *output);
void overlay59DrawFrame(s32 displayList, s32 index, s32 x, s32 y);

#endif
