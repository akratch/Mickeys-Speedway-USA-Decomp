#include "PR/ultratypes.h"

extern u8 gOverlay55ResourceA[];
extern u8 gOverlay55ResourceB[];
extern void freeFrontEndList(void *resource);
extern void overlay56ReleaseResource(void);

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay55ReleaseAll(void) {
    freeFrontEndList(gOverlay55ResourceA);
    freeFrontEndList(gOverlay55ResourceB);
    overlay56ReleaseResource();
}
