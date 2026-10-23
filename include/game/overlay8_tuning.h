#ifndef GAME_OVERLAY8_TUNING_H
#define GAME_OVERLAY8_TUNING_H

#include "PR/ultratypes.h"

/* Tier B: Mickey's getter tables identify twenty distinct 344-byte records.
 * Fixed reads establish the eight scalar fields. All indexed response and
 * damping reads share the following floating array, including samples at
 * the old partial views' boundaries. Tier D: this is a bounded shared access
 * contract, not a claim to the original declaration. Storage stays mutable
 * and valid while overlay 8 is loaded; finite inputs and geometric/runtime
 * index safety are separate requirements, not established by this type. */
typedef struct Overlay8TuningRecord {
    f32 gravity;
    f32 mass;
    f32 levelStep;
    f32 slopeFactor;
    f32 steeringCoefficient;
    f32 steeringLimit;
    f32 turnMultiplier;
    f32 reverseCap;
    f32 response[78];
} Overlay8TuningRecord;

/* These name uses within one array, not separate subarrays: forward+32
 * is the coast threshold, forward+33 is braking, and braking+32 is damping. */
enum Overlay8ResponseIndex {
    O8_RESPONSE_REVERSE = 0,
    O8_RESPONSE_SPEED_BASE = 8,
    O8_RESPONSE_FORWARD = 9,
    O8_RESPONSE_COAST_THRESHOLD = 41,
    O8_RESPONSE_BRAKING = 42,
    O8_RESPONSE_DAMPING = 74,
    O8_RESPONSE_COUNT = 78
};

/* Eight scalar floats plus the complete response array leave no padding. */
typedef char Overlay8TuningSize[(sizeof(f32) == 4 &&
                               sizeof(Overlay8TuningRecord) == 344 &&
                               sizeof(((Overlay8TuningRecord *)0)->response) ==
                                   78 * sizeof(f32)) ? 1 : -1];

#endif
