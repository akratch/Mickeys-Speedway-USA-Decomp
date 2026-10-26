#include "PR/ultratypes.h"
#include "n_audio/gbi.h"

/* Tier B: the clone renderer transfers this SDK viewport for its 64-by-64
 * image. This resource-only TU is an authored boundary (original TU: tier D).
 * Keep the storage mutable; GPU completion and resource lifetime are separate.
 */
Vp o43CloneViewport = {{
    {(64 / 2) * 4, (64 / 2) * 4, G_MAXZ, 0},
    {(64 / 2) * 4, (64 / 2) * 4, 0, 0},
}};
