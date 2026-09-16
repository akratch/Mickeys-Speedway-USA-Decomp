<!-- plateau-handoff:snow_render:start -->
### `snow_render` plateau handoff

- source: `src/main/weather.c`
- score: 0 differing words
- frame: 0x68
- relocations: 20
- first mismatch: none
- summary: MATCHED. This header claimed an open residual until 2026-09-16, when tools/check_shard_metrics.py reconciled every shard against the ranking; the function had already left the queue. The claim it carried was: Four-word and frame gap with seven exact identities; next lever is command-temporary lifetime.

## Resolved by the JFG donor re-derivation

The structured block above is the historical plateau. On 2026-09-08, the
configured full-TU baseline reproduced 146 candidate words versus 142 target
words, 142 raw and masked differences starting at `+0x0`, frame `0x68` versus
`0x58`, and 20 versus 18 relocations with seven exact offset/type/identities.

PROVENANCE: Jet Force Gemini's public retail-derived
`src/weather.c::func_8005B928_5C528` at revision `efd5abb`, with command
expansions from `include/f3ddkr.h` and `include/PR/gbi.h`. Mickey's assembly
requires its four pre-call caches, remaining-count loop, existing batch widths
and absence of JFG's color commands. The donor's accumulating index and global
reloads are therefore not transplanted.

1. Adapt the donor's typed texture access, early returns, reduced local
   carriers and scoped command expansions. Result: 142 words, frame `0x58`,
   all 18 relocations exact, four differing words starting at `+0x78`.
   Workbench verdict: `schedule-mismatch`; playbook `g0-schedule-probe`,
   lever 23, statement line assignment.
2. Reverse the two matrix-command store pairs to Mickey's observed order.
   Result: five differing words starting at `+0x64`; geometry and relocations
   remain exact. This fixes matrix selection but changes the projection
   command's temporary order.
3. Restore the projection command's donor store order and keep those two
   stores on one physical line, as in the donor macro expansion. Result:
   zero raw or masked differing words, no first mismatch, frame `0x58`,
   and all 18 relocation offsets, types and identities exact.

The ordinary canonical C is promoted, with no instruction postprocessing.
Its 568-byte owned range extracted from the linked `.main` section at
`0x8003B4C0` equals baserom offsets `0x3C0C0`–`0x3C2F8`; there is no padding
credit. `gmake -j$(sysctl -n hw.ncpu) verify` passes the expected US SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

Ignored lane evidence is retained under `build/wb/snow_render-attempts/`:
baseline and all three source/object/report sets, diagnosis and hypothesis
notes, canonical verification log, and the extracted linked-range receipt.
There is no remaining mismatch or plateau lever. Next action: integrate the
function-sized lane commit and repeat the canonical gates in the integration
checkout. No public release is included in this handoff.
<!-- plateau-handoff:snow_render:end -->
