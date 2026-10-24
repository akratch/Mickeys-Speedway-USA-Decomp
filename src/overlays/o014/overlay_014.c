#include "overlays/overlay_014.h"

/*
 * Overlay 14: the scripted front-end menu (command lists, value slots and
 * their rule groups). One translation unit; the module's ten exports keep
 * global linkage and every other function is static.
 */

static s32 overlay14ReturnOne(volatile s32 unused);
static s32 overlay14ApplyValues(s32 value, s32 mode);
static void overlay14ResetMode(void);
static s32 overlay14MoveCommandCursor(s32 direction);
static void * overlay14CreateValue(s32 key, s32 alternate);
static void * overlay14LoadRelocatedValue(s32 index, s32 source);
static Overlay14Asset * func_overlay_014_F00009F4_18702CC(s32 index, s32 context);
static void overlay14Reset(void);
static void overlay14PrepareInputState(s32 step);
static void overlay14AdvanceCommand(s32 context);
static void overlay14StepCommand(s32 context);
static s32 overlay14ReturnOneCallbackA(s16 unused);
static s32 overlay14ReturnOneCallbackB(s16 unused);
static void overlay14DispatchCommand(void);
static void overlay14CallUpdate(s32 unused);
static void overlay14BuildRects(Gfx **context, const Overlay14PackedRect *packed, s32 baseX, s32 baseY, s32 alternateX, s32 alternateY, s32 intensityScale);
static void func_overlay_014_F00013F4_1870CCC(Gfx **context);
static void func_overlay_014_F0001540_1870E18(Gfx **context);
static s32 func_overlay_014_F0001830_1871108(Gfx **context, u8 *stream, s32 skip);

/* The slot cursor's initializer shares the `do {` header line, and the loop
 * bound is spelled inline in the tail test rather than kept in a local. Both
 * are as1 list-scheduler levers: the bound then materializes as a hoisted
 * loop invariant born after the count address, so the three pre-header
 * address pairs issue in the retail order. */
void func_overlay_014_F0000000_186F8D8(void) {
    Overlay14ValueSlot *slot;
    s32 invalid;
    overlay14InitializeReloc();
    invalid = -1;
    gOverlay14Current104 = invalid; gOverlay14Next108 = invalid;
    gOverlay14Context10C = 0; gOverlay14Field114 = 0;
    if (gOverlay14AssetF0) mmFree(gOverlay14AssetF0);
    if (gOverlay14OffsetsF4) mmFree(gOverlay14OffsetsF4);
    slot = gOverlay14ValueSlots_o014Reloc; do {
        if ((gOverlay14ValueCountE8 != 0) && slot->value) mmFree(slot->value);
        slot++;
        slot[-1].value = 0;
        slot[-1].key = invalid;
    } while (slot != gOverlay14ValueSlotsEnd128);
    gOverlay14ValueCountE8 = 0;
    gOverlay14CommandCountEC = 0;
    gOverlay14StateC0 = 0; gOverlay14StateC4 = 0; gOverlay14StateC8 = 0;
    invalid = -1;
    gOverlay14PendingValueD0 = invalid; gOverlay14PendingModeD4 = invalid;
    gOverlay14TransitionD8 = 0; gOverlay14CursorDC = 0; gOverlay14PointerE0 = 0;
    gOverlay14AssetF0 = 0; gOverlay14OffsetsF4 = 0;
    gOverlay14ResultF8 = 0; gOverlay14ResultFC = 0;
}

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
    Overlay14Rule *rule;
    Overlay14RuleGroup *entry;

    func_overlay_014_F0000000_186F8D8();
    if (group < 0 || group >= 0) {
        return;
    }
    gOverlay14Context10C = context;
    gOverlay14AssetF0 = (Overlay14RuleGroup *) piRomLoad_o014Reloc(8);
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
            entry = (Overlay14RuleGroup *)&entry->rules[entry->count];
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

/* DKR checks found only generic constant-return helpers. */
static s32 overlay14ReturnOne(volatile s32 unused) {
    return 1;
}

