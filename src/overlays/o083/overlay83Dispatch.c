#include "PR/ultratypes.h"

typedef struct Overlay83SubmitConfig {
    u8 pad00[0x20];
    u8 value20;
    u8 value21;
    u8 value22;
    u8 value23;
} Overlay83SubmitConfig;

extern void sprSetIA2ColOverride(s32 a0, s32 a1, s32 a2, s32 a3,
                                    s32 a4, s32 a5);
extern void camDoSprite(s32 a0, s32 a1, s32 a2,
                                   Overlay83SubmitConfig *config, s32 a4,
                                   s32 mode, s32 alpha);
extern void sprClearIA2ColOverride(void);

/* DKR v77/v80 and JFG have no exact donor for this submission wrapper. */
void overlay83Dispatch(s32 a0, s32 a1, s32 a2,
                       Overlay83SubmitConfig *config, s32 a4) {
    if (config->value23 != 0) {
        sprSetIA2ColOverride(255, 255, 255, config->value20,
                                config->value21, config->value22);
        camDoSprite(a0, a1, a2, config, a4, 14, config->value23);
        sprClearIA2ColOverride();
    }
}
