#include "PR/ultratypes.h"

typedef struct O11StatusSlot {
    u8 mode;
    u8 pad01[7];
    s32 field8;
    u8 pad0C[0x1C];
} O11StatusSlot;

typedef struct O11ObjectSub {
    u8 pad0;
    s8 value1;
} O11ObjectSub;

typedef struct O11Object {
    u8 pad00[0x64];
    O11ObjectSub *sub64;
} O11Object;

/* Overlay-local storage. The option handles are a one-based table: slot 0
 * is unused and the selected option runs 1..5. */
extern s32 gOverlay11PadIndex;
extern s32 gOverlay11OptionIndex;
extern void *gOverlay11OptionHandles[6];
extern s16 gOverlay11OptionHighlight;
extern s32 gOverlay11OptionAction;
extern s32 gOverlay11OptionDone;

/* Resident objects, reached through runtime relocation records. */
extern s8 gOverlay11StickXReloc[];
extern s32 gOverlay11GameStateReloc;
extern u8 gOverlay11GameStateFlagReloc;
extern u8 gOverlay11ConfigAReloc;
extern u8 gOverlay11ConfigBReloc;

extern O11StatusSlot *func_80028F54(void);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F0002A74_186B2BC(void);
/* A same-module callee whose call site is a SYMBOL record, and the two
 * cross-overlay callees: all three are placeholder declarations. */
extern void overlay11ResetMenuReloc(void);
extern void overlay11SetHandleValueReloc(void *handle, s32 value);
extern void overlay11StopOverlay66Reloc(void *arg0);
extern void amSndPlay(s32 soundId, void *handle);
extern u32 joyGetPressed(s32 controller);
extern O11Object *func_80005820(s32 controller);
extern s32 levelGetNumber(void);
extern void mainChangeLevel(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                            s32 arg5);
extern void mainChangeCameras(s32 arg0);
extern void func_800290AC(s32 arg0);
extern void func_800291D8(s32 arg0);
extern void amTuneSetFadeScaled(f32 arg0, s32 arg1);

/* Matched 2026-10-01 by writing each object the relocation records name
 * instead of offsets from one menu base:
 * - the option handles are indexed by the one-based option number, with no
 *   declared walking pointer; the compiler's own cursor takes the temporary
 *   home and is saved before the index;
 * - the action is read from its object at every test, with no local and no
 *   volatile pointer;
 * - the two status modes are an ordinary `||`, and the six status fields are
 *   cleared by a plain loop, which the compiler unrolls as shipped;
 * - the two configuration bytes are unsigned, so the two 4s stay separate;
 * - five locals, which puts the cursor's temporary at the shipped home. */
void func_overlay_011_F0001E4C_186A694(s32 updateRate) {
    s32 index;
    s8 direction;
    O11ObjectSub *sub;
    s16 value;
    O11StatusSlot *status;

    status = func_80028F54();
    direction = gOverlay11StickXReloc[gOverlay11PadIndex];
    if (direction < -32) {
        if (gOverlay11OptionIndex < 5) {
            gOverlay11OptionIndex++;
            amSndPlay(0x32C, 0);
            direction = gOverlay11StickXReloc[gOverlay11PadIndex];
        } else {
            amSndPlay(0x32D, 0);
            direction = gOverlay11StickXReloc[gOverlay11PadIndex];
        }
    }
    if (direction >= 33) {
        if (gOverlay11OptionIndex >= 2) {
            gOverlay11OptionIndex--;
            amSndPlay(0x32C, 0);
        } else {
            amSndPlay(0x32D, 0);
        }
    }

    for (index = 1; index < 6; index++) {
        value = (index == gOverlay11OptionIndex) ? gOverlay11OptionHighlight : 0;
        overlay11SetHandleValueReloc(gOverlay11OptionHandles[index], value);
    }

    if ((joyGetPressed(gOverlay11PadIndex) & 0x8000) || (gOverlay11OptionAction != 0)) {
        switch (gOverlay11OptionIndex) {
        case 1:
            overlay11StopOverlay66Reloc(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            amTuneSetFadeScaled(0.5f, 0x7F);
            overlay11ResetMenuReloc();
            gOverlay11OptionDone = 1;
            break;
        case 2:
            if (gOverlay11OptionAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                if (((gOverlay11GameStateReloc == 2) || (gOverlay11GameStateReloc == 3)) && (gOverlay11GameStateFlagReloc != 0)) {
                    gOverlay11ConfigBReloc = 4 - gOverlay11GameStateReloc;
                    gOverlay11ConfigAReloc = 4;
                } else {
                    gOverlay11ConfigBReloc = 0;
                    gOverlay11ConfigAReloc = gOverlay11GameStateReloc;
                }
                sub = func_80005820(gOverlay11PadIndex)->sub64;
                gOverlay11OptionDone = 1;
                if ((status->mode == 5) || (status->mode == 6)) {
                    for (index = 0; index < 6; index++) {
                        status[index].field8 = 0;
                    }
                }
                mainChangeLevel(levelGetNumber(), sub->value1, 0, 5, 1, 0);
            }
            break;
        case 3:
            if (gOverlay11OptionAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                mainChangeCameras(1);
                mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        case 4:
            if (gOverlay11OptionAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                gOverlay11OptionDone = 1;
                mainChangeCameras(1);
                if (status->mode == 5) {
                    mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                } else {
                    mainChangeLevel(0xC, 0, 0, 0x11, 1, 0);
                }
            }
            break;
        case 5:
            if (gOverlay11OptionAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (gOverlay11OptionAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (gOverlay11OptionAction == 1) {
                gOverlay11OptionDone = 1;
                func_overlay_011_F0002A74_186B2BC();
                mainChangeCameras(1);
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
            }
            break;
        }
    }
}
