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
extern u8 gOverlay11ConfigAReloc;
extern u8 gOverlay11ConfigBReloc;
extern s32 gOverlay11NextModeReloc;

extern O11StatusSlot *func_80028F54(void);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F0002AD8_186B320(void);
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

/* The sibling of func_overlay_011_F0001E4C_186A694 (the same option-menu
 * update for another menu), written as its copy and matched as the first draft of it
 * (227 masked words on the inherited candidate). Same objects, same loop
 * forms and the same five locals; only the case bodies differ: option 2
 * fixes the configuration bytes and clears the status fields
 * unconditionally, options 3 and 4 change level before setting the done
 * flag, and option 5 releases group 6C and sets the resident next-mode word
 * (resident bss 0x800D3048, reached through its relocation record). */
void func_overlay_011_F00022E8_186AB30(s32 updateRate) {
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
                gOverlay11ConfigBReloc = 3;
                gOverlay11ConfigAReloc = 4;
                sub = func_80005820(gOverlay11PadIndex)->sub64;
                gOverlay11OptionDone = 1;
                for (index = 0; index < 6; index++) {
                    status[index].field8 = 0;
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
                mainChangeCameras(1);
                mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                gOverlay11OptionDone = 1;
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
                func_overlay_011_F0002AD8_186B320();
                gOverlay11NextModeReloc = 7;
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                gOverlay11OptionDone = 1;
            }
            break;
        }
    }
}
