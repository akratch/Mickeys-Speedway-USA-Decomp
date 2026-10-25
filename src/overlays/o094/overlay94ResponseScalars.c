#include "PR/ultratypes.h"

/* Tier B: ten distinct four-byte float reads supply the controller's power
 * bases, input/turn scales, inactive target and signed turn thresholds.
 * Tier D: original names/declarations, literal-versus-named-constant origin
 * and source grouping. Conservatively writable: runtime RO role alone does
 * not prove C constness or exclude indirect mutation. No padding object. */
f32 o94MovingDecayBase = 0.85f;
f32 o94InputTargetScale = 0.02f;
f32 o94InputDecayBase = 0.95f;
f32 o94NeutralDecayBase = 0.92f;
f32 o94InactiveDecayBase = 0.95f;
f32 o94InactiveTarget = 0.0032f;
f32 o94NegativeTurnThreshold = -2500.0f;
f32 o94PositiveTurnThreshold = 2500.0f;
f32 o94NegativeTurnVelocityScale = 75000.0f;
f32 o94PositiveTurnVelocityScale = 75000.0f;
