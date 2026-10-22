#include "PR/ultratypes.h"

extern s32 gOverlay14TransitionValue[];
extern s32 gOverlay14FlagC4Base[];
extern s32 gOverlay14FlagC8Base[];
extern s32 gOverlay14ModeE4Base[];
extern s32 gOverlay14EnabledF8Base[];
extern s32 gOverlay14CommandHeaderBase[];

#define gOverlay14FlagC4 gOverlay14FlagC4Base[0x31]
#define gOverlay14FlagC8 gOverlay14FlagC8Base[0x32]
#define gOverlay14ModeE4 gOverlay14ModeE4Base[0x39]
#define gOverlay14EnabledF8 gOverlay14EnabledF8Base[0x3E]
#define gOverlay14CommandHeader gOverlay14CommandHeaderBase[0x3F]

extern void overlay14InitializeReloc(void);
extern void overlay14SetActiveReloc(s32 active);
extern void overlay14PrepareReloc(s32 step);
extern void overlay14AdvanceReloc(s32 step);
extern void overlay14RetreatReloc(s32 step);
extern void overlay14DrawPrimaryReloc(void *context);
extern void overlay14DrawAlternateReloc(void *context);

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this body. */
void overlay14UpdateTransition(void *context, s32 step) {
    overlay14PrepareReloc(step);
    if ((gOverlay14FlagC4 != 0) && (gOverlay14FlagC8 == 0)) {
        if ((gOverlay14ModeE4 != 1) && (gOverlay14EnabledF8 != 0)) {
            overlay14AdvanceReloc(step);
        } else if (gOverlay14CommandHeader != 0) {
            overlay14RetreatReloc(step);
        }
        gOverlay14TransitionValue[0] += step * 0x10;
        if (gOverlay14TransitionValue[0] >= 0x101) {
            gOverlay14TransitionValue[0] = 0x100;
        }
    } else if (gOverlay14TransitionValue[0] > 0) {
        gOverlay14TransitionValue[0] -= step * 0x10;
        if (gOverlay14TransitionValue[0] <= 0) {
            if (gOverlay14FlagC8 == 0) {
                overlay14InitializeReloc();
            } else {
                gOverlay14TransitionValue[0] = 0;
            }
        }
    }
    overlay14SetActiveReloc(1);
    if (gOverlay14TransitionValue[0] != 0) {
        if (gOverlay14ModeE4 != 1) {
            overlay14DrawPrimaryReloc(context);
        } else {
            overlay14DrawAlternateReloc(context);
        }
    }
    overlay14SetActiveReloc(0);
}
