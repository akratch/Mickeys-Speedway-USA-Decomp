#include "PR/ultratypes.h"

typedef struct Overlay57ModeChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} Overlay57ModeChoice;

typedef struct Overlay57ModeOutput {
    u8 mode;
    u8 pad01[3];
    s8 value;
    u8 pad05[0x23];
} Overlay57ModeOutput;

extern u32 D_800D31B8;
extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57State;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57Delay;
extern s32 gOverlay57Timer;
extern s16 gO57ModeValueTable[];
extern Overlay57ModeChoice gO57ModeChoices[4];
extern Overlay57ModeChoice gO57ModeChoicesEnd[];
extern Overlay57ModeOutput *gO57ModeOutputs;
extern u8 D_8007BEF8;
extern u8 D_8007BEFC;
extern u8 D_8007BF0C;
extern u8 D_8007BF04;
extern s16 gO57ModeShortValue;
extern u8 D_8007BF08;
extern s32 gO57ModePrimaryIds[];
extern s32 gO57ModePrimaryValues[];
extern s32 gO57ModeSecondaryIds[];
extern u8 gO57ModeExternalByte;
extern u8 D_800D3140[];
extern u8 gO57ModeFinalArg1[];

extern void amSndPlay(s32 choice, s32 value);
extern void joyCreateMap(const s8 values[4]);
extern void mainSetMode(s32 choice);
extern void mainChangeLevel(s32 kind, s32 value, s32 arg2, s32 arg3,
                                   s32 arg4, s32 arg5);
extern void overlay84Mark(void);
extern void overlay84ClearMode(void);
extern void animseqStopPath(u8 value);
extern void overlay84ActivateCurrent(s32 value);
extern void animseqStartPath(u8 id);
extern void animseqStopPath(u8 id);
extern void overlay94SetValue(s32 value);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void func_8002FB34(void *object, void *state, f32 position,
                                   f32 extent, f32 scaleX, f32 scaleY,
                                   s32 index, s32 mode);

/* Overlay 57 text +0x4064..+0x43C8. */
/* THE JOINED LINE IS LOAD-BEARING. `outputIndex = 0; do {` shares one physical
 * line deliberately; splitting it costs the three-word schedule residual back.
 *
 * IDO hoists both loop base addresses into the loop preheader and stamps the
 * hoisted records with the *loop header's* source line, not their use sites'.
 * The index initializer and those two addresses are independent -- no
 * dependence edge orders them -- so as1's reorganizer separates them on its
 * minimized `lineno` key, and an initializer written on an earlier line wins.
 * Sharing the loop header's line removes the separation. This is why the
 * 119-entry flag lattice and ten source variants could not move it: the
 * deciding key was never a flag or a statement order, it was the line number.
 *
 * Measured with `decomp-workbench trace-emit` on an emit-provenance
 * instrumented ugen (workbench improvement-backlog item #3(b)): block 0 emits
 * the initializers at lines 90 and 91 and both hoisted addresses at line 92.
 * Canonical -O2 -mips2 stays exact-size at 0x364 with the exact 0x58 frame;
 * the three relocation-masked differences at +0xE0..+0xE8 are gone. What
 * remains is the pre-existing relocation-surface difference (four words where
 * the target resolves a shared base plus offset and the C object emits
 * per-symbol LO16 records), which is a symbol-layout question, not a
 * schedule one. */
void overlay57HandleModeInput(s32 updateRate) {
    s32 i;
    s32 outputIndex;
    s32 *idPtr;
    s8 enabled[4];

    gOverlay57ModeFlag = 0;
    gOverlay57State = 3;

    if (((D_800D31B8 & 0x9000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(12, 0);

        gO57ModeOutputs[0].value =
            (s8) gO57ModeValueTable[gO57ModeChoices[0].tableIndex];
        gO57ModeOutputs[0].mode = 2;
        D_8007BEF8 = 1;
        D_8007BEFC = 0;
        D_8007BF0C = 0;
        D_8007BF04 = 0;
        gO57ModeShortValue = 0x3FC;
        D_8007BF08 = 0;

        i = 0;
        outputIndex = 0; do {
            enabled[i] = gO57ModeChoices[i].enabled;
            if (gO57ModeChoices[i].enabled != 0) {
                gO57ModeOutputs[outputIndex].value =
                    (s8) gO57ModeValueTable[
                        gO57ModeChoices[i].tableIndex];
                outputIndex++;
            }
            i++;
        } while (gO57ModeChoicesEnd != &gO57ModeChoices[i]);

        joyCreateMap(enabled);
        mainSetMode(0);
        mainChangeLevel(0x1E,
                               gO57ModeValueTable[
                                   gO57ModeChoices[0].tableIndex],
                               0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    } else if (((D_800D31B8 & 0x4000) != 0) &&
               (gOverlay57DistanceState == 0)) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gO57ModeExternalByte);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(2);

        idPtr = gO57ModePrimaryIds;
        while (*idPtr != -1) {
            animseqStartPath((u8) *idPtr);
            overlay57SetNodeValue(*idPtr, gO57ModePrimaryValues[*idPtr],
                                  0.0070000002f);
            idPtr++;
        }

        idPtr = gO57ModeSecondaryIds;
        while (*idPtr != -1) {
            animseqStopPath((u8) *idPtr);
            idPtr++;
        }
        overlay94SetValue(0);
    }

    for (i = 0; i < updateRate; i++) {
        gOverlay57Delay -= gOverlay57Delay >> 3;
    }

    func_8002FB34(D_800D3140, gO57ModeFinalArg1,
                            (f32) ((gOverlay57Delay >> 5) + 0x104), 184.0f,
                            1.0f, 1.0f, -2, 3);
}

