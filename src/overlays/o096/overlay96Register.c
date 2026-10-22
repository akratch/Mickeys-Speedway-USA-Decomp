#include "PR/ultratypes.h"

extern s32 D_80078F84;
extern void *gO96EntriesReloc[16];

void overlay96Register(void *volume) {
    s32 index;

    index = D_80078F84;
    while (index--) {
        if (gO96EntriesReloc[index] == volume) {
            return;
        }
    }
    if (D_80078F84 < 16) {
        gO96EntriesReloc[D_80078F84] = volume;
        D_80078F84++;
    }
}
