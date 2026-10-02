<!-- plateau-handoff:overlay7UpdateOwnerMode:start -->
### `overlay7UpdateOwnerMode` plateau handoff

- source: `src/overlays/o007/overlay_007_tail.c`
- score: 0/139 words, promoted
- frame: 0x30
- relocations: 23
- first mismatch: none
- summary: Matched. The row pointer is the row of a four-entry table, formed straight after the index call and not inside the guarded arm.

#### 2026-10-02, lane x-ovlb: matched and promoted

81 -> 0 at delta 0, frame 0x30, 23 relocations, verified. All 81 words
were register naming, with every loop web one colour above the target's.
The decision records give the cause: the loop's top web (the loaded entry
value, save 15) has v0 in its `forbidden` mask, so it took v1 and everything
below moved down one. Cut-down copies found the trigger. With no `if` around
the loop the value takes v0; any `if` around it, whatever the condition,
brings the denial back. Moving `entries = base + index * 4` to the line after
the index call removes the denial (81 -> 27), and what is left is a one-temp
ring offset. Writing the row as `table[index]` with
`Overlay7CheckEntry (*table)[4]` emits the table reload before the shift and
closes it. The byte-offset spelling `(u8 *)base + index * 32` does too. The
loop can be a plain `for (i = 0; i < 3; i++)`.

The earlier closure asked for "allocator evidence that moves the loop
index/bound from a0/t0 to v1/a3". It had the right symptom and the wrong
variable: what moves is the value web, and what moves it is where the row
pointer is formed. No force was used.
<!-- plateau-handoff:overlay7UpdateOwnerMode:end -->
