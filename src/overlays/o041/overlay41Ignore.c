#include "PR/ultratypes.h"

/* The resident stream handler passes seven arguments (func_800517E0, command
 * 0x6E) and this export reads none of them. IDO homes each unused register
 * argument, which is the whole body; the three stack arguments leave no
 * trace. */
void overlay41Ignore(void *object, s32 value1, s32 value2, s32 value3, s32 value4,
                     s32 value5, s32 value6) {
}
