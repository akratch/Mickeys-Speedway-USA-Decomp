<!-- plateau-handoff:func_8002CF6C:start -->
### `func_8002CF6C` plateau handoff

- source: `src/main/saves.c`
- score: 0/88 words, promoted
- frame: 0x48
- relocations: 11
- first mismatch: none
- summary: Matched. Plain locals, the donor while (n--) copies, a bitfield for the preserved bit and one u32 pointer for the footer; two pad locals place the homes.

#### 2026-10-01, lane b-misc: matched by discarding the write-state struct

The inherited body kept the buffer and the two saved values in a padded
state struct, reused the parameter to carry the buffer after the reset test,
opened an `if (1)` region for the footer stores, spelled the preserved bit as
a shift pair on a 16-bit read plus a hand mask on the write, and wrote the
three copies as do/while loops from a pre-decremented count.

Written from the listing with the JFG donor's idiom, a 12-cell product (six
footer spellings by two loop spellings) has one structurally exact cell:

- plain locals for the queue, buffer and saved values (the allocator homes
  them and reloads after each call by itself, and takes the callee-saved
  register for the buffer at the end without any parameter reuse);
- `n = 0x200; while (n--)` and `n = 0x18; while (n--)` for the copies;
- the preserved bit as a one-bit bitfield, read and written by name;
- a `u32 *footer` assigned before the checksum call, with the call result
  stored through `footer[0]`. That is the target's leftover add: the footer
  pointer is materialised after its two stores were rewritten on the buffer
  base. Assigning the pointer after the call, incrementing it, or indexing
  the buffer directly are each 4 to 12 bytes off.

That cell was 8 words, all frame displacement at a 0x30 frame. Register-only
locals take homes here, so declaring the two cursors first and two pad
locals around the buffer gives the 0x48 ladder and 0 words. `gmake verify`
passes.
<!-- plateau-handoff:func_8002CF6C:end -->
