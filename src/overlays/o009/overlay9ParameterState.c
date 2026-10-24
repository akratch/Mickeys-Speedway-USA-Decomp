#include "PR/ultratypes.h"

/* Tier B: the shared parameter pointer's actual readers authenticate these
 * field widths and offsets. Tier D: this is a bounded serialized storage
 * view, not a recovered original declaration or compatible shared-reader ABI.
 * unknown18 preserves four unread initialized bytes; their interpretation
 * and semantics are unknown. They are neither a float claim nor padding. */
typedef struct Overlay9ParameterState {
    f32 inputX;
    f32 inputY;
    f32 throttle;
    f32 position;
    f32 acceleration;
    f32 turnRate;
    u8 unknown18[4];
    f32 magnitude;
    s16 angle20;
    s16 angle22;
    f32 velocity;
    s16 angle28;
    s16 angle2A;
    f32 bobOffset;
} Overlay9ParameterState;

Overlay9ParameterState gOverlay9ParameterState = {
    0.0f, 0.0f, 1.0f, 40.0f, 0.0f, 0.0f,
    { 63, 128, 0, 0 },
    0.0f, 0, 0, 0.0f, 0, 0, 0.0f,
};
