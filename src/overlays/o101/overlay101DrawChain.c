#include "PR/ultratypes.h"

typedef struct Overlay101DrawNode {
    s32 type;
    struct Overlay101DrawNode *next;
} Overlay101DrawNode;

typedef struct Overlay101DrawChain {
    s32 type;
    Overlay101DrawNode *next;
    u8 mode;
    u8 pad09[0xE];
    u8 enabled;
} Overlay101DrawChain;

extern void camStandardOrtho(void);
extern void overlay101DrawPanel(void *context,
                                       Overlay101DrawChain *chain);
extern void func_overlay_101_F0002510_18DDD30(void *context,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);
extern void overlay101DrawTransformed(void *context, void *mtx,
                                         s32 *vertices,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);
extern void overlay101DrawElement(void *context,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);

void overlay101DrawChain(void *context, void *mtx, s32 *vertices,
                         Overlay101DrawChain *chain) {
    register Overlay101DrawNode *node;
    s32 type;

    if (chain->enabled != 0) {
        camStandardOrtho();
        if (chain->mode != 4) {
            overlay101DrawPanel(context, chain);
        }
        node = chain->next;
        type = chain->type;
        while (node != NULL) {
            switch (type) {
                case 1: {
                    Overlay101DrawNode *current = node;
                    func_overlay_101_F0002510_18DDD30(context, chain, node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 2: {
                    Overlay101DrawNode *current = node;
                    overlay101DrawTransformed(context, mtx, vertices, chain,
                                                  node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 3: {
                    Overlay101DrawNode *current = node;
                    overlay101DrawElement(context, chain, node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                default:
                    node = NULL;
                    break;
            }
        }
    }
}
