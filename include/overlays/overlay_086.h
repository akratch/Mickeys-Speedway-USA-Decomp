#ifndef OVERLAY_086_H
#define OVERLAY_086_H

#include "PR/ultratypes.h"

typedef struct Overlay86Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay86Vec3f;

typedef struct Overlay86Resource {
    u8 pad00[0x2D];
    u8 active;
} Overlay86Resource;

typedef struct Overlay86Current {
    Overlay86Resource *resource;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *values[13];
    Overlay86Vec3f *position;
} Overlay86Current;

/* The module object's own state block. */
typedef struct Overlay86State {
    u8 mode;
    u8 targetIndex;
    s16 timer;
    Overlay86Vec3f spawn;
    u8 pad10[8];
    s32 loopSound;
    u8 pad1C[0x24];
    s32 effectSound;
} Overlay86State;

typedef struct Overlay86RingRecord {
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
    u16 pad0E;
    u16 flags;
} Overlay86RingRecord;

/* The state block of the racer object the module follows. */
typedef struct Overlay86RacerState {
    s8 ownerIndex;
    s8 vectorIndex;
    u8 pad002[0x37A];
    s16 selectedIndex;
    u8 pad37E[0x62];
    Overlay86RingRecord *record;
} Overlay86RacerState;

typedef struct Overlay86Object {
    s16 angle;
    s16 tilt;
    s16 roll;
    s16 flags;
    f32 scale;
    Overlay86Vec3f position;
    u8 pad18[0x16];
    s16 resultAngle;
    u8 pad30[0x14];
    s16 kind;
    u8 pad46[0x0A];
    void *context;
    u8 pad54[0x10];
    void *state; /* Overlay86State or Overlay86RacerState, by object kind */
    Overlay86Current **currentSlot;
    u8 pad6C[0x18];
    s32 ownerIndex;
} Overlay86Object;

typedef struct Overlay86Entry {
    u8 pad0[0xA];
    u8 type;
} Overlay86Entry;

extern Overlay86Vec3f gOverlay86Vectors[10];

/* Resident callees. */
/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this flow. */
extern void func_8005AF14(Overlay86Current *current, Overlay86Resource *resource,
                          Overlay86Object *owner);
extern void lightObject();
extern void pointListRPY(s32 mode, void *object, void *input, void *output);
extern s16 func_8000FAE0(f32 x, f32 y, f32 z);
extern Overlay86RingRecord *overlay1FindPreviousUsable(
    s32 index, s32 *selectedIndex);
extern s32 overlay1NextIndex(s32 index);
extern Overlay86Object **func_8000572C(s32 *start, s32 *end);

/* Exported. */
void overlay86ProcessCurrent(Overlay86Object *owner);
void overlay86ScaledVectorPosition(Overlay86Object *object,
                                   Overlay86RacerState *state, f32 *outX,
                                   f32 *outY, f32 *outZ);
void overlay86BuildTransform(Overlay86Object *object,
                             Overlay86Object *transform);
s16 overlay86SelectPosition(Overlay86Object *object, Overlay86State *output);
void overlay86Init(Overlay86Object *object, Overlay86Entry *entry);
void func_overlay_086_F0000474_18D22AC(void *object, s32 steps);

/* Same-module calls to exported functions are SYMBOL records in the shipped
 * table, whose call word stores 0, so they go through the module's
 * placeholders rather than the in-module definitions (CLAUDE.md: a
 * same-module call can still be a SYMBOL relocation record). */
void overlay86ProcessCurrent_o086Reloc(Overlay86Object *owner);
void overlay86ScaledVectorPosition_o086Reloc(Overlay86Object *object,
                                             Overlay86RacerState *state,
                                             f32 *outX, f32 *outY, f32 *outZ);
void overlay86BuildTransform_o086Reloc(Overlay86Object *object,
                                       Overlay86Object *transform);
s16 overlay86SelectPosition_o086Reloc(Overlay86Object *object,
                                      Overlay86State *output);

#endif /* OVERLAY_086_H */
