/*
 * Resident boot/thread setup -- ROM 0x21DA0-0x21EE0.
 *
 * DKR's published src/main.c identifies this source unit and supplies the
 * first two bodies. Mickey's shorter stack-check variant is reconstructed
 * from its native u64 stack counters below.
 */

#include "ultra64.h"

extern u8 D_800CB5B0;
extern u64 D_800CB5B8[];
extern u8 D_800CD5B8;
extern OSThread D_800CD5C0;
extern OSThread D_800CD7F0;

void diCpuTraceInit(void);
void mainThread(void *unused);
void thread1_main(void *unused);

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published src/main.c::mainproc.
 * Mickey's compiled and linked function is independently byte-identical.
 */
void mainproc(void) {
    osInitialize();
    osCreateThread(&D_800CD5C0, 1, thread1_main, NULL, &D_800CB5B0, OS_PRIORITY_IDLE);
    osStartThread(&D_800CD5C0);
}

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published
 * src/main.c::thread1_main. Mickey's compiled and linked function is
 * independently byte-identical.
 */
void thread1_main(void *unused) {
    diCpuTraceInit();
    osCreateThread(&D_800CD7F0, 3, mainThread, NULL, &D_800CD5B8, 10);
    D_800CB5B8[0x400] = 0;
    D_800CB5B8[0] = 0;
    osStartThread(&D_800CD7F0);
    osSetThreadPri(NULL, OS_PRIORITY_IDLE);
    while (1) {}
}

void bootCheckStack(void) {
    D_800CB5B8[0x400]++;
    D_800CB5B8[0]++;
}
