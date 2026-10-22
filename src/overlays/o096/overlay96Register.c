#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern void *gO96EntriesReloc[16];

void overlay96Register(void *volume) {
    s32 index;

    index = gO96EntryCountReloc;
    while (index--) {
        if (gO96EntriesReloc[index] == volume) {
            return;
        }
    }
    if (gO96EntryCountReloc < 16) {
        gO96EntriesReloc[gO96EntryCountReloc] = volume;
        gO96EntryCountReloc++;
    }
}
