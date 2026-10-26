#include "overlays/overlay1_action_modes.h"

/* Tier D structural reconstruction of the action callback records.
 * All callback names and signatures below have actual Mickey definitions.
 * There is no explicit unused/padding field. Target o32 alignment is expected
 * to round each 10-byte field set to a 12-byte record; the source-profile proof checks
 * the ordinary padding bytes. Original capacity and original TU remain qualified. */
extern void overlay1ConsumeTimer(void);
extern s32 overlay1ReturnZero(void);
extern s32 overlay1HandleCachedMode(void);
extern s32 overlay1ChooseModeObject(void);
extern void overlay1UpdateModeSound(void);
extern void overlay1UpdateAimedTransient(void);
extern void overlay1UpdateTransient(void);
extern void overlay1UpdateCountdown(void);

O1ActionModeData o1ActionModes[O1_ACTION_MODE_COUNT] = {
    {NULL, NULL, 0},
    {NULL, overlay1ConsumeTimer, 0},
    {overlay1ReturnZero, overlay1UpdateModeSound, (1u << 0) | (1u << 5)},
    {overlay1HandleCachedMode, overlay1UpdateAimedTransient, (1u << 0) | (1u << 5)},
    {NULL, overlay1UpdateTransient, (1u << 0) | (1u << 5)},
    {overlay1ChooseModeObject, overlay1UpdateCountdown, 1u << 0},
};
