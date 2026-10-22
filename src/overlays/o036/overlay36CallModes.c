#include "PR/ultratypes.h"

/* Generic mode wrappers only; no exact DKR donor. */
extern void overlay36SpawnAndUpdate(void *object, s32 mode);

void overlay36CallModeZero(void *object) { overlay36SpawnAndUpdate(object, 0); }
void overlay36CallModeOne(void *object) { overlay36SpawnAndUpdate(object, 1); }
