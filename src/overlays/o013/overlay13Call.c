#include "PR/ultratypes.h"

extern void overlay13Release(void);

/* DKR v77/v80 and JFG checks found no exact donor for this no-argument wrapper. */
void overlay13Call(void) {
    overlay13Release();
}
