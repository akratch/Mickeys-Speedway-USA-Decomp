#include "PR/ultratypes.h"

#define NULL ((void *)0)

typedef struct Overlay43PendingEntry {
    u8 pad00[0x20];
    s32 arg20;
    u8 pad24[0x04];
    s32 arg28;
    u8 pad2C[0x04];
    s32 arg30;
    u8 pad34[0x85];
    u8 pending;
} Overlay43PendingEntry;

typedef struct Overlay43State {
    u8 pad00[0xC8];
    s8 pendingCount;
} Overlay43State;

extern Overlay43State D_C8;
extern Overlay43PendingEntry *D_120[];
extern u8 ext_4d258[];
extern void func_8002E800(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void osRecvMesg(void *queue, s32 *message, s32 flags);
extern void func_overlay_043_F0001378_188B348(void *entry);

void func_overlay_043_F0000194_188A164(void) {
    Overlay43PendingEntry *entry;
    Overlay43PendingEntry *previous;
    s32 message;
    s32 index;

    message = 0;
    for (index = 0; index < D_C8.pendingCount; index++) {
        entry = D_120[index];
        if (index > 0) {
            previous = D_120[index - 1];
        } else {
            previous = NULL;
        }
        if (entry->pending != 0) {
            func_8002E800(entry->arg28, entry->arg30, 3, entry->arg20);
            entry->pending = 0;
            if (previous != NULL) {
                func_overlay_043_F0001378_188B348(previous);
            }
            osRecvMesg(ext_4d258, &message, 1);
        }
    }
    func_overlay_043_F0001378_188B348(entry);
    D_C8.pendingCount = 0;
}
