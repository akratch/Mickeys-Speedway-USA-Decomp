# Executable-byte accounting

ADR 0003 weights functions by their executable size. The overlay atlas and
linked ELF also carry physical alignment extents. The scoreboard excludes only
reviewed nonexecutable ranges listed in `config/nonexecutable-ranges.us.json`;
it continues to report their physical extent separately. Exclusion changes
neither matched credit nor the ROM, instructions, linker layout or atlas.

The initial review is `0c409163656ba64fa7bc75d74fc989a66b11a6d0`, supplemented
by the independent boundary/reference review below. The manifest identifies
the authenticated US ROM, exact owner and extent, and a separately classified
whole-symbol alignment identity. Its entries are decisions, not a rule that
zero words, assembly owners or particular filenames are padding.

## Reviewed boundaries

- 82 overlay alignment owners contain 656 bytes. These include the tail owners
  in overlays 2 and 71 and the second padding owner in overlay 58. A filename
  suffix is insufficient to identify this set.
- Four overlay owners contain four bytes each beyond their final function:
  overlay 9 at text offsets 0x151C..0x1520, overlay 12 at 0x129C..0x12A0,
  overlay 46 at 0x195C..0x1960 and overlay 99 at 0x135C..0x1360. The final
  return and its delay slot precede the excluded range. Overlay 9's final
  exact island ends at the exclusion boundary.
  Overlays 46 and 99 have since had their final functions matched in C
  (2026-10-02). A matched function's object ends at its return, so each of
  those two words became its own `overlay_0NN_padding` owner; the manifest
  now holds 84 overlay padding owners (664 bytes) and two trailing-alignment
  rows (overlays 9 and 12, 8 bytes). The excluded total is unchanged.
- The ELF extents of `func_8003C80C` and `func_8002B040` include
  respectively twelve and eight alignment bytes after their extracted
  function end labels and completed return delay slots. Their executable
  extents remain 472 and 136 bytes. `func_800180B4` carried four such bytes
  until it was matched in C (2026-10-02); its compiled ELF extent is now its
  824 executable bytes, the alignment word lies outside every function
  extent, and its exclusion row was retired.
- `func_8005800C` is a separate four-byte alignment identity. The preceding
  `osFlashReadArray` ends at 0x80058008 after a return and its executed nop
  delay slot; the next function starts at 0x80058010. The isolated identity
  lies within that alignment gap. Independent reviews found no source/header
  declaration or use, absolute pointer literal, direct jump/call, resident
  branch, resident/overlay text low-half address construction, or resident
  runtime-export entry naming it. These are bounded no-observed-reference
  checks, not a proof against every hypothetical computed interior jump.
  The preceding executed delay-slot nop remains counted.

The 700 excluded bytes were unresolved: 656 overlay GLOBAL_ASM bytes, sixteen
overlay NON_MATCHING bytes, twenty-four resident NON_MATCHING bytes and the
four-byte resident GLOBAL_ASM identity. The last exclusion removes one
nonexecutable identity from resident function and per-area counts. All credited
numerators remain unchanged. The twelve alignment bytes beyond two raw
resident ELF function extents were already outside the metric and are not
subtracted again.

## Validation and consumers

`tools/executable_accounting.py` validates the manifest's schema, ROM identity,
non-overlap and exact atlas ownership. Live progress additionally checks the
ROM contents, resident ELF identity/extent/source and existing categories.
Matched ranges and verified assembly cannot intersect exclusions. A changed
owner or category requires review; the helper refuses rather than silently
moving credited bytes. Nonzero bytes invalidate a reviewed exclusion, but zero
contents never create one. Delay-slot nops, executable zero words and unknown
all-zero functions stay counted.

Progress applies the same effective sizes to the denominator, five categories,
function counts and area reports. It prints the physical extent and excluded
bytes beside the executable score. Triage derives its denominator from that
scoreboard snapshot and this shared exclusion contract. CI's partial check
validates the contract and the snapshot's physical/executable arithmetic;
only the full local check verifies the current ELF and authenticated ROM.

Detailed reference scans and boundary receipts remain ignored under
`build/accounting-contract/`. No instruction listing or ROM content belongs
in this manifest or review.
