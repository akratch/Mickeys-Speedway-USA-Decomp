#ifndef OVERLAY_003_H
#define OVERLAY_003_H

#include "PR/ultratypes.h"
#include "game/math.h"

/* A racer's AI state block, as overlay 3's target selection reads it. */
typedef struct Overlay3State {
    u8 pad000[0x1];
    s8 group;
    u8 pad002[0x2];
    u16 flags;
    u8 pad006[0x194];
    u8 mode;
    u8 useWeightedSearch;
    u8 pad19C[0x1F1];
    u8 cachedIndex;
    u16 timer;
    u8 pad390[0x18];
    u8 weights[1];
    u8 pad3A9[0xD];
    s16 scoreHigh;
    s16 scoreLow;
    u8 pad3BA[0x14];
    s16 sameObjectTimer;
    u8 pad3D0[0x2A];
    s16 blocked;
} Overlay3State;

/* A racer object: position, two type words and its AI state. */
typedef struct Overlay3Object {
    s16 heading;
    u8 pad002[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x2C];
    s16 kind;
    s16 type;
    u8 pad048[0x1C];
    Overlay3State *state;
} Overlay3Object;

/* One remembered object and its timeout, 32 per group. */
typedef struct Overlay3Entry {
    void *object;
    s32 timer;
} Overlay3Entry;

extern Overlay3Entry gOverlay3Entries[][32];
extern Overlay3Entry gOverlay3TouchEntries[][32];
extern Overlay3Object *gOverlay3Objects[];
/* Per-consumer stored operands; the actual C data definitions keep their
 * unsuffixed names and final module identities. */
extern u8 gOverlay3ModeChance_o003Reloc[10];
extern f32 gOverlay3SearchMaxDistance_o003Reloc;

extern Overlay3Object **func_80005750(s32 *count);
extern Overlay3Object **func_8000572C(s32 *start, s32 *end);
extern f32 func_8000BD0C(f32, f32, f32, f32, f32, f32);
extern s32 mathDiffAngle(s16 heading, s32 angle);
extern s32 overlay1GetEntry(s32 one);
extern s32 func_overlay_002_F0001A94_185888C(f32 ax, f32 az, f32 bx, f32 bz, s32 angleValue, s32 zero, s32 minusOne, s32 mask);
extern s32 mathRnd(s32 low, s32 high);
extern f32 sqrtf(f32 value);
extern void overlay36SpawnDirectional(Overlay3Object *object);
extern void overlay36SpawnLinked7F(Overlay3Object *object);
extern void overlay36CallModeZero(Overlay3Object *object);
extern void overlay36TickState(Overlay3Object *object);

void overlay3TouchObject(s32 group, s32 remaining);
/*
 * SelectTarget's call to the exported overlay3TouchObject is a SYMBOL
 * relocation record, so the shipped word stores the 0xF0000000 addend rather
 * than the definition's offset; the call names this undefined placeholder
 * until the overlay tables are linker output. Unprototyped: the call passes
 * only the group, and the definition's second parameter is the loop count it
 * reuses as a local.
 */
extern void overlay3TouchObject_o003Reloc();

#endif
