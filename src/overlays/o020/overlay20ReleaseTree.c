#include "PR/ultratypes.h"

typedef struct Overlay20TreeNode {
    u8 pad00[0x20];
    struct Overlay20TreeNode *child0;
    struct Overlay20TreeNode *child1;
    struct Overlay20TreeNode *child2;
} Overlay20TreeNode;

extern void mmFree(void *data);

/* DKR v77/v80 and JFG contain no exact donor. Not recursive: all four calls
 * are shipped SYMBOL records to the resident mmFree, so this frees the three
 * child buffers and then the node itself. */
void overlay20ReleaseTree(Overlay20TreeNode *node) {
    if (node != NULL) {
        if (node->child0 != NULL) {
            mmFree(node->child0);
            node->child0 = NULL;
        }
        if (node->child1 != NULL) {
            mmFree(node->child1);
            node->child1 = NULL;
        }
        if (node->child2 != NULL) {
            mmFree(node->child2);
            node->child2 = NULL;
        }
        mmFree(node);
    }
}
