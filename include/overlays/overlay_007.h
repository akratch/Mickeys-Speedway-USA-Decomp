#ifndef OVERLAY_007_H
#define OVERLAY_007_H

#include "PR/ultratypes.h"

typedef struct Overlay7Owner {
    u8 pad00[0x64];
    s8 *priority;
} Overlay7Owner;

typedef struct Overlay7Entry {
    void *owner;
    s32 field04;
    u16 value;
    u8 type;
    u8 active;
    struct Overlay7Entry *nested;
    struct Overlay7Entry *next;
} Overlay7Entry;

typedef struct Overlay7ModeRecord {
    u16 first;
    u16 second;
    u8 mode;
    u8 pad5;
} Overlay7ModeRecord;

typedef struct Overlay7ModeState {
    u8 pad000[1];
    s8 index;
    u8 pad002[0x381];
    u8 variant;
    u8 pad384[1];
    u8 alternate;
    u8 pad386[0x2D];
    u8 timer;
    u8 pad3B4[0x14];
    f32 height;
} Overlay7ModeState;

typedef struct Overlay7ModeOwner {
    u8 pad00[0x64];
    Overlay7ModeState *state;
} Overlay7ModeOwner;

typedef struct Overlay7CheckEntry {
    s32 value;
    u8 pad04[4];
} Overlay7CheckEntry;

typedef struct Overlay7CheckState {
    u8 pad000[0x1A8];
    u16 flags1A8;
    u8 pad1AA[0x1D9];
    s8 field383;
    u8 pad384;
    u8 field385;
    u8 pad386[0x7A];
    s32 field400;
    s32 limits404[3];
} Overlay7CheckState;

typedef struct Overlay7CheckOwner {
    u8 pad00[0x64];
    Overlay7CheckState *state;
} Overlay7CheckOwner;

typedef struct Overlay7DispatchState {
    u8 pad00[1];
    s8 index;
    u8 pad02[0x45B];
    u8 field45D;
} Overlay7DispatchState;

typedef struct Overlay7DispatchOwner {
    u8 pad00[0x64];
    Overlay7DispatchState *state;
} Overlay7DispatchOwner;

typedef struct Overlay7Pair {
    u16 key;
    u16 value;
} Overlay7Pair;

extern Overlay7Entry *gOverlay7Current;
#ifndef OVERLAY_007_DEFINE_BSS
extern Overlay7Entry *gOverlay7ActiveHead;
extern Overlay7Entry *gOverlay7FreeHead;
extern Overlay7Entry *gOverlay7ActiveTail;
extern Overlay7Entry *gOverlay7Selected;
#endif
extern s32 gOverlay7PriorityThresholdReloc;
extern s32 D_8;
/* Runtime SYMBOL ORT 1579 resolves this proxy to resident BSS D_800D3128. */
extern u32 gOverlay7DispatchFlagsReloc;
extern u16 gOverlay7DispatchOverride[32];
extern u16 gOverlay7DispatchValues[][30];
extern u8 gOverlay7DispatchTypes[];
extern s8 gOverlay7DispatchMap[];
extern Overlay7ModeRecord gOverlay7PrimaryModes[][10];
extern Overlay7ModeRecord gOverlay7AlternateModes[][10];
/* Runtime ORT 1579 resolves this byte proxy to resident D_800D3128 + 2. */
extern u8 gOverlay7DispatchModeReloc;
extern u8 gOverlay7DispatchData[];
extern void *gOverlay7DispatchObject;
extern void *gOverlay7CommitArgument;
extern s16 gOverlay7ValuesEnd;

extern Overlay7Entry *overlay7Acquire(void *owner, u16 value, u8 type);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern u8 *overlay7GetModeReloc(void);
extern void *overlay7GetCheckTableReloc(void);
extern void *overlay7GetCurrentReloc(void);
extern s32 overlay7GetCheckIndexReloc(void *current);
extern void overlay7RecordCheckReloc(s32 value);
extern void overlay7SetOwnerModeReloc(Overlay7CheckOwner *owner, s32 mode);
extern s32 camGetModeReloc(void);
extern void overlay59AppendValueReloc(s32 arg0, s32 index, s32 value,
                                      s32 field);
extern void amSndStopXYZ(void *object);
extern void amSndPlay(u16 value, void **argument);

void overlay7ReleaseEntry(Overlay7Entry *entry);
Overlay7Entry *overlay7AcquireEntry(Overlay7Owner *owner, u16 value, u8 type);
void overlay7CreateEntry(void *owner, u16 value, u8 type);
void overlay7AppendEntry(void *owner, u16 value, u8 type);
void overlay7DispatchModes(Overlay7ModeOwner *first, Overlay7ModeOwner *second);
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous);
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection);
void overlay7CommitSelection(s32 selection);
s32 overlay7FillValues(s16 *value);
void overlay7InitPool(void);

#endif