static s32 overlay14ApplyValues(s32 value, s32 mode) {
    s32 result;
    s32 index;
    Overlay14QueuedCommand *command;

    result = 0;
    if (gOverlay14StateC8 != 0) {
        gOverlay14PendingValueD0Base.value = value;
        gOverlay14PendingModeD4Base.value = mode;
        return 0;
    }

    gOverlay14PendingValueD0Base.value = -1;
    gOverlay14PendingModeD4Base.value = -1;
    if (gOverlay14CommandCountEC < 0x10) {
        if (mode == 2) {
            gOverlay14Value108.value = value;
            gOverlay14StateC8 = 2;
        } else if (mode == 1) {
            gOverlay14ResultFCBase.value = overlay14CreateValue(value, 1);
            if (overlay14MoveCommandCursor(0) != 0) {
                gOverlay14ModeE4.value = 1;
                result = gOverlay14ResultFCBase.value;
            }
        } else {
            gOverlay14ResultF8 = overlay14CreateValue(value, 4);
            gOverlay14TransitionD8Base.value = 0;
            gOverlay14CursorDCBase.value = 0;
            gOverlay14PointerE0Base.value = 0;
            gOverlay14ModeE4.value = mode;
            result = gOverlay14ResultF8;
        }

        if (result != 0) {
            index = gOverlay14CommandCountEC;
            command = &gOverlay14CommandQueue_o014Reloc[index];
            command->value = value;
            command->mode = mode;
            command->result = result;
            gOverlay14CommandCountEC = index + 1;
        }
    }
    return result;
}

/* PROVENANCE: Mickey-derived; pinned DKR v77/v80 and JFG scans found no exact
 * byte donor. A retained function-specific structural scan found no credible
 * near donor (best similarity 0.167).
 * A fidelity-gated scheduler trace exposed a physical-line tie in the prologue.
 * Keeping the function header, local declaration, and do header on one source
 * line produces exact size, frame 0x30, and 56/56 normalized instruction words.
 * Four raw words remain because the friendly data aliases carry zero addends;
 * relocation identity proof also remains pending at the +0x54 call alias. The
 * target runtime surface and candidate each contain 18 records at the expected
 * type/offset sites.
 * The complete 119-configuration flag lattice tied V0. A fidelity-gated IDO
 * allocator trace found seven integer webs (v0 twice and s0..s4). The native
 * scheduler trace passed text/data/rodata/relocation/symbol fidelity and
 * recorded 82 events and 49 ties; its prologue block identified the source-line
 * scheduling lever. An
 * explicit command-type carrier compiled identically; an explicit D_EC pointer
 * lifetime regressed to 55 words and frame 0x28. The two permitted natural
 * probes are exhausted. The remaining work is relocation identity/addend
 * closure, not instruction scheduling. The owned +0x498..+0x578 / ROM
 * 0x186FD70..0x186FE50 range has no padding or export and has five local inbound
 * JUMPs. */
static void overlay14ResetMode(void) { Overlay14QueuedCommand *command; do {
        if (D_EC <= 0) return;
        D_EC--;
        if (D_EC <= 0) return;
        overlay14ReleaseOwner_o014Reloc();
        command = &gOverlay14CommandQueue_o014Reloc[D_EC - 1];
        D_E4 = command->mode;
        if (command->mode != 1) {
            gOverlay14ValueF8 = command->result;
            gOverlay14ValueD8 = 0;
            gOverlay14ValueDC = 0;
            gOverlay14ValueE0 = 0;
            return;
        }
        D_FC = command->result;
    } while (overlay14MoveCommandCursor(0) == 0);
}

