#include "PR/ultratypes.h"

typedef struct Overlay14Command { s16 pad0; s16 type; s32 value; } Overlay14Command;

extern s32 D_EC;
extern s32 D_E4;
extern s32 D_FC;
extern s32 gOverlay14ValueF8;
extern s32 gOverlay14ValueD8;
extern s32 gOverlay14ValueDC;
extern s32 gOverlay14ValueE0;
extern Overlay14Command D_128[];
extern void overlay14ReleaseOwner(void);
extern s32 func_overlay_014_F0000578_186FE50(s32);

/* PROVENANCE: Mickey-derived; pinned DKR v77/v80 and JFG scans found no exact
 * byte donor. A retained function-specific structural scan found no credible
 * near donor (best similarity 0.167).
 * A fidelity-gated scheduler trace exposed a physical-line tie in the prologue.
 * Keeping the function header, local declaration, and do header on one source
 * line produces exact size, frame 0x30, and 56/56 normalized instruction words.
 * Four raw words remain because the friendly data aliases carry zero addends;
 * relocation identity proof also remains pending at the +0x54 call alias. The
 * target runtime surface and candidate each contain 18 records at the expected
 * type/offset sites.
 * The complete 119-configuration flag lattice tied V0. A fidelity-gated IDO
 * allocator trace found seven integer webs (v0 twice and s0..s4). The native
 * scheduler trace passed text/data/rodata/relocation/symbol fidelity and
 * recorded 82 events and 49 ties; its prologue block identified the source-line
 * scheduling lever. An
 * explicit command-type carrier compiled identically; an explicit D_EC pointer
 * lifetime regressed to 55 words and frame 0x28. The two permitted natural
 * probes are exhausted. The remaining work is relocation identity/addend
 * closure, not instruction scheduling. The owned +0x498..+0x578 / ROM
 * 0x186FD70..0x186FE50 range has no padding or export and has five local inbound
 * JUMPs. */
void overlay14ResetMode(void) { Overlay14Command *command; do {
        if (D_EC <= 0) return;
        D_EC--;
        if (D_EC <= 0) return;
        overlay14ReleaseOwner();
        command = &D_128[D_EC - 1];
        D_E4 = command->type;
        if (command->type != 1) {
            gOverlay14ValueF8 = command->value;
            gOverlay14ValueD8 = 0;
            gOverlay14ValueDC = 0;
            gOverlay14ValueE0 = 0;
            return;
        }
        D_FC = command->value;
    } while (func_overlay_014_F0000578_186FE50(0) == 0);
}

