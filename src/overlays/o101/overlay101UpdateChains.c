#include "PR/ultratypes.h"

typedef struct Overlay101UpdateNode {
    s32 type;
    struct Overlay101UpdateNode *next;
    u8 pad08[0xB];
    u8 value13;
    u8 pad14[4];
    s32 flags;
    void *resource;
} Overlay101UpdateNode;

typedef struct Overlay101UpdateChain {
    s32 type;
    Overlay101UpdateNode *next;
    u8 state;
    s8 amount;
} Overlay101UpdateChain;

extern s32 gOverlay101Time214;
extern s32 gOverlay101Time338;
extern s32 gOverlay101OrderCount;
extern Overlay101UpdateChain *gOverlay101Order[];
extern void texAnimateSprite(void *resource, void *flags,
                                     s32 value, void *values, s32 step);

void overlay101UpdateChains(s32 step) {
    s32 count;
    s32 type;
    Overlay101UpdateChain **order;
    Overlay101UpdateChain *chain;
    Overlay101UpdateNode *node;

    gOverlay101Time214 += step;
    gOverlay101Time338 += step;
    count = gOverlay101OrderCount;
    if (count--) {
        order = &gOverlay101Order[count];
        do {
            chain = *order;
            if (chain->state == 1) {
                chain->amount += step * 4;
                if (chain->amount >= 0x40) {
                    chain->state = 2;
                }
            } else if (chain->state == 3) {
                chain->amount -= step * 4;
                if (chain->amount <= 0) {
                    chain->state = 0;
                }
            }

            node = chain->next;
            type = chain->type;
            if ((node != NULL) && (type != 0)) {
                do {
                    if ((type == 2) && (node->flags & 1)) {
                        texAnimateSprite(
                            node->resource, &node->flags, node->value13,
                            &node->flags - 1, step);
                    }
                    type = node->type;
                    node = node->next;
                } while ((node != NULL) && (type != 0));
            }
            order--;
        } while (count--);
    }
}
