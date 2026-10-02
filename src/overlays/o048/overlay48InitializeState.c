#include "PR/ultratypes.h"

typedef struct Overlay48Record {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    u8 active;
    u8 pad0B[5];
} Overlay48Record;

typedef struct Overlay48InitialSeeds {
    s16 values[5];
    u8 pad0A[0x26A];
    s16 initial;
} Overlay48InitialSeeds;

typedef struct Overlay48S16At4 {
    u8 pad00[4];
    s16 value;
} Overlay48S16At4;

typedef struct Overlay48S16At8 {
    u8 pad00[8];
    s16 value;
} Overlay48S16At8;

typedef struct Overlay48U8AtA {
    u8 pad00[0xA];
    u8 value;
} Overlay48U8AtA;

typedef struct Overlay48S16At50 {
    u8 pad00[0x50];
    s16 value;
} Overlay48S16At50;

typedef struct Overlay48S16At54 {
    u8 pad00[0x54];
    void *value;
} Overlay48S16At54;

typedef struct Overlay48S16At58 {
    u8 pad00[0x58];
    s16 value;
} Overlay48S16At58;

typedef struct Overlay48S16At5A {
    u8 pad00[0x5A];
    s16 value;
} Overlay48S16At5A;

/* Overlay 48's BSS is one six-record owner: a header at +0x0 followed by
 * five runtime entries at +0x10..+0x5F. Historical D_* aliases and the
 * indexed timer/script tail resolve within this block at load time. */
Overlay48Record gOverlay48Entries[6];

extern volatile Overlay48Record D_10[4];
extern Overlay48InitialSeeds D_274;
extern u8 D_174[];
extern Overlay48S16At4 gOverlay48HeaderLifetime;
extern Overlay48S16At8 gOverlay48HeaderSeed;
extern Overlay48U8AtA gOverlay48HeaderActive;
extern s32 gOverlay48HeaderHandle;
extern s16 gOverlay48Timer;
extern void *gOverlay48Script;
extern s16 gOverlay48ScriptIndex;
extern s16 gOverlay48Finished;
extern void func_overlay_048_F0000000_1895408();

/* Matched 2026-10-02. One loop over the five entries with the entry pointer
 * walking beside the index: IDO runs the remainder iteration first (entry 0,
 * constant addresses) and unrolls the other four, and because the index is
 * the secondary variable it stays a run-time 1 for the seed address. */
void overlay48InitializeState(void) {
    Overlay48Record *entry;
    s32 i;

    for (i = 0, entry = gOverlay48Entries; i < 5; i++, entry++) {
        entry->lifetime = 0;
        entry->active = 0;
        entry->seed = D_274.values[i];
        entry->handle = 0;
    }
    gOverlay48Timer = 0;
    gOverlay48ScriptIndex = 0;
    gOverlay48Finished = 0;
    gOverlay48Script = D_174;
    func_overlay_048_F0000000_1895408(0x16);
    func_overlay_048_F0000000_1895408();
}
