#include "PR/ultratypes.h"

typedef struct Overlay57Child {
    u8 pad0[0x3A];
    s8 index;
    u8 pad3B[0x2D];
    void **items;
} Overlay57Child;

typedef struct Overlay57Entry {
    u8 pad0[8];
    Overlay57Child *child;
} Overlay57Entry;

extern Overlay57Entry *func_800508B4(u8 id);
extern void modSetTextureFrame(void *item, s32 value, s32 scaled);

/* Pinned DKR v77/v80 and JFG checks found no matching donor. */
void overlay57ApplyValue(s32 id, s32 value, s32 scaled) {
    Overlay57Entry *entry;
    Overlay57Child *child;

    entry = func_800508B4((u8) id);
    if (entry != NULL && entry->child != NULL) {
        child = entry->child;
        modSetTextureFrame(child->items[child->index], value, scaled << 8);
    }
}
