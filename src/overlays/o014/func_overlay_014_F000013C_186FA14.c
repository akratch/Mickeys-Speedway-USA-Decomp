#include "PR/ultratypes.h"

/* Tier D: layout from this function's loads. A group is a count followed by
 * that many 8-byte rules; groups are stored back to back. */
typedef struct Rule {
    u8 left;
    u8 operation;
    u8 right;
    u8 field114;
    s16 value;
    s16 field110;
} Rule;

typedef struct RuleGroup {
    s16 count;
    Rule rules[1];
} RuleGroup;

extern s32 gOverlay14StateC4;
extern s32 gOverlay14Context10C;
extern s32 gOverlay14Field110;
extern s32 gOverlay14Field114;
extern RuleGroup *gOverlay14AssetF0;
extern void *gOverlay14OffsetsF4;

/* The call to overlay 14 +0x0 is a SYMBOL relocation record, so it is named
 * through a placeholder; the three later same-module calls are JUMP records
 * and name their callees directly. */
extern void overlay14ResetReloc(void);
extern void *piRomLoad_o014Reloc(s32 resourceId);
extern s32 frontGetLanguage_o014Reloc(void);
extern s32 overlay14ReturnOne(s32 rule);
extern s32 overlay14ApplyValues(s32 value, s32 mode);
extern void overlay14Reset(void);

/* Rewritten from the listing on 2026-10-02 (the inherited candidate was 29
 * masked words off at frame 0x30). What closed it: the group walk indexes the
 * group's own rule array (`&entry->rules[entry->count]`, which emits the
 * cursor-first add) instead of byte arithmetic on an s16 cursor, the locals
 * are declared in this order (frame 0x28), and both loops keep the
 * post-decremented count in `right` before testing it, which is the dead copy
 * the shipped loops carry. The guard really is always taken. */
void func_overlay_014_F000013C_186FA14(s32 group, s32 context) {
    s32 size;
    s32 left;
    s32 right;
    s32 count;
    Rule *rule;
    RuleGroup *entry;

    overlay14ResetReloc();
    if (group < 0 || group >= 0) {
        return;
    }
    gOverlay14Context10C = context;
    gOverlay14AssetF0 = piRomLoad_o014Reloc(8);
    switch (frontGetLanguage_o014Reloc()) {
        case 1: size = 0xB; break;
        case 2: size = 0xD; break;
        case 3: size = 0xF; break;
        case 5: size = 0x11; break;
        default: size = 9; break;
    }
    gOverlay14OffsetsF4 = piRomLoad_o014Reloc(size);
    entry = gOverlay14AssetF0;
    right = group--;
    if (right != 0) {
        do {
            entry = (RuleGroup *)&entry->rules[entry->count];
            right = group--;
        } while (right != 0);
    }
    count = entry->count - 1;
    rule = entry->rules;
    right = count--;
    if (right != 0) {
        do {
            left = overlay14ReturnOne(rule->left & 0x7F);
            if (rule->left & 0x80) {
                left = !left;
            }
            right = overlay14ReturnOne(rule->right & 0x7F);
            if (rule->right & 0x80) {
                right = !right;
            }
            if (rule->operation == 0) {
                if (left && right) {
                    break;
                }
            } else if (rule->operation == 1) {
                if (left || right) {
                    break;
                }
            } else if (left) {
                break;
            }
            rule++;
            right = count--;
        } while (right != 0);
    }
    gOverlay14Field110 = rule->field110;
    gOverlay14Field114 = rule->field114;
    if (overlay14ApplyValues(rule->value, 3) != 0) {
        gOverlay14StateC4 = 1;
    }
    overlay14Reset();
}
