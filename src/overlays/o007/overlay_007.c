#include "overlays/overlay_007.h"

/* Overlay 7, ADR 0006 consolidation: C before the middle assembly island. */

void overlay7ReleaseEntry(Overlay7Entry *entry) {
    Overlay7Entry *previous;
    Overlay7Entry *current;

    if (gOverlay7ActiveHead != 0 && entry != 0) {
        entry->active = 0;
        if (entry == gOverlay7Selected) {
            gOverlay7Selected = 0;
        }

        previous = 0;
        current = gOverlay7ActiveHead;
        if (current != entry) {
            do {
                previous = current;
                current = current->next;
            } while (current != entry);
        }

        if (previous != 0) {
            previous->next = current->next;
        }
        if (current == gOverlay7ActiveTail) {
            gOverlay7ActiveTail = previous;
        }
        if (current == gOverlay7ActiveHead) {
            gOverlay7ActiveHead = current->next;
        }
        current->next = gOverlay7FreeHead;
        gOverlay7FreeHead = current;
    }
}

/* Matched 2026-09-12, lane p9-tight. Sixteen words to zero, two edits in the
 * tail, both named below at the point of use.
 *
 * 1. The `result = entry;` self-copy in the else arm. It is the whole of the
 *    ten register words: uopt propagates `result` into every use of `entry`
 *    after the copy, so the ActiveHead store, the `entry->next = 0` store and
 *    the final ActiveTail store all read $v0 where the ROM reads $s0.
 * 2. `return entry;` inside the guard with a bare `return 0;` after it,
 *    instead of `else { entry = 0; } return entry;`. That is what puts the
 *    `move $v0, $zero` in the branch-likely's delay slot and leaves the dead
 *    copy the ROM carries at the join; the shared-exit form sets $s0 there
 *    instead and needs two more instructions to get back to $v0.
 *
 * Neither edit works alone: the reshape on its own is 22 words (recorded by the
 * 2026-09-04 probe as 74/96) and the self-copy on its own is 6.
 */
Overlay7Entry *overlay7AcquireEntry(Overlay7Owner *owner, u16 value, u8 type) {
    Overlay7Entry *entry;
    Overlay7Entry *result;
    s8 *ownerPriority;

    entry = gOverlay7ActiveHead;
    if (entry != 0) {
        do {
            if (entry->active != 0 && entry->owner == owner &&
                entry->value == value) {
                return 0;
            }
            entry = entry->next;
        } while (entry != 0);
    }

    entry = gOverlay7ActiveHead;
    ownerPriority = owner->priority;
    if (entry != 0) {
        do {
            if (entry->active != 0 &&
                ((entry->type < type && entry->owner == owner) ||
                 (*ownerPriority < D_8007C1A0 &&
                  type >= entry->type))) {
                if (entry->nested != 0) {
                    entry->nested->active = 1;
                }
                overlay7ReleaseEntry(entry);
            }
            entry = entry->next;
        } while (entry != 0);
    }

    result = gOverlay7FreeHead;
    if (result != 0) {
        entry = result;
        gOverlay7FreeHead = result->next;
        if (gOverlay7ActiveTail != 0) {
            gOverlay7ActiveTail->next = result;
            gOverlay7ActiveTail = result;
        } else {
            gOverlay7ActiveHead = entry;
            /* Load-bearing, and it compiles to nothing. uopt copy-propagates
             * `result` into every use of `entry` after `entry = result`, which
             * is why the three tail stores read $v0 here and $s0 in the ROM.
             * Redefining `result` on ONE arm makes the two disagree at the
             * merge, so the propagation stops there and the stores below keep
             * `entry`; the assignment itself is a self-copy at this point and
             * uopt deletes it. */
            result = entry;
        }
        entry->next = 0;
        gOverlay7ActiveTail = entry;
        return entry;
    }
    return 0;
}

/* DKR v77/v80 and JFG exact-object scans are negative for this allocator. */
void overlay7CreateEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;

    entry = overlay7Acquire(owner, value, type);
    if (entry == NULL) {
        gOverlay7Current = NULL;
    } else {
        gOverlay7Current = entry;
        entry->owner = owner;
        entry->field04 = 0;
        entry->value = value;
        entry->type = type;
        entry->nested = 0;
        entry->active = 1;
    }
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
void overlay7AppendEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;
    Overlay7Entry *current;

    entry = overlay7Acquire(owner, value, type);
    current = gOverlay7Current;
    if (current == 0) {
        overlay7CreateEntry(owner, value, type);
    } else {
        current->nested = entry;
        if (entry != 0) {
            entry->owner = owner;
            entry->field04 = 0;
            entry->value = value;
            entry->type = type;
            entry->nested = 0;
            entry->active = 2;
        }
    }
}