static s32 overlay14MoveCommandCursor(s32 direction) {
    Overlay14SelectionList *list;
    Overlay14SelectionEntry *entry;
    s32 index;
    s32 eligible;
    s16 value;

    list = gOverlay14CommandHeader;
    eligible = 0;
    if (list != 0) {
        index = 0;
        entry = (Overlay14SelectionEntry *)list;
        if (list->count > 0) {
            do {
                value = entry->value;
                index++;
                entry++;
                if (overlay14ReturnOneCallbackA(value)) {
                    eligible++;
                }
            } while (index < list->count);
        }

        if (eligible != 0) {
            index = list->selected;
            if (direction > 0) {
                do {
                    index++;
                    if (index >= list->count) {
                        break;
                    }
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
            } else if (direction < 0) {
                do {
                    index--;
                    if (index < 0) {
                        break;
                    }
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
            } else if (!overlay14ReturnOneCallbackA(
                           ((Overlay14SelectionEntry *)list)[index].value)) {
                do {
                    index--;
                    if (index < 0) {
                        break;
                    }
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
                if (index < 0) {
                    index = list->selected;
                    do {
                        index++;
                        if (index >= list->count) {
                            break;
                        }
                    } while (!overlay14ReturnOneCallbackA(
                        ((Overlay14SelectionEntry *)list)[index].value));
                }
            }

            if ((index >= 0) && (index < list->count)) {
                list->selected = index;
            }
        }
    }
    return eligible;
}

/* Matched by subscripting the one slot array everywhere and declaring no slot
 * pointer. The chosen slot's address is then the compiler's own temporary:
 * it is spilled around the calls to a temporary cell, and with it the counter
 * load schedules above the key store as shipped. A declared pointer does not;
 * the earlier listing replay points at the assembler's no-alias fact for an
 * address derived from the array, which is inferred here, not dumped. The
 * reloads of the value after each store come from the joins, not from a
 * volatile field. The free-slot scan clears its index on a line of its own.
 */
static void *overlay14CreateValue(s32 key, s32 alternate) {
    s32 i;
    s32 kind;

    for (i = 0; i < 32; i++) {
        if ((gOverlay14ValueSlots_o014Reloc[i].value != 0) &&
            (gOverlay14ValueSlots_o014Reloc[i].key == key)) {
            return gOverlay14ValueSlots_o014Reloc[i].value;
        }
    }
    i = 0;
    while ((i < 32) && (gOverlay14ValueSlots_o014Reloc[i].value != 0)) {
        i++;
    }
    if (i >= 32) {
        return 0;
    }
    switch (frontGetLanguage()) {
        case 1:
            kind = 0xC;
            break;
        case 2:
            kind = 0xE;
            break;
        case 3:
            kind = 0x10;
            break;
        case 5:
            kind = 0x12;
            break;
        default:
            kind = 0xA;
            break;
    }
    if (alternate != 1) {
        gOverlay14ValueSlots_o014Reloc[i].value = overlay14LoadRelocatedValue(key, kind);
    } else {
        gOverlay14ValueSlots_o014Reloc[i].value =
            func_overlay_014_F00009F4_18702CC(key, kind);
    }
    if (gOverlay14ValueSlots_o014Reloc[i].value != 0) {
        gOverlay14ValueSlots_o014Reloc[i].key = key;
        gOverlay14SlotCountE8++;
    }
    return gOverlay14ValueSlots_o014Reloc[i].value;
}

/* Tier A: linked ROM identity with untouched IDO instruction output.
 * The result carrier keeps both relocation operations in one register.
 * Read through cursor before advancing it; command retains the store address.
 * Declare the two spilled offsets first so their homes bound the frame. */
static void *overlay14LoadRelocatedValue(s32 index, s32 source) {
    s32 start;
    s32 size;
    u8 *buffer;
    u8 *cursor;
    u32 value;

    start = gOverlay14OffsetsF4[index];
    size = gOverlay14OffsetsF4[index + 1] - start;
    buffer = mmAlloc(size, 0x85);
    if (buffer != NULL) {
        piRomLoadSection(source, buffer, start, size);
        cursor = buffer;
        while (*cursor != 0) {
            u8 *command;

            command = cursor;
            switch (*cursor) {
                case 1:
                    cursor += 4;
                    break;
                case 2:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 2;
                    break;
                case 3:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 3;
                    break;
                case 4:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 4;
                    break;
                case 5:
                    value =
                        ((*(u32 *)cursor & 0xFFFFFF) | 0x80000000);
                    cursor += 4;
                    value += (u32)buffer;
                    *(u32 *)command = value;
                    *command = 5;
                    break;
                case 6:
                    cursor += 8;
                    break;
                case 7:
                    cursor += 8;
                    break;
            }
        }
    }
    return buffer;
}

static Overlay14Asset *func_overlay_014_F00009F4_18702CC(s32 index, s32 context)
{
  s32 pad;
  s32 start;
  s32 size;
  Overlay14Asset *asset;
  s32 i;
  Overlay14Asset *entry;
  void *pointer;
  pad = index;
  start = gOverlay14State.ranges[index];
  size = gOverlay14State.ranges[index + 1] - start;
  asset = (Overlay14Asset *) mmAlloc(size, 0x85);
  if (asset != 0)
  {
    piRomLoadSection(context, asset, start, size);
    if (1)
    {
      for (i = 0, entry = asset; i < asset->count; i++, entry++)
      {
        pointer = entry->pointer;
        if (pointer == 0)
        {
          if (entry->marker == 0x4000)
          {
            entry->pointer = &D_A4;
          }
          else
          {
            entry->pointer = &D_B4;
          }
        }
        else
        {
          entry->pointer = ((s32) pointer) + ((u8 *) asset);
        }
      }

    }
  }
  return asset;
}

void overlay14ResetFlags(void) {
    gOverlay14FlagC4 = 0;
    gOverlay14FlagCC = 0;
}

s32 overlay14GetFlagC4(void) {
    return gOverlay14FlagC4;
}

s32 overlay14GetFlagC8(void) {
    return gOverlay14FlagC8;
}

/* DKR v77/v80 and JFG checks found no exact donor for this cleanup wrapper. */
void overlay14ReleaseCurrent(void) {
    gOverlay14FlagC8 = 0;
    if (gOverlay14ResourceD0 != -1) {
        overlay14ApplyValues(gOverlay14ResourceD0, gOverlay14ValueD4);
    }
}

s32 overlay14GetFlagCC(void) {
    return gOverlay14FlagCC;
}

/* The input state's BSS words, defined before their first mention: IDO lays
 * .bss out in first-declaration order. */
s32 gOverlay14InputFlag0;
s32 gOverlay14InputFlag4;
s32 gOverlay14InputFlag8;
s32 gOverlay14PulseC;
s32 gOverlay14Pulse10;
s32 gOverlay14Pulse14;
s32 gOverlay14Pulse18;
s32 gOverlay14Timer1C;
s32 gOverlay14Timer20;

/* Tier B: actual independently bounded cache and command backing. The
 * cache clear/search walks establish 32 eight-byte key/pointer slots;
 * the producer's count<16 guard and replay stride establish 16 commands.
 * Existing input words keep their actual offsets. IDO supplies alignment
 * before the cache and at section end; no padding or reserve object.
 * The queued result is the existing target-width address/result carrier;
 * broader DATA views, pointer-domain and lifetime contracts remain open. */
Overlay14ValueSlot gOverlay14ValueSlots[32];
Overlay14QueuedCommand gOverlay14CommandQueue[16];

/*
 * Overlay 14 +0xB40. JFG proves the 0x1C-byte instruction sequence but names
 * it only with a generated placeholder, which is not promoted here.
 */
static void overlay14Reset(void) {
    gOverlay14Timer1C_o014Reloc = 0;
    gOverlay14Timer20_o014Reloc = 0;
    gOverlay14ResetCC = 0;
}

/* Tier A source reconstruction: update and retest each timer directly.
 * No intervening call or volatile access can change the timer. */
static void overlay14PrepareInputState(s32 step) {
    s32 first;
    s32 second;
    s32 vertical;

    first = joyGetPressed(0);
    second = joyGetStickX(0);
    vertical = joyGetStickY(0);

    gOverlay14InputFlag0_o014Reloc = first & 0x8000;
    gOverlay14InputFlag4_o014Reloc = first & 0x4000;
    gOverlay14InputFlag8_o014Reloc = first & 0x1000;
    gOverlay14PulseC_o014Reloc = 0;
    gOverlay14Pulse10_o014Reloc = 0;
    gOverlay14Pulse14_o014Reloc = 0;
    gOverlay14Pulse18_o014Reloc = 0;

    if ((vertical >= -0x1E) && (vertical < 0x1F)) {
        gOverlay14Timer20_o014Reloc = 0;
    } else if (vertical >= 0x1F) {
        if (gOverlay14Timer20_o014Reloc > 0) {
            gOverlay14Timer20_o014Reloc -= step;
            if (gOverlay14Timer20_o014Reloc <= 0) {
                gOverlay14Timer20_o014Reloc = 0xA;
                gOverlay14PulseC_o014Reloc = 1;
            }
        } else {
            gOverlay14Timer20_o014Reloc = 0x14;
            gOverlay14PulseC_o014Reloc = 1;
        }
    } else if (vertical < -0x1E) {
        if (gOverlay14Timer20_o014Reloc < 0) {
            gOverlay14Timer20_o014Reloc += step;
            if (gOverlay14Timer20_o014Reloc >= 0) {
                gOverlay14Timer20_o014Reloc = -0xA;
                gOverlay14Pulse10_o014Reloc = 1;
            }
        } else {
            gOverlay14Timer20_o014Reloc = -0x14;
            gOverlay14Pulse10_o014Reloc = 1;
        }
    }

    if ((second >= -0x1D) && (second < -0x1E)) {
        gOverlay14Timer1C_o014Reloc = 0;
        return;
    }
    if (second >= 0x1F) {
        if (gOverlay14Timer1C_o014Reloc > 0) {
            gOverlay14Timer1C_o014Reloc -= step;
            if (gOverlay14Timer1C_o014Reloc <= 0) {
                gOverlay14Timer1C_o014Reloc = 0xA;
                gOverlay14Pulse18_o014Reloc = 1;
            }
        } else {
            gOverlay14Timer1C_o014Reloc = 0x14;
            gOverlay14Pulse18_o014Reloc = 1;
        }
    } else if (second < -0x1E) {
        if (gOverlay14Timer1C_o014Reloc < 0) {
            gOverlay14Timer1C_o014Reloc += step;
            if (gOverlay14Timer1C_o014Reloc <= 0) {
                gOverlay14Timer1C_o014Reloc = -0xA;
                gOverlay14Pulse14_o014Reloc = 1;
            }
        } else {
            gOverlay14Timer1C_o014Reloc = -0x14;
            gOverlay14Pulse14_o014Reloc = 1;
        }
    }
}

static void overlay14AdvanceCommand(s32 context) {
    s32 divisor;
    s32 limit;
    s32 extra;

    divisor = func_8004D5C0(2);
    if (gOverlay14Transition != 0) {
        if (gOverlay14Transition < 0) {
            gOverlay14Transition += 1;
            gOverlay14Cursor -= 1;
        } else {
            gOverlay14Transition -= 1;
            gOverlay14Cursor += 1;
        }
    } else if ((gOverlay14PointerE0Base.value != 0) && (gOverlay14InputFlag0_o014Reloc != 0)) {
        gOverlay14TransitionD8Base.value = (0x58 / divisor) - 1;
    } else {
        limit = (0x58 / divisor) - 1;
        if ((gOverlay14Cursor >= limit) && (gOverlay14InputFlag4_o014Reloc != 0)) {
            gOverlay14TransitionD8Base.value = -limit;
        }
    }

    if ((gOverlay14PointerE0Base.value == 0) && (gOverlay14InputFlag0_o014Reloc != 0)) {
        if ((gOverlay14ModeE4.value == 4) && (gOverlay14ValueEC.value >= 2)) {
            overlay14ResetMode();
            return;
        }
        if (gOverlay14ModeE4.value == 3) {
            extra = gOverlay14Value114.value;
            if (extra != 0) {
                overlay14ApplyValues(gOverlay14Value110.value, extra);
                return;
            }
        }
        gOverlay14StateCC.value = 1;
        return;
    }
    if ((gOverlay14InputFlag4_o014Reloc != 0) && (gOverlay14ValueEC.value >= 2)) {
        overlay14ResetMode();
    }
}

static void overlay14StepCommand(s32 context) {
    Overlay14SelectionList *command;

    if (gOverlay14InputFlag4_o014Reloc != 0) {
        overlay14ResetMode();
        return;
    }
    if (gOverlay14InputFlag0_o014Reloc != 0) {
        overlay14DispatchCommand();
        return;
    }
    if ((gOverlay14PulseC_o014Reloc != 0) && (gOverlay14CommandHeaderBase[0x3F]->selected > 0)) {
        overlay14MoveCommandCursor(-1);
        return;
    }
    if (gOverlay14Pulse10_o014Reloc != 0) {
        command = gOverlay14CommandHeaderBase[0x3F];
        if (command->selected < (command->count - 1)) {
            overlay14MoveCommandCursor(1);
        }
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
static s32 overlay14ReturnOneCallbackA(s16 unused) {
    return 1;
}

static s32 overlay14ReturnOneCallbackB(s16 unused) {
    return 1;
}

static void overlay14DispatchCommand(void) {
    Overlay14MenuCommand *command;
    Overlay14SelectionList *header;
    s32 encoded;
    s32 kind;
    s32 action;

    header = gOverlay14CommandHeader;
    if (header != NULL) {
        command = (Overlay14MenuCommand *)((u8 *)header +
            (header->selected * 0x10) + 4);
        if (overlay14ReturnOneCallbackB(command->checkValue) != 0) {
            encoded = command->encodedAction;
            kind = encoded & 0xF000;
            switch (kind) {
                case 0:
                    overlay14CallUpdate(encoded);
                    break;
                case 0x1000:
                    overlay14ApplyValues(encoded & 0xFFF, 4);
                    break;
                case 0x2000:
                    overlay14ApplyValues(encoded & 0xFFF, 1);
                    break;
                case 0x3000:
                    gOverlay14ActiveHandle = encoded & 0xFFF;
                    gOverlay14FlagC8 = 2;
                    break;
                case 0x4000:
                    overlay14ResetMode();
                    break;
                default:
                    gOverlay14FlagC4 = 0;
                    break;
            }
            action = command->trailingAction;
            if (action != -1) {
                overlay14ApplyValues(action, 4);
            }
        } else {
            action = command->fallbackAction;
            if (action != -1) {
                overlay14ApplyValues(action, 4);
            }
        }
    }
}

static void overlay14CallUpdate(s32 unused) {
    overlay14ResetMode();
}

#define gOverlay14FlagC4 gOverlay14FlagC4Base[0x31]
#define gOverlay14FlagC8 gOverlay14FlagC8Base[0x32]
#define gOverlay14ModeE4 gOverlay14ModeE4Base[0x39]
#define gOverlay14EnabledF8 gOverlay14EnabledF8Base[0x3E]
#define gOverlay14CommandHeader gOverlay14CommandHeaderBase[0x3F]
/* Pinned DKR v77/v80 and JFG scans found no exact donor for this body. */
void overlay14UpdateTransition(void *context, s32 step) {
    overlay14PrepareInputState(step);
    if ((gOverlay14FlagC4 != 0) && (gOverlay14FlagC8 == 0)) {
        if ((gOverlay14ModeE4 != 1) && (gOverlay14EnabledF8 != 0)) {
            overlay14AdvanceCommand(step);
        } else if (gOverlay14CommandHeader != 0) {
            overlay14StepCommand(step);
        }
        gOverlay14TransitionValue[0] += step * 0x10;
        if (gOverlay14TransitionValue[0] >= 0x101) {
            gOverlay14TransitionValue[0] = 0x100;
        }
    } else if (gOverlay14TransitionValue[0] > 0) {
        gOverlay14TransitionValue[0] -= step * 0x10;
        if (gOverlay14TransitionValue[0] <= 0) {
            if (gOverlay14FlagC8 == 0) {
                overlay14InitializeReloc();
            } else {
                gOverlay14TransitionValue[0] = 0;
            }
        }
    }
    fontSetButtonMode(1);
    if (gOverlay14TransitionValue[0] != 0) {
        if (gOverlay14ModeE4 != 1) {
            func_overlay_014_F00013F4_1870CCC(context);
        } else {
            func_overlay_014_F0001540_1870E18(context);
        }
    }
    fontSetButtonMode(0);
}
#undef gOverlay14FlagC4
#undef gOverlay14FlagC8
#undef gOverlay14ModeE4
#undef gOverlay14EnabledF8
#undef gOverlay14CommandHeader

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
static void overlay14BuildRects(Gfx **context, const Overlay14PackedRect *packed,
                         s32 baseX, s32 baseY, s32 alternateX,
                         s32 alternateY, s32 intensityScale) {
    Overlay14RenderRect *rect;
    Overlay14RenderRect rects[24];
    s32 count;

    rect = rects;
    count = 0;
    while ((packed->useAlternateX0 >= 0) && (count != 24)) {
        rect->left = packed->x0 + baseX;
        if (packed->useAlternateX0 != 0) {
            rect->left += alternateX;
        }

        rect->top = packed->y0 + baseY;
        if (packed->useAlternateY0 != 0) {
            rect->top += alternateY;
        }

        rect->right = packed->x1 + baseX;
        if (packed->useAlternateX1 != 0) {
            rect->right += alternateX;
        }

        rect->bottom = packed->y1 + baseY;
        if (packed->useAlternateY1 != 0) {
            rect->bottom += alternateY;
        }

        rect->colour = (packed->intensity * intensityScale) >> 7;
        count++;
        packed++;
        rect++;
    }

    frontDrawRectangles(context, count, rects, 0);
}

static void func_overlay_014_F00013F4_1870CCC(Gfx **context) {
    s32 mode;

    overlay14BuildRects(context, (const Overlay14PackedRect *)&gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0Base.value * 0xA0) >> 8);
    gOverlay14HandleE0.value =
        func_overlay_014_F0001830_1871108(context, (u8 *)gOverlay14ValueF8Base.value,
                              gOverlay14ValueDCBase.value);
    fontBackground(0, 0, 0, 0);
    fontColour(0xFF, 0xC0, 0, 0xFF,
                           (gOverlay14ValueC0Base.value * 0xFF) >> 8);
    if (gOverlay14ValueDCBase.value != 0) {
        fontPrintXY(context, 0xC4, 0x14, gOverlay14Args2C, 0xC);
    }
    if ((gOverlay14HandleE0.value != 0) ||
        ((mode = gOverlay14ModeE4.value, mode == 4) &&
         (gOverlay14ValueEC.value >= 2)) ||
        ((mode == 3) && (gOverlay14Value114.value != 0))) {
        fontPrintXY(context, 0xC4, 0x6C, gOverlay14Args30, 0xC);
    }
}

/* Matched 2026-09-16 (lane lm-a), 188/188 words at frame 0x80, unforced.
 * The last twelve words were a v0/v1 exchange between the entries-base web
 * and the index reload. The base web had v0 forbidden with no v0-coloured web
 * in its blocks: the only v0 values there were the unused results of the two
 * dispatch calls before the loop head (a call's result is a v0 web that
 * occupies the call's block; docs/lastmile-region-boundary.md). The calls
 * whose results this body never reads now use the genuine void font-state
 * providers. Text draws and next-line queries use their actual cursor/text
 * contracts; the spacing query uses the word-valued height API.
 * Runtime targets are independently authenticated.
 * The outer while and the generated entry stride (no declared offset carrier)
 * are the previous lanes' work and stay load-bearing. */
static void func_overlay_014_F0001540_1870E18(Gfx **context) {
    u8 saved;
    s32 index;
    s32 cellWidth;
    s32 remaining;
    s32 x;
    s32 y;
    s32 opacity;
    u8 *cursor;
    u8 *drawArg;
    s32 first;

    overlay14BuildRects(context, (const Overlay14PackedRect *)&gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0 * 0xA0) >> 8);
    index = 0;
    cellWidth = func_8004D5C0(2);
    remaining = (0x58 / cellWidth) - 1;
    fontUseFont(2);
    fontBackground(0, 0, 0, 0);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    opacity = (gOverlay14ValueC0 * 0xFF) >> 8;
    while ((index < gOverlay14Entries->count) && (remaining > 0)) {


        first = 1; x = 0x60;
        if (index == gOverlay14Entries->selected)
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, opacity);
        else
            fontColour(0, 0xC0, 0xC0, 0xFF, opacity);
        cursor = gOverlay14Entries[index].text;
        if (overlay14ReturnOneCallbackA(
                gOverlay14Entries[index].kind) == 0) cursor = 0;
        if ((cursor != 0) && (remaining > 0)) {
            do {
                cursor = func_8004D40C(2, (char *)cursor, 0xC8, &drawArg, 0);
                if (cursor != 0) {
                    saved = *cursor; *cursor = 0;
                    fontPrintXY(context, x, y, (char *)drawArg, 0);
                    *cursor = saved; y += cellWidth;
                    if (first != 0) { x += 8; first = 0; }
                    remaining--;
                }
            } while ((cursor != 0) && (remaining > 0));
        }
        index++;


    }
    fontBackground(0, 0, 0, 0);
    fontColour(0xFF, 0xC0, 0, 0xFF, opacity);
    if ((index < gOverlay14Entries->count) || (cursor != 0))
        fontPrintXY(context, 0xC4, 0x6C, gOverlay14Args34, 0xC);
}

/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/overlays/o7/overlay_7/func_overlay_7_007023D4_1EFD4FC.s;
 * body reconstructed from Mickey evidence. */
/* Matched 2026-09-23 (lane A2-ov), 201/201 words at frame 0x90, unforced.
 * The word-valued height getter matches the module's integer consumers.
 * The last five words were case 7's schedule:
 * the ROM loads the four colour bytes at +1..+5 and puts the stream
 * increment in the call's delay slot. as1 breaks a tie between the
 * increment and the loads on source line number (L59), and the increment
 * written first takes the smaller line, so it is picked first and the last
 * load fills the slot. Case 7 therefore draws from a block placed before its
 * label: its own statements are the increment and a jump back, so uopt
 * merges the two blocks with the increment keeping the later line. Temps
 * loaded before the increment reproduce the case too, but a third
 * temporary is globalcoloured to t0 and removes it from the ring (47). */
static s32 func_overlay_014_F0001830_1871108(Gfx **context, u8 *stream, s32 skip) {
    s32 remaining;
    s32 y;
    s32 x;
    s32 extra;
    s32 width;
    s32 cellWidth;
    s32 done;
    s32 result;
    u8 saved;
    u8 *drawArgs[7];
    s32 adjust;
    u8 *cursor;

    result = 0;
    done = 0;
    cellWidth = func_8004D5C0(2);
    remaining = (0x58 / cellWidth) - 1;
    fontBackground(0, 0, 0, 0);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF,
               (gOverlay14ValueC0 * 0xFF) >> 8);
    fontUseFont(2);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    do {
        x = 0x60; width = 0xC8; cursor = 0; adjust = 0; extra = 0;
        switch (*stream) {
        case 1:
            if (skip == 0) { y += cellWidth; remaining--; } else skip--;
            stream += 4;
            break;
        case 2:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 4; x = 0xC4;
            break;
        case 3:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            break;
        case 4:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 1; x = 0x127;
            break;
        case 5:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            adjust = 1;
            break;
        case 6:
            stream += 8;
            saved = stream[-2];
            fontColour(stream[-7], stream[-6], stream[-5], stream[-3],
                       (saved * gOverlay14ValueC0) >> 8);
            break;
        case7Draw:
            fontBackground(stream[-7], stream[-6], stream[-5], stream[-3]);
            break;
        case 7:
            stream += 8;
            goto case7Draw;
        default:
            done = 1;
            break;
        }
        if ((cursor != 0) && (remaining >= 0)) {
            do {
                if (adjust != 0) { x += 8; width -= 8; }
                cursor = func_8004D40C(2, (char *)cursor, width,
                                       drawArgs, 0);
                if (cursor != 0) {
                    if (skip == 0) {
                        remaining--;
                        if (remaining >= 0) {
                            saved = *cursor; *cursor = 0;
                            fontPrintXY(context, x, y,
                                          (char *)drawArgs[0], extra);
                            *cursor = saved; y += cellWidth;
                        }
                    } else {
                        skip--;
                    }
                }
                if (adjust != 0) { x -= 8; width += 8; adjust = 0; }
            } while ((cursor != 0) && (remaining >= 0));
        }
        if (remaining < 0) {
            result = 1;
            done = 1;
        }
    } while (done == 0);
    return result;
}

void overlay14ReleaseOwner(void) {
    Overlay14Owner *owner;

    owner = gOverlay14Owner;
    if (owner != NULL) {
        owner->released = 1;
        gOverlay14Owner = NULL;
    }
}

/* Pinned DKR v77/v80 and JFG scans found no exact finalization donor. */
void overlay14FinalizeActiveHandle(void) {
    Overlay14Object *object;

    if (gOverlay14ActiveHandle != -1) {
        object = func_80005820(0);
        if (object != 0) {
            controlSetPlayerSetup((s16)(s32)object->x,
                          (s16)(s32)(object->y + 2.0f),
                          (s16)(s32)object->z, object->id);
        }
        mainChangeLevel(gOverlay14ActiveHandle, mainGetNextCharacter(), 0,
                        frontGetMode(), 1, 0);
        gOverlay14ActiveHandle = -1;
    }
}
