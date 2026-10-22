#include "PR/ultratypes.h"

extern s32 D_8007BF44;
extern s32 gOverlay101CurrentConfig;
extern s32 gOverlay101ConfigValue;
extern s32 gOverlay101CurrentResource;
extern s32 gOverlay101Ready;

extern void mainChangeLevel(s32 mode, s32 arg1, s32 arg2, s32 value,
                                     s32 enabled, s32 arg5);
extern void overlay101SetScaleReloc(f32 scale, s32 arg1);
extern void mainTitlePageInit(s32 mode);
extern void mainSetMode(s32 resource);
extern void func_80001608(void);

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101Initialize(void) {
    if (D_8007BF44 > 0) {
        mainChangeLevel(1, 0, 0, 15, 1, 0);
        return;
    }

    overlay101SetScaleReloc(0.5f, 0);
    if (gOverlay101CurrentConfig == -1) {
        mainTitlePageInit(1);
    } else {
        mainChangeLevel(gOverlay101CurrentConfig, 0, 0,
                                 gOverlay101ConfigValue, 1, 0);
        mainSetMode(gOverlay101CurrentResource);
    }
    gOverlay101Ready = 0;
    func_80001608();
}
