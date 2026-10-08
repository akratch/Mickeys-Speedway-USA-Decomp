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

extern void overlay101BeginChainReloc(void);
extern void overlay101PrepareChainReloc(void *context,
                                       Overlay101DrawChain *chain);
extern void overlay101DrawChainType1Reloc(void *context,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);
extern void overlay101DrawChainType2Reloc(void *context, void *mtx,
                                         s32 *vertices,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);
extern void overlay101DrawChainType3Reloc(void *context,
                                         Overlay101DrawChain *chain,
                                         Overlay101DrawNode *node);

void overlay101DrawChain(void *context, void *mtx, s32 *vertices,
                         Overlay101DrawChain *chain) {
    register Overlay101DrawNode *node;
    s32 type;

    if (chain->enabled != 0) {
        overlay101BeginChainReloc();
        if (chain->mode != 4) {
            overlay101PrepareChainReloc(context, chain);
        }
        node = chain->next;
        type = chain->type;
        while (node != NULL) {
            switch (type) {
                case 1: {
                    Overlay101DrawNode *current = node;
                    overlay101DrawChainType1Reloc(context, chain, node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 2: {
                    Overlay101DrawNode *current = node;
                    overlay101DrawChainType2Reloc(context, mtx, vertices, chain,
                                                  node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 3: {
                    Overlay101DrawNode *current = node;
                    overlay101DrawChainType3Reloc(context, chain, node);
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
