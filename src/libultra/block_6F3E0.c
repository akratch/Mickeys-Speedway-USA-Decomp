#include "PR/ultratypes.h"

/* verdict: structure-mismatch; target range is a 0x40-byte zero hole */
/* first mismatch: +0x0; an empty C function still emits a return sequence */
/* blocker: project docs classify 0x6F3E0-0x6F420 as unclaimed padding */
/* Re-tested 2026-09-11 (lane/p9-oneoff) against the return-island reading
 * that closed func_8005807C the same day, and the closure holds. The
 * aligner puts the surplus at candidate +0x0, one word: the empty body's
 * `jr $ra`. Its delay-slot `nop` is what matches the target's single word.
 * The discriminator is the byte value. At 0x800580E8 the eight surplus
 * bytes were a real `jr $ra` + `nop` and therefore a function; here the
 * whole 0x40-byte region is zero and the preceding routine's epilogue ends
 * exactly at the boundary, which is what padding looks like in this image
 * (compare 0x80058008, also zero-filled). Nothing in the 32MB image
 * references 0x8006E7E0 either way. No C body emits a lone zero word, so
 * the fix remains a segment reclassification in mickey.us.yaml, which is
 * shared configuration and belongs to an integration pass. */
#ifdef NON_MATCHING
void func_8006E7E0(void) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/libultra/block_6F3E0/func_8006E7E0.s")
#endif

/* PLATEAU-HANDOFF:func_8006E7E0:start
 * symbol: func_8006E7E0
 * score: 2 differing words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x0
 * summary: One zero padding word; every IDO body is at least two words. Needs a yaml reclassification, not C.
 * PLATEAU-HANDOFF:func_8006E7E0:end
 */
