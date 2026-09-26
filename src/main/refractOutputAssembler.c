#include "PR/ultratypes.h"

/*
 * PROVENANCE: control flow follows Jet Force Gemini's published
 * src/hasm/refractOutputAssembler.s. Mickey's own symbols and the linked
 * ROM remain authoritative. The Jet Force Gemini object is the same
 * extent and is not byte-identical.
 */
#ifdef NON_MATCHING
void refractOutputAssembler(void) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/refractOutputAssembler/refractOutputAssembler.s")
#endif
