<!-- plateau-handoff:func_overlay_011_F0001E4C_186A694:start -->
### `func_overlay_011_F0001E4C_186A694` plateau handoff

- source: `src/overlays/o011/func_overlay_011_F0001E4C_186A694.c`
- score: 109/295 words
- frame: 0x40
- relocations: 87
- first mismatch: +0xD8
- summary: Size -4 (294/295), extra +0x28 action spill. index=2 folds scale to -28. Need rematerialized 2 plus nop delay, not colour (L155).

Lane `w18-o011` on base `cbaed235`. Re-measure: ranking size -4 is right;
the prior V0 "-1 word" / first `+0x10` were stale. Configured IDO 5.3
`-O2 -mips2 -32` emits 294 words / 1,176 bytes against 295 / 1,180. Both
frames are `0x40`. Masked 186 / raw 189 / artifact 3. Aligned 190 exact,
53 naming, 11 immediate, 44 structural. Candidate-only `+0x23C` (1) and
`+0x2A4` (2); target-only `+0x1FC` (1) and `+0x288` (3). First naming
`+0xD8` (handle vs index in `v0`/`v1`); first immediate `+0x14C`; first
structural `+0x200`. Candidate has nine stack slots, target eight: extra
`+0x28` is 1 store / 1 load.

Call test: p1 only (the body issues calls). Instrumented IDO at
`ido-instrumented` `.text` is byte-identical to stock. `CDX_PROC=0`, 21
p1 decisions, 15 coloured webs. Unforced `forced=-2`. Cross-kind `s0`
forces move size by +/-8 or +/-12 and destroy the score. `p1:w39=c5`
and `action = D_1C4` each reach size 0 by inserting a copy/`lui`, not
by recovering the missing word; the same four-word insert/delete pair
remains. Colour / `web_footprint --every-colour` is L155 insertion
shadow until size is honestly 0.

The missing word is the unfilled delay of `bne mode, 6` (target `nop`)
together with rematerialized `addiu 2` for `action * 2`. The candidate
spills `action` across `func_80005820`, reloads it at `+0x28`, and
fills that delay with speculative `action << 1`. Target materialises
2 in the mode block and keeps the scale `index * 0x28`.

Tried, all against this 294-word baseline unless noted:

- drop volatile on `menuInput`: size -24, still nine slots
- `index = 2`: extra slot gone (8=8), size -28 because uopt folds
  `status[2]` to a constant offset and deletes the scale
- hoist `action * 2` before the call, `action + action`, L97 `if (1)`
  around the stores, L144 `menuInput = &D_1C4`, sibling do-while,
  generated cursor `D_1CC - 1`, `D_1B8`/`D_204` scalars, `mode` local:
  flat at -4 or a frame regression (`D_1CC - 1` is frame `0x50`)
- sibling `void *` / `void * volatile *`: size +20
- `action = D_1C4` (non-volatile global): coincidental size 0, extra
  `lui`, slot remains, 88 masked

Ownership, ABI, donor scan, and the 87-site reloc surface are unchanged
from the prior shard. The retired carrier-schedule rewrite is still not
admissible. Body restored to the plateau candidate.

Next lever: a source form where `action * 2` rematerialises as `addiu 2`
after the call (action dead, no `+0x28` spill) without forwarding 2 into
the GEPs, and where as1 leaves the `bne mode, 6` delay as `nop`. Do not
run the colour landscape, restore the retired rewrite, or weaken reloc
checks. Do not hardcode `index = 2`.
<!-- plateau-handoff:func_overlay_011_F0001E4C_186A694:end -->
