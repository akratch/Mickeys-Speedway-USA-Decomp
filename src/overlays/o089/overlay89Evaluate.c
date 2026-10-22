#include "PR/ultratypes.h"

typedef struct Overlay89Rule {
    u8 mode;
    u8 pad1;
    u8 index;
    u8 pad3[5];
    u8 value;
} Overlay89Rule;

extern s32 func_8002905C(u8, Overlay89Rule *);

/* DKR v77/v80 and JFG contain no exact donor for this rule evaluator. */
s32 overlay89Evaluate(Overlay89Rule *rule) {
    switch (rule->mode) {
        case 0:
            return rule->value;
        case 1:
            return func_8002905C(rule->index, rule);
        case 2:
            return func_8002905C(rule->index, rule) == 0;
        default:
            return 1;
    }
}
