#include "PR/ultratypes.h"

typedef struct Overlay57ModeObject {
    s32 value;
} Overlay57ModeObject;

typedef struct Overlay57ModeResult {
    u8 pad00[0x16];
    u8 flags16;
} Overlay57ModeResult;


/* Global so the promotion proof has a named canonical .bss ownership witness;
 * it sits at offset 0, is never referenced, and every other object in the
 * block stays static so its accesses keep their section-relative addends. */
u8 gOverlay57BssPad000[0x114];
static s32 gOverlay57Countdown;
static s32 gOverlay57Timer;
static s32 gOverlay57BssPad11C;
static s32 gOverlay57BssPad120;
static s32 gOverlay57BssPad124;
static s32 gOverlay57BssPad128;
static s32 gOverlay57BssPad12C;
static s32 gOverlay57BssPad130;
static s32 gOverlay57BssPad134;
static s32 gOverlay57BssPad138;
static s32 gOverlay57SetupStatus;
static s32 gOverlay57SetupDelay;
static s32 gOverlay57ModeFlag;
static s32 gOverlay57BssPad148;
static s32 gOverlay57BssPad14C;
static s32 gOverlay57BssPad150;
static s32 gOverlay57BssPad154;
static s32 gOverlay57BssPad158;
static s32 gOverlay57BssPad15C;
static s32 gOverlay57BssPad160;
static s32 gOverlay57BssPad164;
static s32 gOverlay57SetupValues[6];
static Overlay57ModeObject gOverlay57Object;
static s32 gOverlay57BssPad184;
static s32 gOverlay57TriggerLatched;
static s32 gOverlay57BssPad18C;
static s32 gOverlay57BssPad190;
static s32 gOverlay57ObjectStatus;
static u8 gOverlay57BssPad198[0x20];

/* Physical a0 retains updateRate at this site; consumption is not proven. */
extern void *overlay57TailQueryModeReloc(void);
extern s32 overlay57TailQueryChoiceReloc(s32 mode);
extern void overlay57TailPrepareObjectReloc(u8 id);
extern void overlay57TailStartObjectReloc(u8 id);
extern Overlay57ModeResult *overlay57TailFindObjectReloc(u8 id);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void overlay57AdvanceReloc(s32 updateRate);

/* Overlay 57 text +0x4C18..+0x4D90. Exact: 94 words, frame 0x28, 38/38
 * relocation identities.
 *
 * The setup array is one six-iteration constant loop. uopt unrolls it by four,
 * so it peels 6 % 4 = 2 iterations -- those fold to the literal stores of 0 and
 * 0x30 -- and emits a single unrolled body whose induction variable stays
 * register-resident at 2 rather than being propagated. A straight-line
 * reconstruction of the six stores cannot reach that shape. The body is on the
 * `for` line because as1 breaks scheduling ties on the source line, and a
 * braced body puts the unrolled copies on a different line than the peeled
 * ones.
 *
 * framePad is declared before `trigger` because the declaration order fixes
 * which frame cells the two spill homes take; declared after, the frame closes
 * at 0x20 instead of 0x28.
 *
 * The BSS block below is this TU's: IDO shares one `lui $at` across the two
 * constant-index setup stores only for a locally-defined symbol, and that
 * shared materialisation is in the shipped bytes. `static` makes every access
 * a section-relative record whose addend is already the module-relative offset
 * the shipped word carries, so the static link must not adjust it a second
 * time -- overlay 57's own runtime relocation table owns those 31 records, and
 * mk/overlays.mk drops them with filter_elf_relocations.py. IDO 4-aligns .bss
 * scalars and 8-aligns arrays, which is why the interior pads are scalars:
 * 0x13C, 0x144 and 0x194 are not 8-aligned. */
void overlay57UpdateModeTrigger(s32 updateRate) {
    s32 framePad[2];
    s32 trigger;

    gOverlay57ModeFlag = 1;

    if (gOverlay57Countdown > 0) {
        gOverlay57Countdown = gOverlay57Countdown - updateRate;
        if (gOverlay57Countdown <= 0) {
            trigger = 1;
        }
    }

    gOverlay57SetupStatus = 0;
    if (overlay57TailQueryModeReloc() == 0) {
        s32 index;
        s32 i;
        Overlay57ModeResult *result;

        index = 2;

        for (i = 0; i < 6; i++) gOverlay57SetupValues[i] = i * 0x30;

        trigger = 1;
        if (overlay57TailQueryChoiceReloc(index) == 5) {
            gOverlay57Timer = 10;
        }

        gOverlay57ObjectStatus = 0;
        gOverlay57Object.value = 80;
        overlay57TailPrepareObjectReloc(((u8 *)&gOverlay57Object)[3]);
        overlay57TailStartObjectReloc(((u8 *)&gOverlay57Object)[3]);
        result = overlay57TailFindObjectReloc(((u8 *)&gOverlay57Object)[3]);
        if (result != 0) {
            result->flags16 |= 2;
        }

        if (gOverlay57TriggerLatched != 0) {
            gOverlay57SetupDelay = 0;
        } else {
            gOverlay57SetupDelay = 60;
        }
    }

    /* Retail reads this deliberately uninitialized value when neither path
     * above assigns it. Initializing it changes the physical function. */
    if ((trigger != 0) && (gOverlay57TriggerLatched == 0)) {
        overlay57SetNodeValue(0x2F, 0, 0.012f);
        gOverlay57TriggerLatched = 1;
    }

    overlay57AdvanceReloc(updateRate);
}
