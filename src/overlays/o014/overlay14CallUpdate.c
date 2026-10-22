#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay14ResetMode(void);

void overlay14CallUpdate(s32 unused) {
    overlay14ResetMode();
}
