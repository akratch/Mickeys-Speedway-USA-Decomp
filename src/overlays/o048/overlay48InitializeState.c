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
extern Overlay48S16At50 gOverlay48Timer;
extern Overlay48S16At54 gOverlay48Script;
extern Overlay48S16At58 gOverlay48ScriptIndex;
extern Overlay48S16At5A gOverlay48Finished;
extern void func_overlay_048_F0000000_1895408();

#ifdef NON_MATCHING
/* Bounded plateau: 10/57 positional words, first mismatch +0x0; candidate
 * 53 words versus target 57. Natural array/loop/BSS forms cannot retain the
 * target's unrolled index setup without changing its direct-store schedule. */
void overlay48InitializeState(void) {
    register volatile s16 *initial;
    s32 index;

    gOverlay48HeaderSeed.value = D_274.initial;
    gOverlay48HeaderLifetime.value = 0;
    gOverlay48HeaderActive.value = 0;
    gOverlay48HeaderHandle = 0;

    index = 1;
    D_10[0].lifetime = 0;
    D_10[0].active = 0;
    initial = &D_274.values[index];
    D_10[1].active = 0;
    D_10[1].lifetime = 0;
    D_10[0].handle = 0;
    D_10[0].seed = initial[0];
    D_10[2].active = 0;
    D_10[2].lifetime = 0;
    D_10[1].handle = 0;
    D_10[1].seed = initial[1];
    D_10[3].active = 0;
    D_10[3].lifetime = 0;
    D_10[2].handle = 0;
    D_10[2].seed = initial[2];
    D_10[3].handle = 0;
    D_10[3].seed = initial[3];

    gOverlay48Timer.value = 0;
    gOverlay48ScriptIndex.value = 0;
    gOverlay48Finished.value = 0;
    gOverlay48Script.value = D_174;
    func_overlay_048_F0000000_1895408(0x16);
    func_overlay_048_F0000000_1895408();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o048/overlay48InitializeState/func_overlay_048_F0000060_1895468.s")
#endif

/* PLATEAU-HANDOFF:overlay48InitializeState:start
 * symbol: overlay48InitializeState
 * score: 47 differing words
 * frame: 0x18
 * relocations: 24
 * first-mismatch: +0x0
 * summary: Hand-unroll folds index=1 to addiu+2; loops peel record 0. Missing sll+addu cursor and v0+=0x30. Colour refused at nonzero size.
 * PLATEAU-HANDOFF:overlay48InitializeState:end
 */
