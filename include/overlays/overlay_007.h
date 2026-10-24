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

typedef struct Overlay7SelectionRow {
    u16 values[3];
} Overlay7SelectionRow;

typedef struct Overlay7RuntimeState {
    u8 pad000[1];
    s8 kind;
    u8 pad002[0x1A6];
    u16 flags;
    u8 pad1AA[0x1DB];
    u8 level;
    u8 previousLevel;
    u8 pad387[0x2C];
    u8 cooldown;
    u8 pad3B4[0x10];
    f32 difference;
    f32 scale;
} Overlay7RuntimeState;

typedef struct Overlay7RuntimeObject {
    u8 pad00C[0xC];
    s32 x;
    s32 y;
    s32 z;
    u8 pad018[0x4C];
    Overlay7RuntimeState *state;
} Overlay7RuntimeObject;

extern Overlay7Entry *gOverlay7Current;
extern s32 D_8007C1A0;
extern s32 D_8;
/* Runtime SYMBOL ORT 1579 resolves this proxy to resident BSS D_800D3128. */
extern u32 D_800D3128;
extern u16 gOverlay7DispatchOverride[10];
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
extern u8 *func_80028F54(void);
extern void *func_800291C4(void);
extern s32 levelGetNumber(void);
extern s32 levelGetBlurEffect(s32 levelIndex);
extern void overlay7RecordCheckReloc(s32 value);
extern void overlay7SetOwnerModeReloc(Overlay7CheckOwner *owner, s32 mode);
extern s32 camGetMode(void);
extern void overlay59AppendValue(s32 arg0, s32 index, s32 value,
                                      s32 field);
extern void amSndStopXYZ(void *object);
extern void amSndPlay(u16 value, void **argument);

extern u16 D_2AA;
extern Overlay7Pair D_844[];
extern void *D_800C947C;
extern void *overlay7RuntimeHandleReloc;
extern s32 overlay7RuntimeTimerReloc;
extern Overlay7RuntimeObject *overlay7RuntimeLastObjectReloc;
/* Tier B: ORT 1476 names this word view at overlay 1 BSS plus four.
 * Its backing remains the retained BSS reserve, not a C-owned allocation. */
extern s32 B_overlay_001_F00083E4;
extern u8 D_8007BEFC;
extern u8 D_8007BEF8;
extern u8 overlay7RuntimeValuesReloc[];

extern Overlay7RuntimeObject **func_80005750(s32 *count);
extern void amSndSetXYZ(void *handle, s32 x, s32 y, s32 z);
extern f32 overlay1DistanceFromSelected(Overlay7RuntimeObject *object);
extern f32 overlay1DistanceFromCurrent(Overlay7RuntimeObject *object);
extern void overlay7StartRuntimeValueReloc(u8 value);
extern void overlay7ReleaseRuntimeOwnerReloc(void *owner);
extern void *amSndPlayXYZ(u16 value, s32 x, s32 y, s32 z,
                          s32 kind, void **argument);
extern void overlay7ReleaseRuntimeHandleReloc(void *handle);
extern void overlay7SetRuntimeModeReloc(Overlay7RuntimeObject *object,
                                        s32 mode);
/*
 * The runtime update's call to the exported overlay7CommitSelection is a
 * SYMBOL relocation record: the shipped word stores the 0xF0000000 addend,
 * not the callee's offset, so it names this undefined placeholder until the
 * overlay tables are linker output.
 */
extern void overlay7CommitSelection_o007Reloc(s32 event);

Overlay7Entry *overlay7AcquireEntry(Overlay7Owner *owner, u16 value, u8 type);
void overlay7DispatchModes(Overlay7ModeOwner *first, Overlay7ModeOwner *second);
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous);
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection);
void overlay7CommitSelection(s32 selection);
s32 overlay7FillValues(s16 *value);
void overlay7InitPool(void);

#endif
